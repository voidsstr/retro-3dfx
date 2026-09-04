#!/usr/bin/env python3
"""UT99 UTbench.dem timedemo on the V5 6000 box, one renderer per run.

Technique borrowed from retro-agent's driver-bench skill (run_bench.py), which
had already solved the two traps that silently wrecked hand-rolled attempts:

  * `UnrealTournament.exe UTbench.dem?timedemo=1` is UT2003/4 syntax. UT99's URL
    parser sees the dot and treats it as a HOSTNAME -- the log shows
    "Resolving UTbench.dem... / Can't find host (WSAHOST_NOT_FOUND)" and the game
    quietly falls back to CityIntro, so the run produces no timedemo at all.
    UT99 needs the console command, bound to a key: F9 = "timedemo 1|demoplay".
  * UT holds bench.log open EXCLUSIVELY, so DOWNLOAD fails with error 32 while it
    runs, and force-killing it truncates the log before the fps line is written.
    F10 = "Exit" exits cleanly, which flushes and releases the log AND clears the
    crash flag, so the next launch does not stop on the "Unreal Tournament
    Recovery Mode" dialog.

The desktop is put at 640x480 first so that dialog's "Run Unreal Tournament"
button is at a known position.

  bench-ut.py glide|opengl|d3d|software [--res 1024x768] [--vsync]

--vsync matters on this board: the UTbench timedemo reset it on BOTH Glide and
OpenGL, but it survives frame-limited. See V56K-SLI-FINDINGS.
"""
import argparse, asyncio, json, os, re, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, os.environ.get('RETRO_AGENT_DIR',
                os.path.expanduser('~/development/retro-agent')))
from client.retro_protocol import RetroConnection

IP = os.environ.get('BENCH_IP', '192.168.1.133')
UT = r'C:\Games\Unreal Tournament (Installed)\System'
# NOT the 8.3 form driver-bench uses: on THIS box UNREAL~1 is "Unreal Tournament
# GOTY [GOG]" and the install we want is UNREAL~3, so a hardcoded UNREAL~1 starts
# the wrong game entirely (and bench.log never appears). The quoted long path
# works here, so use it and avoid the ambiguity.
UTS = UT
DEMO = 'UTbench.dem'
RECOVERY_BTN = (512, 280)                  # "Run Unreal Tournament" @640x480
DEV = {'glide':    'GlideDrv.GlideRenderDevice',
       'opengl':   'OpenGLDrv.OpenGLRenderDevice',
       'd3d':      'D3DDrv.D3DRenderDevice',
       'software': 'SoftDrv.SoftwareRenderDevice'}


async def C(retries=8, delay=10):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: raise
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=25):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def pids(c):
    d = (await c.send_command('PROCLIST', timeout=20))[1].decode('ascii', 'replace')
    return [p['pid'] for p in json.loads(d) if 'unrealtournament' in p['name'].lower()]


async def uptime(c):
    return json.loads((await c.send_command('SYSINFO', timeout=15))[1].decode())['uptime_seconds']


