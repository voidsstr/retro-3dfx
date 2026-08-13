#!/usr/bin/env python3
"""Medal of Honor: Allied Assault on the V5 6000 box, on OUR ICD.

Prereqs this script handles itself:
  * CD1 must be mounted -- MOHAA is SafeDisc (drvmgt.dll/secdrv.sys) and checks for
    the disc. The Disc 1 ISO lives on the box at the Administrator's Desktop; it is
    staged to C:\\ISO\\MOHAA_CD1.iso because DAEMON Tools 3.47 will not take a path
    with spaces, then mounted with `daemon.exe -mount 0,<image>`. daemon.exe stays
    resident, so it MUST be launched detached (`start ""`) or EXECW tree-kills it
    and undoes the mount.
  * Our ICD is staged as MOHAA\\opengl32.dll -- the game directory wins the DLL
    search order, so MOHAA loads it instead of the system one.

Traps encoded (from retro-agent's driver-bench skill, confirmed here):
  * MOHAA's Ritual build CRASHES on command-line `+set` of logfile/r_mode/
    r_gldriver. Everything must go through a cfg run with `+exec`.
  * MOHAA.exe is a launcher front-end; after an unclean exit it shows a
    "Play in Safe Mode / Play in Normal Mode" dialog that blocks startup.

  bench-mohaa.py record      # stage a timedemo demo (once)
  bench-mohaa.py timedemo    # play it back and report fps
  bench-mohaa.py verify      # just confirm it renders on our ICD
"""
import argparse, asyncio, json, re, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP = '192.168.1.133'
M = r'C:\Program Files\EA GAMES\MOHAA'
DT = r'C:\Program Files\D-Tools'
DTS = r'C:\PROGRA~1\D-Tools'
ISO_SRC = r'C:\Documents and Settings\Administrator\Desktop\Medal of Honor Allied Assault (2002) - Disc 1.iso'
ISO = r'C:\ISO\MOHAA_CD1.iso'
ICD = r'C:\WINDOWS\system32\3dfxogl.dll'
DEMO = 'mohbench'
MAP = 'dm/mohdm1'
PLAY_NORMAL_BTN = (512, 489)     # "Play in Normal Mode" @1024x768


async def C(retries=25, delay=12):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: return None
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=25):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def kill(c):
    d = (await c.send_command('PROCLIST', timeout=20))[1].decode('ascii', 'replace')
    for p in json.loads(d):
        if 'mohaa' in p['name'].lower():
            await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % p['pid'])


async def mount_cd(c):
    """Mount CD1. Returns the drive letter, or None."""
    dl = await ex(c, 'cmd /c wmic logicaldisk where drivetype=5 get deviceid,volumename', 30)
    for line in dl.splitlines():
        m = re.match(r'\s*([D-Z]):\s+(MOHAA\S*)\s*$', line)
        if m:
            return m.group(1)                      # already mounted
    if 'PRESENT' not in await ex(c, 'cmd /c if exist "%s" echo PRESENT' % ISO, 20):
        await ex(c, 'cmd /c mkdir C:\\ISO 2>nul & echo ok', 20)
        await ex(c, 'cmd /c copy /Y "%s" "%s" >nul & echo done' % (ISO_SRC, ISO), 300)
    await ex(c, 'cmd /c start "" /d "%s" %s\\daemon.exe' % (DT, DTS), 15)
    await asyncio.sleep(6)
    await ex(c, 'cmd /c start "" /d "%s" %s\\daemon.exe -mount 0,%s' % (DT, DTS, ISO), 15)
    await asyncio.sleep(10)
    dl = await ex(c, 'cmd /c wmic logicaldisk where drivetype=5 get deviceid,volumename', 30)
    for line in dl.splitlines():
        m = re.match(r'\s*([D-Z]):\s+(MOHAA\S*)\s*$', line)
        if m:
            return m.group(1)
    return None


async def launch(c, cfg_text, label):
    await c.send_command(r'UPLOAD %s\main\moh_bench.cfg' % M, binary_payload=cfg_text.encode('latin-1'))
    await ex(c, 'cmd /c del /f /q "%s\\main\\qconsole.log" 2>nul & echo ok' % M, 20)
    await ex(c, 'cmd /c cd /d "%s" ^&^& start "" MOHAA.exe +exec moh_bench.cfg' % M, 20)
    print('  %s: MOHAA launched' % label)
    await c.close()
    await asyncio.sleep(25)
    c = await C()
    if c:
        try: await c.command_text('UICLICK %d %d' % PLAY_NORMAL_BTN, timeout=15)
        except Exception: pass
    return c


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('what', choices=['record', 'timedemo', 'verify'])
    ap.add_argument('--wait', type=int, default=110)
    a = ap.parse_args()

    c = await C()
    if not c:
        print('box down'); return 2
    before = json.loads((await c.send_command('SYSINFO', timeout=15))[1].decode())['uptime_seconds']
    await kill(c)

    drive = await mount_cd(c)
    print('CD1 mounted at %s:' % drive if drive else '!! CD1 NOT mounted (SafeDisc check will fail)')
    await ex(c, 'cmd /c copy /Y "%s" "%s\\opengl32.dll" >nul & echo ok' % (ICD, M), 40)
    print('our ICD staged as MOHAA\\opengl32.dll')

    if a.what == 'record':
        cfg = ('seta logfile 2\r\nseta r_fullscreen 1\r\n'
               'map %s\r\nwait 1200\r\nrecord %s\r\nwait 2400\r\nstoprecord\r\nwait 60\r\nquit\r\n'
               % (MAP, DEMO))
    elif a.what == 'timedemo':
        cfg = ('seta logfile 2\r\nseta r_fullscreen 1\r\nseta timescale 1\r\n'
               'timedemo 1\r\ndemo %s\r\nwait 3000\r\nquit\r\n' % DEMO)
    else:
        cfg = 'seta logfile 2\r\n'

    c = await launch(c, cfg, a.what)
    if not c:
        print('  box gone -> REBOOT'); return 2
    waited = 0
    while waited < a.wait:
        await asyncio.sleep(20); waited += 20
        c2 = await C(3, 8)
        if not c2:
            print('  *** box gone -> REBOOT ***'); return 2
        up = json.loads((await c2.send_command('SYSINFO', timeout=15))[1].decode())['uptime_seconds']
        if up < before:
            print('  *** REBOOTED ***'); await c2.close(); return 2
        c = c2
        await c.close()
    c = await C()
    txt = await ex(c, 'cmd /c type "%s\\main\\qconsole.log" 2>nul' % M, 40)
    open('/tmp/qa256/mohaa_%s.log' % a.what, 'w').write(txt)
    print('  qconsole.log %d bytes' % len(txt))
    gl = next((l.split('GL_RENDERER:', 1)[1].strip() for l in txt.splitlines() if 'GL_RENDERER' in l), None)
    if gl: print('  GL_RENDERER: %s' % gl)
    runs = re.findall(r'(\d+) frames,?\s+([\d.]+) seconds:?\s+([\d.]+) fps', txt)
    if runs:
        r = max(runs, key=lambda x: int(x[0]))
        print('RESULT mohaa -> %s fps (%s frames in %ss)' % (r[2], r[0], r[1]))
    dchk = await ex(c, 'cmd /c if exist "%s\\main\\%s.dm_*" echo DEMO-PRESENT' % (M, DEMO), 20)
    print(' ', dchk.strip()[:40] or '(no demo yet)')
    await kill(c); await c.close()
    return 0

sys.exit(asyncio.run(main()))
