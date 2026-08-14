#!/usr/bin/env python3
"""Quake 3 timedemo on .143, for the kernel-display-driver A/B (ours vs AmigaMerlin).

Identity always comes from the game's own GL_RENDERER/GL_VERSION line, never from
the flag we passed -- that is exactly the mistake that mislabelled 31 historical
.143 rows, and it caught a silent ICD fallback earlier today.

FX_GLIDE_SWAPINTERVAL=0 because our glide3x ships vsync ON by default (a
deliberate source invariant); leaving it on measures the refresh rate, not the
driver.
"""
import argparse, asyncio, json, re, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP  = '192.168.1.143'
Q3  = r'C:\Quake III Arena\Quake3'
HOME = r'C:\q3home'
LOG = HOME + r'\baseq3\qconsole.log'
MODES = {3: '640x480', 4: '800x600', 6: '1024x768'}


async def C(retries=20, delay=10):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: return None
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=30):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def running(c):
    d = (await c.send_command('PROCLIST', timeout=20))[1].decode('ascii', 'replace')
    return [p['pid'] for p in json.loads(d) if p['name'].lower() == 'quake3.exe']


async def kill(c):
    """Exit Quake 3 CLEANLY. Never taskkill /f a fullscreen Glide app.

    FINDINGS.md:674 says it outright -- a force-kill lands mid-FIFO-packet, and
    FINDINGS.md:1333 records it spawning Windows Error Reporting. It also robs the
    game of its chance to restore the display mode, which is what leaves the
    desktop stranded at 640x480x16/60Hz and reads to a human as "Quake 3 crashed".
    Verified 2026-08-14: F10 -> `quit` exits with no crash reporter and the desktop
    restores itself; taskkill does neither.

    F10 is bound to `quit` by q3quit.cfg, shipped at launch."""
    if not await running(c):
        return
    try:
        await c.command_text('UIKEY f10', timeout=15)
    except Exception:
        pass
    for _ in range(8):                       # give it up to ~16s to shut down
        await asyncio.sleep(2)
        if not await running(c):
            return
    # last resort only: it is wedged and would block the next run anyway
    for pid in await running(c):
        print('    [warn] clean quit failed, force-killing pid %d' % pid)
        await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % pid)
    await asyncio.sleep(3)


DESKTOP = (1024, 768, 32, 100)      # the box's normal desktop mode


async def set_desktop(c):
    """Pin the desktop mode before and after every run.

    This is not cosmetic. A fullscreen Glide app that does not exit cleanly leaves
    the desktop in the GAME's mode, and the next run then starts from a different
    desktop state -- which measurably moves the result (same driver, same mode 3:
    73.4 fps from a 1024x768x32 desktop vs 88.6 from a stranded 640x480x16 one).
    Any A/B that does not pin this is comparing desktop states as much as drivers.
    """
    try:
        await c.command_text('DISPLAYCFG set %d %d %d %d' % DESKTOP, timeout=40)
        await asyncio.sleep(3)
    except Exception:
        pass


async def run(mode, demo, wait):
    c = await C()
    if not c: return None, 'box down', None
    before = json.loads((await c.send_command('SYSINFO', timeout=15))[1].decode())['uptime_seconds']
    await kill(c)
    await set_desktop(c)
    await ex(c, 'cmd /c del /f /q "%s" 2>nul & echo ok' % LOG, 20)
    # F10 -> clean shutdown; see kill() for why this is not optional
    await c.send_command(r'UPLOAD %s\baseq3\q3quit.cfg' % HOME,
                         binary_payload=b'bind F10 "quit"\r\n')
    await ex(c, ('cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=0^&^& start "" quake3.exe '
                 '+set r_glDriver 3dfxogl +set r_mode %d +set r_fullscreen 1 +set r_colorbits 16 '
                 '+set fs_homepath %s +set s_initsound 0 +set com_introPlayed 1 +set sv_pure 0 '
                 '+set logfile 2 +exec q3quit.cfg +set timedemo 1 +demo %s') % (Q3, mode, HOME, demo), 25)
    print('    launched (%s, demo %s)' % (MODES.get(mode, mode), demo))
    await c.close()

    fps = rend = ver = None
    for _ in range(0, wait, 15):
        await asyncio.sleep(15)
        c = await C(3, 8)
        if not c:
            print('    *** box gone ***'); return None, 'BOX GONE', None
        up = json.loads((await c.send_command('SYSINFO', timeout=15))[1].decode())['uptime_seconds']
        if up < before:
            print('    *** REBOOTED ***'); await c.close(); return None, 'REBOOTED', None
        await ex(c, 'cmd /c copy /Y "%s" "%s.copy" >nul 2>nul & echo ok' % (LOG, LOG), 20)
        txt = await ex(c, 'cmd /c type "%s.copy" 2>nul' % LOG, 25)
        for line in txt.splitlines():
            if 'GL_RENDERER' in line: rend = line.split(':', 1)[-1].strip()
            if 'GL_VERSION'  in line: ver  = line.split(':', 1)[-1].strip()
        m = re.search(r'(\d+) frames,?\s+([\d.]+) seconds:?\s+([\d.]+) fps', txt)
        if m:
            fps = float(m.group(3)); break
        if 'Couldn\'t open' in txt or 'not found' in txt.lower():
            open('/tmp/qa256/q3_fail.log', 'w').write(txt)
        await c.close()
    c = await C()
    if c:
        await kill(c)
        await set_desktop(c)          # always leave the desktop as we found it
        await c.close()
    return fps, rend, ver


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--mode', type=int, default=3)
    ap.add_argument('--demo', default='four')
    ap.add_argument('--runs', type=int, default=4)
    ap.add_argument('--wait', type=int, default=180)
    ap.add_argument('--label', default='')
    a = ap.parse_args()
    print('Q3 timedemo .143  %s  demo=%s  %s' % (MODES.get(a.mode, a.mode), a.demo, a.label))
    got = []
    for i in range(1, a.runs + 1):
        print('  run %d/%d' % (i, a.runs))
        fps, rend, ver = await run(a.mode, a.demo, a.wait)
        print('    -> %s fps   GL_RENDERER=%s' % (fps if fps else 'NO RESULT', rend))
        if fps: got.append(fps)
        if rend in ('BOX GONE', 'REBOOTED'): break
    if got:
        warm = got[1:] or got
        print('RESULT %s %s -> warm mean %.1f fps  (all: %s)'
              % (a.label, MODES.get(a.mode, a.mode), sum(warm)/len(warm),
                 ', '.join('%.1f' % g for g in got)))
        return 0
    print('RESULT %s -> NO RESULT' % a.label)
    return 1

sys.exit(asyncio.run(main()))
