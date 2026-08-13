#!/usr/bin/env python3
"""RtCW wolfbench timedemo on the V5 6000 box, forced onto OUR ICD.

RtCW's `r_glDriver` is CVAR_LATCH and this GOG build pins it to the vintage 2001
Wicked3D wrapper it ships as `gl\\openglv5.dll` -- setting the cvar on the command
line, in wolfconfig_mp.cfg, and even making that file read-only all failed to move
it (GL_VENDOR stayed METABYTE/WICKED3D). So instead of fighting the cvar, we put
OUR ICD *at the path it insists on*: back up gl\\openglv5.dll and drop
system32\\3dfxogl.dll in its place. The backup is always restored.

`--maxfps` is on by default: this board resets under unrestricted frame rates
(V56K-SLI-FINDINGS S18), and RtCW's timedemo runs flat out otherwise.

  bench-rtcw.py [--mode 6] [--maxfps 85] [--no-icd]
"""
import argparse, asyncio, json, re, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP = '192.168.1.133'
RT = r'C:\GOG Games\Return to Castle Wolfenstein'
EXE = 'WolfMP.exe'
DEMO = 'wolfbench'
LOCAL_DEMO = '/mnt/c/development/retro-agent/benchmarks/demos_wolfbench.dm_60'
ICD = r'C:\WINDOWS\system32\3dfxogl.dll'
MODE_RES = {3: '640x480', 4: '800x600', 6: '1024x768', 8: '1280x1024'}


async def C(retries=8, delay=10):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: raise
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=25):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def pids(c, name):
    d = (await c.send_command('PROCLIST', timeout=20))[1].decode('ascii', 'replace')
    return [p['pid'] for p in json.loads(d) if p['name'].lower() == name.lower()]


async def uptime(c):
    return json.loads((await c.send_command('SYSINFO', timeout=15))[1].decode())['uptime_seconds']


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--mode', type=int, default=6)
    ap.add_argument('--maxfps', type=int, default=85, help='0 = unlimited (resets this board)')
    ap.add_argument('--no-icd', action='store_true', help='leave the stock Wicked3D wrapper in place')
    a = ap.parse_args()
    res = MODE_RES.get(a.mode, '1024x768'); w, h = res.split('x')
    log = r'%s\Main\rtcwconsole.log' % RT
    gl = r'%s\gl\openglv5.dll' % RT
    bak = gl + '.v56kprev'

    c = await C()
    before = await uptime(c)
    for pid in await pids(c, EXE):
        await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % pid)

    # stage the demo if absent
    chk = await ex(c, 'cmd /c if exist "%s\\main\\demos\\%s.dm_60" echo Y' % (RT, DEMO))
    if 'Y' not in chk:
        await ex(c, 'cmd /c mkdir "%s\\main\\demos" 2>nul & echo ok' % RT)
        await c.send_command(r'UPLOAD %s\main\demos\%s.dm_60' % (RT, DEMO),
                             binary_payload=open(LOCAL_DEMO, 'rb').read())
        print('staged %s.dm_60' % DEMO)

    swapped = False
    try:
        if not a.no_icd:
            # r_glDriver is latched to gl\openglv5.dll -- so put OUR ICD there.
            await ex(c, 'cmd /c if not exist "%s" copy /Y "%s" "%s" >nul & echo ok' % (bak, gl, bak))
            await ex(c, 'cmd /c copy /Y "%s" "%s" >nul & echo ok' % (ICD, gl))
            swapped = True
            print('staged our ICD as gl\\openglv5.dll (backup kept)')

        await ex(c, 'cmd /c del /f /q "%s" 2>nul & echo ok' % log)
        await c.command_text('DISPLAYCFG set %s %s 16 75' % (w, h), timeout=25)
        await asyncio.sleep(2)
        cap = ('+set com_maxfps %d ' % a.maxfps) if a.maxfps else ''
        await ex(c, ('cmd /c cd /d "%s" ^&^& start "" %s +set r_mode %d +set r_fullscreen 1 '
                     '+set r_colorbits 16 +set sv_pure 0 +set s_initsound 0 +set logfile 2 '
                     '%s+set timedemo 1 +demo %s') % (RT, EXE, a.mode, cap, DEMO), 20)
        print('RtCW %s maxfps=%s: running %s...' % (res, a.maxfps or 'unlimited', DEMO))
        await c.close()

        fps = rend = None
        for waited in range(0, 200, 20):
            await asyncio.sleep(20)
            try:
                c = await C(retries=2, delay=8)
            except Exception:
                print('  *** box gone -> REBOOT ***'); return 2
            if await uptime(c) < before:
                print('  *** REBOOTED during the timedemo ***'); await c.close(); return 2
            # copy first: RtCW holds the log open
            await ex(c, 'cmd /c copy /Y "%s" "%s.copy" >nul 2>nul & echo ok' % (log, log))
            txt = await ex(c, 'cmd /c type "%s.copy" 2>nul' % log, 25)
            rend = next((l.split('GL_RENDERER:', 1)[1].strip()
                         for l in txt.splitlines() if 'GL_RENDERER' in l), rend)
            m = re.search(r'(\d+) frames,?\s+([\d.]+) seconds:?\s+([\d.]+) fps', txt)
            if m:
                fps = m.group(3)
                open('/tmp/qa256/rtcw.log', 'w').write(txt)
                print('  result after %ds' % (waited + 20)); break
            await c.close()

        c = await C()
        for pid in await pids(c, EXE):
            await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % pid)
        if rend: print('  GL_RENDERER: %s' % rend)
        print('RESULT rtcw %s maxfps=%s -> %s fps' % (res, a.maxfps or 'unlimited', fps or 'NO RESULT'))
        return 0 if fps else 1
    finally:
        try:
            if swapped:
                await ex(c, 'cmd /c copy /Y "%s" "%s" >nul & echo ok' % (bak, gl))
                print('[stock openglv5.dll restored]')
            await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
            await c.close()
        except Exception:
            pass

sys.exit(asyncio.run(main()))
