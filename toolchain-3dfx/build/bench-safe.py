#!/usr/bin/env python3
"""Thermally-considerate benchmark/stability runner for the Voodoo 5 6000 (.133).

Written after a session where the benchmark harness itself was part of the problem:
back-to-back unlimited timedemos (including 1280x1024, no cooldown) rebooted a board
that plays games perfectly well. This board's own design notes warn its HiNT bridge
runs hot and it pulls ~80W through a 6-pin input.

Rules encoded here:
  * COOLDOWN between runs (default 90s idle at the desktop) -- never back-to-back.
  * Stability testing uses a VSYNC-LIMITED run; only --perf uses an unlimited
    timedemo, and then one resolution at a time.
  * Every run is bracketed by an uptime check, so a reboot is detected and the
    sequence STOPS instead of continuing to hammer a board that just reset.
  * Aborts if the box fails to come back, rather than looping on a sick machine.

  bench-safe.py stability [--modes 3,6] [--cooldown 90]
  bench-safe.py perf      [--modes 3,6,8] [--cooldown 120]
"""
import argparse, asyncio, json, re, sys
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP = '192.168.1.133'
Q3 = r'C:\Games\Quake III Arena\Quake3'
QUIESCE = ['rotate_wall.exe', 'daemon.exe', 'wuauclt.exe', '3dfxMan.exe', 'wmiprvse.exe']

async def C(retries=8, delay=10):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: raise
            await asyncio.sleep(delay)

async def uptime(c):
    return json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']

async def run_q3(mode, vsync):
    """One Q3 timedemo. Returns (fps, rebooted)."""
    c = await C()
    before = await uptime(c)
    for img in QUIESCE:
        try: await c.command_text('EXEC cmd /c taskkill /f /im %s 2>nul' % img, timeout=20)
        except Exception: pass
    await c.command_text(r'EXECW 12 cmd /c del /f /q C:\q3home\baseq3\qconsole.log 2>nul & echo ok', timeout=32)
    # swapinterval 1 = vsync-limited (gentle); 0 = flat out (perf only)
    sw = '1' if vsync else '0'
    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=%s^&^& start "" quake3.exe '
           r'+set r_glDriver 3dfxogl +set r_mode %s +set r_fullscreen 1 +set r_colorbits 16 '
           r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1 '
           r'+set r_swapInterval %s +set nextdemo quit +set timedemo 1 +demo four') % (Q3, sw, mode, sw)
    await c.command_text(cmd, timeout=40)
    await c.close()

    fps = None
    for _ in range(24):
        await asyncio.sleep(10)
        try:
            c = await C(retries=2, delay=8)
        except Exception:
            return None, True                     # box gone -> reboot
        try:
            up = await uptime(c)
            if up < before:
                await c.close(); return None, True
            t = await c.command_text(r'EXECW 20 cmd /c type C:\q3home\baseq3\qconsole.log 2>nul', timeout=45)
        except Exception:
            await c.close(); continue
        m = re.search(r'(\d+) frames, ([\d.]+) seconds: ([\d.]+) fps', t)
        await c.close()
        if m: fps = m.group(3); break

    c = await C()
    try:
        await c.command_text('EXECW 20 cmd /c taskkill /f /im quake3.exe 2>nul & echo ok', timeout=45)
        await asyncio.sleep(3)
        rebooted = (await uptime(c)) < before
        await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
    finally:
        await c.close()
    return fps, rebooted

async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('what', choices=['stability', 'perf'])
    ap.add_argument('--modes', default='')
    ap.add_argument('--cooldown', type=int, default=0)
    a = ap.parse_args()
    vsync = (a.what == 'stability')
    modes = (a.modes or ('3,6' if vsync else '3,6,8')).split(',')
    cool = a.cooldown or (90 if vsync else 120)

    print('%s run: modes=%s vsync=%s cooldown=%ds' % (a.what, modes, vsync, cool))
    for i, mode in enumerate(modes):
        if i:
            print('  cooling %ds...' % cool); await asyncio.sleep(cool)
        fps, rebooted = await run_q3(mode, vsync)
        if rebooted:
            print('  mode %s: *** REBOOTED *** — STOPPING (treat as thermal/power first)' % mode)
            return 1
        print('  mode %s: %s fps, no reboot' % (mode, fps))
    print('all runs completed with no reboot')
    return 0

sys.exit(asyncio.run(main()))
