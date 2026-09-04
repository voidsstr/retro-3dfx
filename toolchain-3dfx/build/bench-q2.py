#!/usr/bin/env python3
"""Quake II timedemo on a Voodoo 5 box, through our OpenGL ICD.

The V5-6000 lane had a Q2 number (135 fps on .133, 2026-08-12) but never a
committed harness -- it was measured by hand, so it could not be re-run when
the card moved to another box. This is that harness, built to the same rules
as bench-safe.py (its Q3 sibling):

  * V56K-NO-STOMP: refuse to run if a quake2.exe is ALREADY up. It belongs to
    a person, not to us, and killing it looks exactly like Quake II crashing.
    Only PIDs this script started are ever killed, and only by PID.
  * Every run is bracketed by an uptime check, so a reboot is DETECTED and
    reported rather than silently reported as "no result".
  * COOLDOWN between runs -- this board resets under back-to-back load.

Method (matches the Q3 harness so the two are comparable):
  quake2.exe +set vid_ref gl +set gl_driver 3dfxogl +set gl_mode N
             +set timedemo 1 +map demo1.dm2
`map <name>.dm2` is Quake II's demo-map path (SV_Map_f dispatches on the .dm2
extension), and `timedemo 1` makes it print the same
"%i frames, %3.1f seconds: %3.1f fps" line Quake III uses -- so one regex reads
both engines.

  bench-q2.py [--modes 3,6] [--ip A.B.C.D] [--dir C:\\Games\\Quake2Complete]

gl_mode: 3=640x480, 4=800x600, 6=1024x768.
"""
import argparse, asyncio, json, os, re, sys, functools
print = functools.partial(print, flush=True)   # never lose a result to buffering
sys.path.insert(0, os.environ.get('RETRO_AGENT_DIR',
                os.path.expanduser('~/development/retro-agent')))
from client.retro_protocol import RetroConnection

IP = os.environ.get('BENCH_IP', '192.168.1.191')
Q2 = os.environ.get('BENCH_Q2', r'C:\Games\Quake2Complete')
# Background CPU thieves only -- deliberately NO game executables here.
QUIESCE = ['rotate_wall.exe', 'daemon.exe', 'wuauclt.exe', '3dfxMan.exe',
           'wmiprvse.exe', 'wpabaln.exe']
FPS_RE = re.compile(r'(\d+) frames, ([\d.]+) seconds: ([\d.]+) fps')


async def C(retries=8, delay=10):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898)
            await c.connect('retro-agent-secret', timeout=8)
            return c
        except Exception:
            if i == retries - 1:
                raise
            await asyncio.sleep(delay)


async def uptime(c):
    return json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']


async def q2_pids(c):
    d = (await c.send_command('PROCLIST', timeout=15))[1].decode('ascii', 'replace')
    return [p['pid'] for p in json.loads(d) if p['name'].lower() == 'quake2.exe']


async def run_q2(mode, vsync):
    """One Q2 timedemo. Returns (fps, rebooted)."""
    c = await C()
    before = await uptime(c)

    pre = await q2_pids(c)
    if pre:
        await c.close()
        raise SystemExit('ABORT: quake2.exe already running (pid %s). Someone is '
                         'using the box -- refusing to touch it.' % ','.join(map(str, pre)))
    for img in QUIESCE:
        try:
            await c.command_text('EXEC cmd /c taskkill /f /im %s 2>nul' % img, timeout=20)
        except Exception:
            pass

    log = r'%s\baseq2\qconsole.log' % Q2
    await c.command_text(r'EXECW 12 cmd /c del /f /q "%s" 2>nul & echo ok' % log, timeout=32)

    sw = '1' if vsync else '0'
    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=%s^&^& start "" quake2.exe '
           r'+set vid_ref gl +set gl_driver 3dfxogl +set gl_mode %s +set vid_fullscreen 1 '
           r'+set s_initsound 0 +set logfile 2 +set timedemo 1 +map demo1.dm2') % (Q2, sw, mode)
    await c.command_text(cmd, timeout=40)
    await asyncio.sleep(4)
    mine = [p for p in (await q2_pids(c)) if p not in pre]
    await c.close()

    fps = None
    for _ in range(18):
        await asyncio.sleep(10)
        try:
            c = await C(retries=2, delay=8)
        except Exception:
            return None, True                      # box gone -> reboot
        try:
            if await uptime(c) < before:
                await c.close()
                return None, True
            t = await c.command_text(r'EXECW 20 cmd /c type "%s" 2>nul' % log, timeout=45)
        except Exception:
            await c.close()
            continue
        m = FPS_RE.search(t)
        gl = next((l for l in t.splitlines() if 'GL_RENDERER' in l), None)
        await c.close()
        if m:
            if gl:
                print('  %s' % gl.strip())
            fps = m.group(3)
            break

    # Cleanup must never mask the result: if the box died here, that IS the answer.
    try:
        c = await C(retries=3, delay=10)
    except Exception:
        return fps, True
    rebooted = False
    try:
        for pid in mine:                            # by PID only -- never /im
            try:
                await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok' % pid,
                                     timeout=45)
            except Exception:
                pass
        await asyncio.sleep(3)
        rebooted = (await uptime(c)) < before
        await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
    except Exception:
        rebooted = True
    finally:
        try:
            await c.close()
        except Exception:
            pass
    return fps, rebooted


async def main():
    global IP, Q2
    ap = argparse.ArgumentParser()
    ap.add_argument('--modes', default='3,6', help='gl_mode list (3=640x480, 6=1024x768)')
    ap.add_argument('--cooldown', type=int, default=90)
    ap.add_argument('--vsync', action='store_true', help='cap to refresh (gentler on this board)')
    ap.add_argument('--ip', default=IP)
    ap.add_argument('--dir', default=Q2, help='Quake II directory ON THE BOX')
    a = ap.parse_args()
    IP, Q2 = a.ip, a.dir
    print('box=%s q2=%s vsync=%s' % (IP, Q2, a.vsync))

    rc = 0
    for i, mode in enumerate(a.modes.split(',')):
        if i:
            print('  cooling %ds...' % a.cooldown)
            await asyncio.sleep(a.cooldown)
        fps, rebooted = await run_q2(mode, a.vsync)
        if rebooted:
            print('  gl_mode %s: *** REBOOTED *** -- STOPPING' % mode)
            return 1
        print('  gl_mode %s: %s fps' % (mode, fps))
        if fps is None:
            rc = 1
    return rc

sys.exit(asyncio.run(main()))
