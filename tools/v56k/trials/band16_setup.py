#!/usr/bin/env python3
"""Persistent trial - SETUP: force Glide's SLI band height to 16 and PROVE it took.

3dfx's own "Video SLI AA Configs.xls" states the whole configuration table
assumes "16-high SLI bands", and its 4-chip analog-SLI row gives literal mask
values 0x30 / 0x10 / 0x20 / 0x30 = (3,1,2,3) << 4, i.e. sli_nLinesLog2 = 4.
Our hardware runs band height 8 (Glide reports band=3, renderMask=0x18 = 3<<3).

This forces 16 and verifies from Glide's own SLICTRL diagnostic that the change
actually reached the hardware, BEFORE anyone is asked to look at the monitor.
Leaves the game running - tear down with trial_teardown.py after the verdict.
"""
import sys, asyncio, json

sys.path.insert(0, "/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection

IP = '192.168.1.191'
Q3 = r'C:\Games\Quake3-TeamArena'
BAND = sys.argv[1] if len(sys.argv) > 1 else '16'
MODE = sys.argv[2] if len(sys.argv) > 2 else '3'
GK = r'HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\Glide'


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
        print('ABORT: box unreachable'); return 2

    for p in [p['pid'] for p in json.loads((await c.send_command('PROCLIST', timeout=15))[1]
              .decode('ascii', 'replace')) if p['name'].lower() == 'quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok' % p, timeout=45)

    # force the band height on the GLIDE side (the display-driver knob is not in
    # this code path - a Glide app makes the display driver release the hardware)
    for name in ('FX_GLIDE_FORCE_SLI_BAND_HEIGHT', 'FX_GLIDE_SLI_BAND_HEIGHT'):
        await c.command_text('EXECW 30 cmd /c reg add "%s" /v %s /t REG_SZ /d %s /f'
                             % (GK, name, BAND), timeout=60)
    print('forced Glide band height = %s' % BAND, flush=True)

    await c.command_text(r'EXECW 15 cmd /c del /f /q C:\glide_sli.log 2>nul & echo ok', timeout=35)
    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SLI_LOG=C:\glide_sli.log^&^& '
           r'set FX_GLIDE_SWAPINTERVAL=1^&^& start "" quake3.exe '
           r'+set r_glDriver 3dfxogl +set r_mode %s +set r_fullscreen 1 +set r_colorbits 16 '
           r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1') % (Q3, MODE)
    await c.command_text(cmd, timeout=40)
    await c.close()
    print('Q3 launched (mode %s); settling 28s' % MODE, flush=True)
    await asyncio.sleep(28)

    c = await C()
    log = await c.command_text(r'EXECW 30 cmd /c type C:\glide_sli.log 2>nul', timeout=60)
    print('\n--- Glide SLICTRL (machine-readable proof the setting took) ---', flush=True)
    print(log.strip() or '  (empty)', flush=True)

    ok = False
    for line in log.splitlines():
        if 'SLICTRL' in line and 'band=' in line:
            try:
                b = int(line.split('band=')[1].split()[0])
                want = {2: 1, 4: 2, 8: 3, 16: 4, 32: 5, 64: 6, 128: 7}.get(int(BAND))
                ok = (b == want)
                print('\n  band field = %d  (want %s for %s lines)  -> %s'
                      % (b, want, BAND, 'TOOK' if ok else 'DID NOT TAKE'), flush=True)
            except Exception:
                pass
            break

    await c.close()
    if not ok:
        print('\n*** the band height did NOT change - not worth looking at the monitor ***', flush=True)
        return 1
    print('\n>>> TRIAL IS UP AND STAYS UP. Nothing will close it. <<<', flush=True)
    return 0


sys.exit(asyncio.run(main()))