async def main():
    global IP, UT, UTS
    ap = argparse.ArgumentParser()
    ap.add_argument('renderer', choices=sorted(DEV))
    ap.add_argument('--res', default='1024x768')
    ap.add_argument('--vsync', action='store_true',
                    help='cap to refresh (FX_GLIDE_SWAPINTERVAL=1); this board resets unlimited')
    ap.add_argument('--demo-wait', type=int, default=150)
    ap.add_argument('--ip', default=IP, help='box to bench (default %(default)s)')
    ap.add_argument('--dir', default=UT, help='UT System directory ON THE BOX')
    ap.add_argument('--outdir', default='/tmp/qa256', help='where to save bench.log')
    a = ap.parse_args()
    IP = a.ip; UT = a.dir; UTS = UT
    os.makedirs(a.outdir, exist_ok=True)
    print('box=%s ut=%s' % (IP, UT))
    w, h = a.res.split('x')

    c = await C()
    before = await uptime(c)
    if await pids(c):
        print('ABORT: UnrealTournament already running'); await c.close(); return 1

    ini0 = await c.command_binary('DOWNLOAD %s\\UnrealTournament.ini' % UT, timeout=150)
    usr0 = await c.command_binary('DOWNLOAD %s\\User.ini' % UT, timeout=150)
    try:
        ini = ini0.decode('latin-1')
        ini = re.sub(r'(?m)^(Game)?RenderDevice=.*$',
                     lambda m: (m.group(1) or '') + 'RenderDevice=' + DEV[a.renderer], ini)
        for k, v in (('FullscreenViewportX', w), ('FullscreenViewportY', h),
                     ('WindowedViewportX', w), ('WindowedViewportY', h)):
            ini = re.sub(r'(?m)^%s=.*$' % k, '%s=%s' % (k, v), ini)
        await c.send_command('UPLOAD %s\\UnrealTournament.ini' % UT, binary_payload=ini.encode('latin-1'))

        usr = usr0.decode('latin-1')
        usr = re.sub(r'\bF9=[^\r\n]*', 'F9=timedemo 1|demoplay %s' % DEMO, usr, count=1)
        usr = re.sub(r'\bF10=[^\r\n]*', 'F10=Exit', usr, count=1)
        await c.send_command('UPLOAD %s\\User.ini' % UT, binary_payload=usr.encode('latin-1'))

        await ex(c, 'cmd /c del /f /q "%s\\bench.log" 2>nul & echo ok' % UT)
        await c.command_text('DISPLAYCFG set 640 480 16 75', timeout=25)
        await asyncio.sleep(2)
        swap = 'set FX_GLIDE_SWAPINTERVAL=1^&^& ' if a.vsync else ''
        await ex(c, 'cmd /c cd /d "%s" ^&^& %sstart "" UnrealTournament.exe -log=bench.log -nosound'
                 % (UTS, swap), 15)
        print('%s @ %s %s: launching...' % (a.renderer, a.res, 'vsync' if a.vsync else 'UNLIMITED'))
        await c.close()

        await asyncio.sleep(10)
        c = await C()
        await c.command_text('UICLICK %d %d' % RECOVERY_BTN, timeout=15)   # no-op if absent
        await asyncio.sleep(20)                                            # intro -> main menu
        await c.command_text('UIKEY F9', timeout=15)                       # timedemo 1|demoplay
        print('  timedemo started; waiting %ds' % a.demo_wait)
        await c.close()

        waited = 0
        while waited < a.demo_wait:
            await asyncio.sleep(20); waited += 20
            try:
                c = await C(retries=2, delay=8)
            except Exception:
                print('  *** box gone -> REBOOT ***'); return 2
            if await uptime(c) < before:
                print('  *** REBOOTED during the timedemo ***'); await c.close(); return 2
            await c.close()

        c = await C()
        await c.command_text('UIKEY F10', timeout=15)                      # clean Exit: flush log
        await asyncio.sleep(8)
        for pid in await pids(c):
            await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % pid)
        txt = await ex(c, 'cmd /c type "%s\\bench.log" 2>nul' % UT, 30)
        open(os.path.join(a.outdir, 'ut_%s.log' % a.renderer), 'w').write(txt)
        # UT prints a timedemo line for EVERY timedemo segment, including a 3-frame
        # artifact from the menu/intro before the real demo starts. Take the run
        # with the most frames, not the first match.
        runs = re.findall(r'([\d.]+) frames rendered in ([\d.]+) seconds\.\s*Min ([\d.]+) '
                          r'Max ([\d.]+) Avg ([\d.]+)', txt)
        m = max(runs, key=lambda r: float(r[0])) if runs else None
        gl = next((l.split('GL_RENDERER):', 1)[1].strip()
                   for l in txt.splitlines() if 'GL_RENDERER' in l), None)
        if gl: print('  GL_RENDERER: %s' % gl)
        if m:
            print('RESULT %s %s %s -> avg %s fps (min %s, max %s, %s frames in %ss)'
                  % (a.renderer, a.res, 'vsync' if a.vsync else 'unlimited',
                     m[4], m[2], m[3], m[0], m[1]))
            return 0
        print('RESULT %s %s -> NO fps line (log %d bytes)' % (a.renderer, a.res, len(txt)))
        return 1
    finally:
        try:
            await c.send_command('UPLOAD %s\\UnrealTournament.ini' % UT, binary_payload=ini0)
            await c.send_command('UPLOAD %s\\User.ini' % UT, binary_payload=usr0)
            await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
            print('[UT ini/User.ini and display restored]')
            await c.close()
        except Exception:
            pass

sys.exit(asyncio.run(main()))
