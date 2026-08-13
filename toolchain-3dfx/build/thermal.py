#!/usr/bin/env python3
"""Thermal / power telemetry for the Voodoo 5 6000 box (.133 "P3-DUAL").

WHAT CAN AND CANNOT BE MEASURED
-------------------------------
The VSA-100 has NO on-die thermal sensor, and there is no sensor anywhere on the
V5 6000 board -- SpeedFan's full ISA + PIIX4-SMBus sweep finds nothing on the
card. **There is no GPU temperature on this machine and there never will be.**

What the Tyan 440BX motherboard *does* expose is a real hardware monitor, and it
is enough to characterise the card indirectly:

  LM79  on ISA        $290  -- 1 temp (board), 3 fan tachs, 7 voltage rails
  LM75  on Intel SMBus $4C  -- temp
  LM75  on Intel SMBus $4D  -- temp
  Samsung SSD via AdvSMART  -- drive temp

The two channels that actually matter for this board's reboot problem:

  * case/board temperature -- the ~80 W the card dumps into the case shows up
    here with a lag, a usable proxy for cumulative duty cycle
    (see memory `v56k-thermal-envelope`, V56K-SLI-FINDINGS.md 11).
  * **+12V rail (V12)** -- the V5 6000 pulls ~80 W through a 6-pin input. If the
    resets are power rather than heat, a sag here under 4-chip load is the
    direct evidence. Idle reference measured 2026-08-12: 12.22 V.

MECHANISM
---------
SpeedFan 4.49 is installed on the box and its signed kernel driver
(speedfan.sys) is what actually touches the ISA/SMBus ports -- we do not do port
I/O ourselves. Logging is driven entirely from its plain-text config:
`speedfansens.cfg` (`logged=true` per reading, names set to the CSV headers
below) and `speedfanparams.cfg` (`LogEnabled=true`). SpeedFan appends a
tab-separated `SFLog<YYYYMMDD>.csv` every ~3 s, which we download.

  NOTE: SpeedFan REWRITES BOTH CONFIGS WHEN IT EXITS. Kill it before uploading a
  config or the change is silently lost.

Columns: Seconds SysISA Temp4C Temp4D HD0 Fan1 Fan2 Fan3
         COREA COREB V3_3 V5 V12 Vn12 Vn5
(`Seconds` is seconds-since-midnight on the box, not uptime.)

USAGE
  thermal.py ensure                 # start SpeedFan + verify it is logging
  thermal.py now                    # print the latest sample
  thermal.py window <startSec>      # summarise min/max/delta since a mark
  thermal.py stop                   # close SpeedFan (leave the box clean)

Typical bracket around a benchmark:
  s = subprocess.check_output(['thermal.py','now','--seconds-only'])
  ...run the timedemo...
  subprocess.run(['thermal.py','window', s])
"""
import argparse, asyncio, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP = '192.168.1.133'
SFDIR = r'C:\Program Files\SpeedFan'
COLS = ['Seconds', 'SysISA', 'Temp4C', 'Temp4D', 'HD0', 'Fan1', 'Fan2', 'Fan3',
        'COREA', 'COREB', 'V3_3', 'V5', 'V12', 'Vn12', 'Vn5']
# Channels worth alarming on. The +12V rail feeds the card's 6-pin.
LIMITS = {'V12': (11.4, 12.9), 'V5': (4.75, 5.25), 'V3_3': (3.13, 3.47)}


async def C(retries=6, delay=8):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=10); return c
        except Exception:
            if i == retries - 1: raise
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=25):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def running(c):
    import json
    d = (await c.send_command('PROCLIST', timeout=15))[1].decode('ascii', 'replace')
    return [p['pid'] for p in json.loads(d) if p['name'].lower() == 'speedfan.exe']


async def fetch(c):
    """Return the log as a list of dicts, newest last."""
    t = await ex(c, 'cmd /c dir /b "%s\\SFLog*.csv"' % SFDIR)
    names = [l.strip() for l in t.splitlines() if l.strip().lower().endswith('.csv')]
    if not names:
        return []
    raw = await c.command_binary(r'DOWNLOAD %s\%s' % (SFDIR, sorted(names)[-1]), timeout=120)
    rows = []
    for line in raw.decode('latin-1').splitlines():
        f = line.rstrip().split('\t')
        if len(f) < len(COLS) or f[0] == 'Seconds':
            continue
        try:
            rows.append({k: float(v) for k, v in zip(COLS, f)})
        except ValueError:
            continue
    return rows


def fmt(r):
    return ('t=%d  board=%.0fC  smb4C=%.0fC  smb4D=%.0fC  hd=%.0fC  '
            'fans=%.0f/%.0f/%.0f  +12V=%.2f  +5V=%.2f  +3.3V=%.2f  Vcore=%.2f/%.2f'
            % (r['Seconds'], r['SysISA'], r['Temp4C'], r['Temp4D'], r['HD0'],
               r['Fan1'], r['Fan2'], r['Fan3'], r['V12'], r['V5'], r['V3_3'],
               r['COREA'], r['COREB']))


def check(rows):
    """Flag any rail excursion -- this is the point of logging voltages at all."""
    bad = []
    for ch, (lo, hi) in LIMITS.items():
        vals = [r[ch] for r in rows]
        if not vals:
            continue
        if min(vals) < lo or max(vals) > hi:
            bad.append('%s ranged %.2f..%.2f (limit %.2f..%.2f)' % (ch, min(vals), max(vals), lo, hi))
    return bad


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('cmd', choices=['ensure', 'now', 'window', 'stop'])
    ap.add_argument('start', nargs='?', type=float)
    ap.add_argument('--seconds-only', action='store_true')
    a = ap.parse_args()
    c = await C()
    try:
        if a.cmd == 'stop':
            await ex(c, 'cmd /c taskkill /f /im speedfan.exe 2>nul & echo ok')
            print('speedfan stopped')
            return 0

        if a.cmd == 'ensure':
            if not await running(c):
                await ex(c, 'cmd /c start "" "%s\\speedfan.exe"' % SFDIR, 15)
                await asyncio.sleep(30)
            rows = await fetch(c)
            if not rows:
                print('!! SpeedFan is not logging. Check LogEnabled=true in speedfanparams.cfg '
                      'and logged=true in speedfansens.cfg (it rewrites both on exit).')
                return 1
            print('logging OK, %d samples' % len(rows)); print(' ', fmt(rows[-1]))
            return 0

        rows = await fetch(c)
        if not rows:
            print('no samples -- run "thermal.py ensure" first'); return 1

        if a.cmd == 'now':
            if a.seconds_only:
                print(int(rows[-1]['Seconds'])); return 0
            print(fmt(rows[-1])); return 0

        # window
        sel = [r for r in rows if r['Seconds'] >= (a.start or 0)]
        if not sel:
            print('no samples after t=%s' % a.start); return 1
        print('window: %d samples, %.0fs span' % (len(sel), sel[-1]['Seconds'] - sel[0]['Seconds']))
        for ch in ('SysISA', 'Temp4C', 'Temp4D', 'V12', 'V5', 'V3_3'):
            v = [r[ch] for r in sel]
            print('  %-7s min=%.2f max=%.2f delta=%+.2f' % (ch, min(v), max(v), v[-1] - v[0]))
        bad = check(sel)
        print('  RAIL EXCURSION: ' + '; '.join(bad) if bad else '  all rails within tolerance')
        return 0
    finally:
        try: await c.close()
        except Exception: pass

sys.exit(asyncio.run(main()))
