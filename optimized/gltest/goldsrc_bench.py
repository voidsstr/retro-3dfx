#!/usr/bin/env python3
"""goldsrc_bench.py — automated GoldSrc (Half-Life / Counter-Strike) benchmark
on the Voodoo5 box (.143), driven entirely through the 3dfx ICD's own
instrumentation so it works even under CS's BCShield anti-cheat (which blocks
-condebug / qconsole.log).

How it works (no human input, no console injection, no anti-cheat conflict):
  * a generated `<gamedir>/listenserver.cfg` runs client-side the moment a
    listen-server map finishes loading.  It settles a few frames then drives a
    deterministic `noclip` fly-through (`+forward` + slow `+right` yaw) so the
    camera sweeps the whole map — a repeatable "timedemo"-equivalent workload.
  * the ICD (retro3dfx >= 0.3.9) logs per-window fps + WORST single-frame time
    (`maxFrame`) + texture-download counts to C:\\icd_perf.log (gate:
    C:\\icd_perf.on).  We read that file — it is the ICD's, so BCShield can't
    hide it.  maxFrame catches the ~1s stutter as a periodic spike; texDl>0
    would prove texture streaming (it does NOT on de_dust — textures are
    resident, so a stutter is elsewhere).
  * the ICD also dumps the real hardware front buffer to C:\\fbdump_NN.raw
    (gate: C:\\icd_fbdump.on) so rendering correctness (the CS green world,
    etc.) is verified from the actual scanout, not a GDI screenshot.

Usage:
  python3 goldsrc_bench.py [cs|cs2|hl] [WIDTH HEIGHT] [seconds] [nofb]
Examples:
  python3 goldsrc_bench.py cs 1024 768 40         # perf + fbdump frames
  python3 goldsrc_bench.py cs 1024 768 40 nofb    # perf ONLY (truthful stutter)
  python3 goldsrc_bench.py hl 640 480 30

`nofb` disables the front-buffer dump.  Use it for STUTTER measurement: the
fbdump grLfbReadRegion readback stalls the pipe and injects its own periodic
hitch, so maxFrame is only trustworthy with fbdump off.  Keep fbdump ON for
render-correctness runs (the CS green world etc.), OFF for timing.

Exit 0 always; prints the fps timeline, the peak hitch, and saves frames to
/tmp/goldsrc_bench_*.png.  Pair with the golden gate for a full render+perf gate.
"""
import asyncio, sys, os, struct, functools
sys.path.insert(0, '/home/voidsstr/development/retro-agent')
from PIL import Image
from client.retro_protocol import RetroConnection
print = functools.partial(print, flush=True)

HOST = '192.168.1.143'
OUT = '/tmp'
GAMES = {
    # tag: (game root dir, -game modname, mapname)
    'cs':  (r'C:\Program Files\Bcs16 Romania\Counter-Strike 1.6', 'cstrike', 'de_dust'),
    'cs2': (r'C:\Program Files\Counter-strike', 'cstrike', 'de_dust'),
    'hl':  (r'C:\Sierra\Half-Life', 'valve', 'crossfire'),
}

async def rc(c, cmd, t=40):
    s, d = await c.send_command(cmd, timeout=t); return d.decode('ascii', 'replace')

