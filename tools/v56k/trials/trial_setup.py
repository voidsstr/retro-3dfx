#!/usr/bin/env python3
"""Persistent visual trial - SETUP half.  Launches Q3 fullscreen, applies a
per-chip register state, prints what is on screen, and EXITS leaving everything
running.  Nothing here closes the game or restores the mode: the operator's
verdict is worthless if the screen changes before they answer.

  trial_setup.py yorigin [mode]    give slaves the master's miscInit0 (Y-origin)
  trial_setup.py fullgeom [mode]   give slaves the master's FULL scanout geometry
  trial_setup.py baseline [mode]   launch only, change nothing

Tear down explicitly, afterwards, with trial_teardown.py.
"""
import sys, asyncio, json

sys.path.insert(0, "/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection

IP = '192.168.1.191'
Q3 = r'C:\Games\Quake3-TeamArena'
EXE = r'C:\RETRO_AGENT\v56k-deploy\fxscan2.exe'

WHAT = sys.argv[1] if len(sys.argv) > 1 else 'yorigin'
MODE = sys.argv[2] if len(sys.argv) > 2 else '3'

# name -> IO offset, for the registers the ring showed the slaves never receive
GEOM = [
    ('miscInit0',        '0x010'),
    ('vidScreenSize',    '0x098'),
    ('vidOvlEndCoord',   '0x0A0'),
    ('vidDesktopStart',  '0x0E4'),
    ('vidDesktopStride', '0x0EC'),
    ('lfbMemoryConfig',  '0x00C'),
]


async def C(retries=6, delay=6):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898)
            await c.connect('retro-agent-secret', timeout=10)
            return c
        except Exception:
            if i == retries - 1:
                return None
            await asyncio.sleep(delay)
    return None


async def main():
    c = await C()
    if c is None:
        print('ABORT: box unreachable')
        return 2
    up = json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']
    print('box up (uptime %ds)' % up, flush=True)

    for p in [p['pid'] for p in json.loads((await c.send_command('PROCLIST', timeout=15))[1]
              .decode('ascii', 'replace')) if p['name'].lower() == 'quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok' % p, timeout=45)

    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=1^&^& start "" quake3.exe '
           r'+set r_glDriver 3dfxogl +set r_mode %s +set r_fullscreen 1 +set r_colorbits 16 '
           r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1') % (Q3, MODE)
    await c.command_text(cmd, timeout=40)
    await c.close()
    print('Q3 launched (mode %s); settling 25s' % MODE, flush=True)
    await asyncio.sleep(25)

    c = await C()
    dump = await c.command_text('EXECW 60 cmd /c %s dump' % EXE, timeout=120)
    vals = {}
    for line in dump.splitlines():
        t = line.split()
        if len(t) >= 5 and t[0] in dict(GEOM):
            vals[t[0]] = t[1:5]

    if WHAT == 'baseline':
        print('\nBASELINE - nothing changed. Per-chip state:', flush=True)
        for n, _ in GEOM:
            if n in vals:
                print('  %-18s %s' % (n, ' '.join(vals[n])), flush=True)
    else:
        todo = GEOM[:1] if WHAT == 'yorigin' else GEOM
        print('\napplying master values to chips 1,2,3:', flush=True)
        for name, off in todo:
            if name not in vals:
                continue
            master = vals[name][0]
            for chip in (1, 2, 3):
                await c.command_text('EXECW 40 cmd /c %s poke %d %s 0x%s 0'
                                     % (EXE, chip, off, master), timeout=90)
            print('  %-18s -> all chips 0x%s' % (name, master), flush=True)

    await c.close()
    print('\n>>> TRIAL IS UP AND WILL STAY UP. Nothing will close it. <<<', flush=True)
    print('    Run trial_teardown.py only after the verdict.', flush=True)
    return 0


sys.exit(asyncio.run(main()))
