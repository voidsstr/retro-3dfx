#!/usr/bin/env python3
"""Deterministic single-frame A/B screenshot from Q3, for ICD rendering changes.

A demo playback is NOT a controlled comparison -- two runs land on different
frames (different room, different HUD), and comparing their mean luminance is
meaningless. This loads a fixed map and uses Q3's cheat-protected `setviewpos`
to pin the camera to an exact position/angle, so two runs differ only by the
driver under test.

  icd-ab-shot.py --label stock
  icd-ab-shot.py --label arb
then compare /tmp/qa256/ab_<label>.png

Used to validate queue item 12 (icd-multitexture): single-pass modulate rounds
differently than two-pass blend, so the diff will not be zero -- what matters is
that it is small and uniform, not a global darkening (the earlier
3dfxogl-0.1.4-multitexture-itercolor-DARK build failed exactly that way).
"""
import argparse, asyncio, json, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

IP = '192.168.1.133'
Q3 = r'C:\Games\Quake III Arena\Quake3'
SHOTS = r'C:\q3home\baseq3\screenshots'
VIEWPOS = '685 100 100 0 0 0'      # fixed camera in q3dm1


async def C(retries=25, delay=12):
    for i in range(retries):
        try:
            c = RetroConnection(IP, 9898); await c.connect('retro-agent-secret', timeout=8); return c
        except Exception:
            if i == retries - 1: return None
            await asyncio.sleep(delay)


async def ex(c, cmd, secs=25):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 25)


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--label', required=True)
    ap.add_argument('--map', default='q3dm1')
    a = ap.parse_args()

    c = await C()
    if not c:
        print('box down'); return 2
    await ex(c, r'cmd /c del /f /q %s\\*.tga 2>nul & echo ok' % SHOTS)
    # Quoted console commands do NOT survive the EXECW/cmd/start quoting chain --
    # that is why an inline +bind "setviewpos ..." silently produced no shot.
    # Ship them in a cfg and +exec it instead.
    cfg = ('bind F11 "setviewpos %s"\r\nbind F12 "screenshot"\r\n'
           'seta cg_drawGun 0\r\nseta cg_draw2D 0\r\n' % VIEWPOS)
    await c.send_command(r'UPLOAD C:\q3home\baseq3\abshot.cfg',
                         binary_payload=cfg.encode('latin-1'))
    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=1^&^& start "" quake3.exe '
           r'+set r_glDriver 3dfxogl +set r_mode 6 +set r_fullscreen 1 +set r_colorbits 16 '
           r'+set fs_homepath C:\q3home +set s_initsound 0 +set com_introPlayed 1 '
           r'+exec abshot.cfg +devmap %s') % (Q3, a.map)
    await ex(c, cmd, 20)
    print('%s: loading %s...' % (a.label, a.map))
    await c.close()
    await asyncio.sleep(50)

    c = await C(10)
    if not c:
        print('box gone during load -> REBOOT'); return 2
    for _ in range(2):
        await c.command_text('UIKEY F11', timeout=15)   # pin camera
        await asyncio.sleep(4)
    await c.command_text('UIKEY F12', timeout=15)       # screenshot
    await asyncio.sleep(6)
    t = await ex(c, r'cmd /c dir /b %s\*.tga 2>nul' % SHOTS)
    shots = [l.strip() for l in t.splitlines() if l.strip().lower().endswith('.tga')]
    print('  shots:', shots[:3])
    if shots:
        d = await c.command_binary(r'DOWNLOAD %s\%s' % (SHOTS, shots[-1]), timeout=200)
        open('/tmp/qa256/ab_%s.tga' % a.label, 'wb').write(d)
        print('  saved /tmp/qa256/ab_%s.tga (%d bytes)' % (a.label, len(d)))
    pl = json.loads((await c.send_command('PROCLIST', timeout=20))[1].decode('ascii', 'replace'))
    for p in pl:
        if p['name'].lower() == 'quake3.exe':
            await ex(c, 'cmd /c taskkill /f /pid %d 2>nul & echo ok' % p['pid'])
    await c.close()
    return 0 if shots else 1

sys.exit(asyncio.run(main()))
