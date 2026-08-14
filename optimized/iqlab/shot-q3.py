#!/usr/bin/env python3
"""Deterministic same-frame Q3 screenshots for driver image-quality A/B.

A demo playback is NOT a controlled comparison - two runs land on different
frames. This loads a fixed map and pins the camera with Q3's cheat-protected
`setviewpos`, so two captures differ only by the driver under test.

Quoted console commands do NOT survive the EXECW/cmd/start quoting chain, so
the binds go in a cfg and we +exec it. GDI SCREENSHOT of a Glide fullscreen
surface is dark/interlaced - we use Q3's own in-engine screenshot instead.

  shot-q3.py --label amigamerlin --map q3dm1
"""
import argparse, asyncio, json, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP = '192.168.1.143'
Q3 = r'C:\Quake III Arena\Quake3'
HOME = r'C:\q3home'
SHOTS = HOME + r'\baseq3\screenshots'
OUT = '/mnt/c/development/retro-3dfx/optimized/iq-compare/'

# fixed camera positions: (name, "x y z pitch yaw roll")
VIEWS = {
    'q3dm1_a': '685 100 100 0 0 0',
    'q3dm1_b': '300 -100 200 10 90 0',
}


async def C(retries=20, delay=10):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: return None
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=30):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def kill(c):
    d = (await c.send_command('PROCLIST', timeout=20))[1].decode('ascii', 'replace')
    for p in json.loads(d):
        if p['name'].lower() == 'quake3.exe':
            await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % p['pid'])
    await asyncio.sleep(3)


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--label', required=True)
    ap.add_argument('--map', default='q3dm1')
    ap.add_argument('--mode', type=int, default=6)
    a = ap.parse_args()

    import os
    os.makedirs(OUT, exist_ok=True)
    c = await C()
    if not c:
        print('box down'); return 2
    await kill(c)
    await ex(c, r'cmd /c del /f /q "%s\*.tga" 2>nul & echo ok' % SHOTS, 20)

    cfg = ''.join('bind %s "setviewpos %s"\r\n' % (k, v)
                  for k, v in zip(('F9', 'F10'), VIEWS.values()))
    cfg += 'bind F12 "screenshot"\r\nseta cg_drawGun 0\r\nseta cg_draw2D 0\r\nseta r_gamma 1\r\n'
    await c.send_command(r'UPLOAD %s\baseq3\iqshot.cfg' % HOME, binary_payload=cfg.encode('latin-1'))

    await ex(c, ('cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=0^&^& start "" quake3.exe '
                 '+set r_glDriver 3dfxogl +set r_mode %d +set r_fullscreen 1 +set r_colorbits 16 '
                 '+set fs_homepath %s +set s_initsound 0 +set com_introPlayed 1 +set sv_pure 0 '
                 '+exec iqshot.cfg +devmap %s') % (Q3, a.mode, HOME, a.map), 25)
    print('  loading %s ...' % a.map)
    await c.close()
    await asyncio.sleep(55)

    c = await C(10)
    if not c:
        print('  box gone during load'); return 2
    names = list(VIEWS.keys())
    for key, name in zip(('f9', 'f10'), names):
        for _ in range(2):                       # pin the camera (twice: first can land early)
            await c.command_text('UIKEY %s' % key, timeout=15)
            await asyncio.sleep(3)
        await c.command_text('UIKEY f12', timeout=15)
        await asyncio.sleep(4)
        print('  shot %s' % name)

    t = await ex(c, r'cmd /c dir /b "%s\*.tga" 2>nul' % SHOTS, 25)
    tgas = sorted(l.strip() for l in t.splitlines() if l.strip().lower().endswith('.tga'))
    print('  captured: %s' % tgas)
    for i, f in enumerate(tgas):
        d = await c.command_binary(r'DOWNLOAD %s\%s' % (SHOTS, f), timeout=300)
        nm = '%s__%s.tga' % (a.label, names[i] if i < len(names) else 'shot%d' % i)
        open(OUT + nm, 'wb').write(d)
        print('    saved %s (%d B)' % (nm, len(d)))
    await kill(c); await c.close()
    return 0 if tgas else 1

sys.exit(asyncio.run(main()))