def raw565_to_png(raw, out):
    w = 640; h = len(raw) // (w * 2)
    if h <= 0: return None
    im = Image.new('RGB', (w, h)); px = im.load()
    for y in range(h):
        row = struct.unpack_from('<%dH' % w, raw, y * w * 2)
        for x in range(w):
            v = row[x]; px[x, y] = (((v >> 11) & 31) * 255 // 31,
                                    ((v >> 5) & 63) * 255 // 63, (v & 31) * 255 // 31)
    im.save(out); return im

def build_listenserver_cfg(settle_frames=120, fps_max=1000, hud=0, developer=1):
    """Deterministic noclip fly-through, all client console commands.

    Two useful profiles:
      * perf  (fps_max=1000, hud=0) — uncapped, HUD off: measures the raw
        render/glide pipeline. Steady-state is clean here (proves the stutter
        is not texture streaming / not a glide-swap stall).
      * real  (fps_max=100,  hud=1) — capped fps + HUD on, like actual play:
        reproduces a vsync/frame-cap BEAT stutter that only shows when the
        render rate is pinned near the display refresh. Pair with an 85Hz
        refresh override to A/B whether the beat is the ~1s walking stutter.
    """
    waits = '\r\n'.join(['wait'] * settle_frames)
    return ('// retro3dfx auto-bench camera (client-side, runs on listen-server map start)\r\n'
            'developer %d\r\nfps_max %d\r\nhud_draw %d\r\n' % (developer, fps_max, hud)
            + waits +
            '\r\nsv_cheats 1\r\nnoclip\r\ncl_yawspeed 40\r\n+forward\r\n+right\r\n').encode('latin-1')

async def main():
    flags = {'nofb', 'real'}
    argv = [a for a in sys.argv[1:] if a not in flags]
    nofb = 'nofb' in sys.argv[1:]
    real = 'real' in sys.argv[1:]   # capped fps + HUD on: reproduce the play-time beat stutter
    tag = argv[0] if len(argv) > 0 else 'cs'
    w = int(argv[1]) if len(argv) > 2 else 1024
    h = int(argv[2]) if len(argv) > 2 else 768
    secs = int(argv[3]) if len(argv) > 3 else 40
    root, mod, mp = GAMES[tag]

    c = RetroConnection(HOST, 9898); await c.connect('retro-agent-secret', timeout=15)
    await rc(c, r'EXEC taskkill /f /im hl.exe 2>nul', t=15); await asyncio.sleep(3)

    cfg = build_listenserver_cfg(fps_max=100, hud=1, developer=0) if real else build_listenserver_cfg()
    print('profile: %s (fps_max=%d hud=%d)' % ('real' if real else 'perf', 100 if real else 1000, 1 if real else 0))
    await c.send_command(r'UPLOAD %s\%s\listenserver.cfg' % (root, mod), binary_payload=cfg, timeout=30)
    gate = r'& echo x > C:\icd_perf.on' + ('' if nofb else r' & echo x > C:\icd_fbdump.on')
    await rc(c, r'EXEC cmd /c del /f /q C:\icd_perf.log C:\fbdump_*.raw 2>nul ' + gate, t=15)
    await rc(c, r'EXEC cmd /c cd /d "%s" ^&^& start "" hl.exe -game %s -gl -w %d -h %d '
                r'-full -noipx -nojoy +map %s' % (root, mod, w, h, mp), t=15)
    await asyncio.sleep(secs)

    alive = 'hl.exe' in (await rc(c, r'EXEC cmd /c tasklist | findstr /I hl.exe', t=20))
    log = await rc(c, r'EXEC cmd /c type C:\icd_perf.log 2>nul', t=20)
    rows = [l for l in log.strip().splitlines() if l.startswith('f=')]

    print('=== %s %dx%d  alive=%s  (%d perf windows) ===' % (tag, w, h, alive, len(rows)))
    fps_vals, hitches = [], []
    for l in rows:
        d = dict(kv.split('=') for kv in l.split() if '=' in kv)
        digits = lambda s: int(''.join(ch for ch in s if ch.isdigit()) or '0')
        fps = digits(d.get('fps10', '0')) / 10.0; mx = digits(d.get('maxFrame', '0'))
        fps_vals.append(fps); hitches.append(mx)
    for l in rows[-12:]:
        print(' ', l[:118])
    if fps_vals:
        body = fps_vals[3:] if len(fps_vals) > 4 else fps_vals   # drop warm-up windows
        print('--- fps: min %.0f  avg %.0f  max %.0f | worst single frame: %d ms (hitch) ---'
              % (min(body), sum(body) / len(body), max(body), max(hitches)))

    saved = []
    if not nofb:
        lst = await rc(c, r'EXEC cmd /c dir /b C:\fbdump_*.raw 2>nul', t=15)
        names = sorted(x.strip() for x in lst.split() if x.strip().lower().endswith('.raw'))
        for n in names[-2:]:
            rawd = await c.command_binary(r'DOWNLOAD C:\%s' % n, timeout=180)
            p = '%s/goldsrc_bench_%s_%s.png' % (OUT, tag, n.replace('.raw', ''))
            if raw565_to_png(rawd, p): saved.append(p)
        print('frames saved:', saved)
    else:
        print('frames saved: (perf-only run, fbdump disabled)')

    await rc(c, r'EXEC taskkill /f /im hl.exe 2>nul', t=15)
    # IMPORTANT: remove the auto-bench camera cfg so normal play is not hijacked
    await rc(c, r'EXEC cmd /c del /f /q "%s\%s\listenserver.cfg" 2>nul' % (root, mod), t=10)
    await rc(c, r'EXEC cmd /c del /f /q C:\icd_perf.on C:\icd_fbdump.on 2>nul', t=10)
    await c.close()

asyncio.run(main())
