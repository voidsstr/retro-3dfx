#!/usr/bin/env python3
"""Sweep the SLI hsync handover column (vidOverlayDudx, IO 0x0A4) with a
LIVENESS GATE around every step.

The first version of this sweep hard-froze the box (NIC dead, physical power
cycle).  Poking a live SLI scanout can wedge the video hardware, so here every
step is bracketed: the agent must answer before the hold and after it, and the
uptime must not go backwards.  If either check fails the sweep STOPS and names
the step that did it -- which is itself the diagnostic.
"""
import sys, asyncio, json

sys.path.insert(0, "/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection

IP = '192.168.1.191'
Q3 = r'C:\Games\Quake3-TeamArena'
EXE = r'C:\RETRO_AGENT\v56k-deploy\fxscan2.exe'
HOLD = int(sys.argv[1]) if len(sys.argv) > 1 else 25

STEPS = [
    ("1  all 0     baseline (the broken default)",         [0, 0, 0, 0]),
    ("2  all 160   W/4",                                   [160] * 4),
    ("3  all 320   W/2 (the 2-chip spec example)",          [320] * 4),
    ("4  all 480   3W/4",                                  [480] * 4),
    ("5  all 639   max ('Scott says set it to the max')",   [639] * 4),
    ("6  staggered 160/320/480/639",                       [160, 320, 480, 639]),
    ("7  master 320, slaves 0",                            [320, 0, 0, 0]),
    ("8  master 0, slaves 320",                            [0, 320, 320, 320]),
]


async def C(retries=4, delay=6, timeout=8):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898)
            await c.connect('retro-agent-secret', timeout=timeout)
            return c
        except Exception:
            if i == retries - 1:
                return None
            await asyncio.sleep(delay)
    return None


async def uptime():
    c = await C()
    if c is None:
        return None
    try:
        d = json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())
        return d['uptime_seconds']
    except Exception:
        return None
    finally:
        try:
            await c.close()
        except Exception:
            pass


async def kill_q3(c):
    pl = json.loads((await c.send_command('PROCLIST', timeout=15))[1].decode('ascii', 'replace'))
    for p in [p['pid'] for p in pl if p['name'].lower() == 'quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok' % p, timeout=45)


async def main():
    base = await uptime()
    if base is None:
        print('ABORT: box not reachable at start', flush=True)
        return 2
    print('box up (uptime %ds)' % base, flush=True)

    c = await C()
    await kill_q3(c)
    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=1^&^& start "" quake3.exe '
           r'+set r_glDriver 3dfxogl +set r_mode 3 +set r_fullscreen 1 +set r_colorbits 16 '
           r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1') % Q3
    await c.command_text(cmd, timeout=40)
    await c.close()
    print('Q3 up at 640x480, 4-way SLI. Sweep starts in 25s, %ds per step.\n' % HOLD, flush=True)
    await asyncio.sleep(25)

    for label, vals in STEPS:
        c = await C()
        if c is None:
            print('*** BOX DIED BEFORE STEP %s -- STOPPING ***' % label.split()[0], flush=True)
            return 3
        try:
            for chip in range(4):
                await c.command_text('EXECW 40 cmd /c %s poke %d 0x0A4 %d 0'
                                     % (EXE, chip, vals[chip]), timeout=90)
        except Exception as e:
            print('*** POKE FAILED ON STEP %s (%s) -- STOPPING ***' % (label.split()[0], e), flush=True)
            return 3
        finally:
            try:
                await c.close()
            except Exception:
                pass

        print('>>> STEP %-46s chips=%s' % (label, vals), flush=True)
        await asyncio.sleep(HOLD)

        u = await uptime()
        if u is None:
            print('*** BOX WENT UNREACHABLE DURING STEP %s -- that step wedges it ***'
                  % label.split()[0], flush=True)
            return 3
        if u < base:
            print('*** BOX REBOOTED DURING STEP %s (uptime %d < %d) ***'
                  % (label.split()[0], u, base), flush=True)
            return 3

    print('\nsweep finished cleanly - closing Q3, restoring desktop', flush=True)
    c = await C()
    if c is not None:
        await kill_q3(c)
        await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
        await c.close()
    return 0


sys.exit(asyncio.run(main()))
