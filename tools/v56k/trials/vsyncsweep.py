#!/usr/bin/env python3
"""Sweep the ONLY inter-chip horizontal alignment knob on the VSA-100:
cfgSliAaMisc[8:0] vga_vsync_offset = pixels[2:0] | chars[5:3] | hxtra[8:6].

The driver hard-codes pixels=7, chars=4 (= 39 px) for our config
(Miniport/H5/SLIAA.C:2489-2515, Case A), and its own comment admits the value is
a fudge around a vga_crtc_fast bug. Zeroing it visibly MOVED the bands, so this
is the right register - this finds the value that ALIGNS them.

Each step draws sligrid's static, bit-exact pattern and holds; the operator only
has to report a step NUMBER. Liveness-gated: the agent must answer between steps.
"""
import sys, asyncio, json

sys.path.insert(0, "/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection

IP = '192.168.1.191'
EXE = r'C:\RETRO_AGENT\v56k-deploy\sligrid.exe'
HOLD = int(sys.argv[1]) if len(sys.argv) > 1 else 25

# pixels fixed at 7 (both documented cases use it); sweep the char field.
# field value = 0x800 | (chars<<3) | pixels
STEPS = []
for chars in range(8):
    px = 7 + chars * 8
    note = ''
    if px == 31: note = "  (Case A's 'desired' value, before the vga_crtc_fast bump)"
    if px == 39: note = '  <-- CURRENT DRIVER DEFAULT'
    if px == 47: note = "  (Case B, 'run slave 8 clocks ahead')"
    STEPS.append((px, 0x800 | (chars << 3) | 7, note))


async def C(retries=5, delay=6):
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
    base = json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']
    await c.close()
    print('box up (uptime %ds). %d steps, %ds each.\n' % (base, len(STEPS), HOLD), flush=True)

    for n, (px, val, note) in enumerate(STEPS, 1):
        c = await C()
        if c is None:
            print('*** BOX UNREACHABLE before step %d -- STOPPING ***' % n, flush=True)
            return 3
        try:
            await c.command_text('EXEC cmd /c taskkill /f /im sligrid.exe 2>nul & echo ok', timeout=40)
            pokes = ' '.join('--poke %d:AC=0x%03X' % (ch, val) for ch in (1, 2, 3))
            await c.command_text(
                r'LAUNCH cmd /c %s --mode 640x480 --hold %d %s' % (EXE, HOLD + 8, pokes),
                timeout=45)
        except Exception as e:
            print('*** step %d failed (%s) -- STOPPING ***' % (n, e), flush=True)
            return 3
        finally:
            try: await c.close()
            except Exception: pass

        print('>>> STEP %d  vga_vsync_offset = %2d px  (cfgSliAaMisc=0x%03X on chips 1,2,3)%s'
              % (n, px, val, note), flush=True)
        await asyncio.sleep(HOLD)

        c = await C()
        if c is None:
            print('*** BOX WENT UNREACHABLE during step %d -- that value wedges it ***' % n, flush=True)
            return 3
        u = json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']
        await c.close()
        if u < base:
            print('*** BOX REBOOTED during step %d ***' % n, flush=True)
            return 3

    c = await C()
    if c is not None:
        await c.command_text('EXEC cmd /c taskkill /f /im sligrid.exe 2>nul & echo ok', timeout=40)
        await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
        await c.close()
    print('\nsweep finished cleanly.', flush=True)
    return 0


sys.exit(asyncio.run(main()))
