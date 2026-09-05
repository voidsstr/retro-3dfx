#!/usr/bin/env python3
"""Does the retro-agent (clean-room / open) stack work on the Voodoo 5 6000?

Reproduces on `.191` (4x VSA-100) the isolation matrix that
`OPEN-STACK-ON-VSA100.md` ran on `.143` (2x VSA-100), where the open Glide hung
at init.  Two user-mode layers varied independently over a FIXED kernel driver:

    cell  OpenGL ICD                    Glide3 in game dir   isolates      risk
    A     AmigaMerlin 3dfxgl.dll        (none -> system32)   baseline      none
    C     open MesaFX (retail-linked)   (none -> system32)   the ICD       low
    D     AmigaMerlin 3dfxgl.dll        open glide3x_h5      the Glide     FREEZE
    B     open MesaFX (undecorated)     open glide3x_h5      both open     FREEZE

*** THE OPEN GLIDE HARD-FREEZES THIS BOARD. ***  On 2026-09-04 cell B took the
whole machine down - 100% ping loss, NIC dead, physical power cycle.  On the
2-chip 5500 the same pairing merely hung the game.  Consequences, all baked in
below:

  - Cells run SAFE-FIRST (A, C, then D, then B).  Never queue a cell you want a
    result from behind one that can freeze the box.
  - Every cell's result is written to the results file THE MOMENT it completes,
    so a freeze costs one cell, not the run.
  - `--cells` lets you resume with only what is left.
  - Never run the freezing cells unattended when nobody can power-cycle.

Everything is staged GAME-LOCAL in the Quake 2 directory; `system32` is NEVER
written.  A game directory shadows system32 for DLL lookup, so dropping
`glide3x.dll` in the game dir swaps the Glide for that game alone, and deleting
it reverts.

Identity comes from Q2's own GL_RENDERER / GL_VERSION, never from which file we
think we selected: Q2 silently falls back to Microsoft's software GL when an ICD
fails to bind, and that fallback still prints a plausible fps number.

Usage:
    openstack_matrix.py [--cells A,C,D,B] [--results FILE]
"""
import sys, asyncio, json, re, os, hashlib, argparse

sys.path.insert(0, "/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection

IP = '192.168.1.191'
Q2 = r'C:\Games\Quake2Complete'
LOG = Q2 + r'\baseq2\qconsole.log'
OUT = '/home/voidsstr/development/retro-agent/voodoo-cleanroom/out'
RESP_ERROR = 0xFF

FPS = re.compile(r'(\d+) frames, ([\d.]+) seconds: ([\d.]+) fps')
REND = re.compile(r'GL_RENDERER:\s*(.+)')
VERS = re.compile(r'GL_VERSION:\s*(.+)')
VEND = re.compile(r'GL_VENDOR:\s*(.+)')

STAGE = {
    'openicd.dll':   'opengl32.dll',         # MesaFX, imports grFoo@N  (undecorated)
    'openicdr.dll':  'opengl32_retail.dll',  # MesaFX, imports _grFoo@N (retail/AmigaMerlin)
    'openglide.dll': 'glide3x_h5.dll',       # open Glide3, VSA-100 build
}

# tag -> (gl_driver, stage open glide?, risk, description)
CELLS = {
    'A': ('3dfxgl',   False, 'none',   'baseline: AmigaMerlin ICD + AmigaMerlin Glide'),
    'C': ('openicdr', False, 'low',    'open MesaFX (retail-linked) + AmigaMerlin Glide -> ICD alone'),
    'D': ('3dfxgl',   True,  'FREEZE', 'AmigaMerlin ICD + open Glide -> Glide alone'),
    'B': ('openicd',  True,  'FREEZE', 'open MesaFX + open Glide -> the .143 pairing'),
}


async def C(retries=6, delay=7):
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


async def uptime(c):
    return json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']


async def stage(c):
    """Upload the candidates game-local, verifying by DOWNLOAD+md5 out of band.

    NOT `certutil` on the target: XP's certutil formats (and may not support)
    -hashfile in a way that silently fails the compare.  deploy171.py's
    upload-then-download-and-md5 is the method that actually works here.
    """
    print('--- staging candidates in %s ---' % Q2, flush=True)
    all_ok = True
    for dest, src in STAGE.items():
        p = os.path.join(OUT, src)
        if not os.path.exists(p):
            print('  SKIP %-14s (%s not built)' % (dest, src), flush=True)
            all_ok = False
            continue
        data = open(p, 'rb').read()
        want = hashlib.md5(data).hexdigest()
        st, resp = await c.send_command(r'UPLOAD %s\%s' % (Q2, dest),
                                        binary_payload=data, timeout=300)
        if st == RESP_ERROR:
            print('  UPLOAD FAILED %-14s %s' % (dest, resp[:100]), flush=True)
            all_ok = False
            continue
        back = await c.command_binary(r'DOWNLOAD %s\%s' % (Q2, dest), timeout=300)
        got = hashlib.md5(back).hexdigest()
        ok = (got == want)
        all_ok &= ok
        print('  %-14s <- %-22s %7d B  md5 %s  %s'
              % (dest, src, len(data), want[:12], 'OK' if ok else 'MISMATCH ' + got[:12]),
              flush=True)
    return all_ok


async def run_cell(tag, base):
    gldriver, local_glide, risk, note = CELLS[tag]
    c = await C()
    if c is None:
        return dict(cell=tag, status='BOX UNREACHABLE before cell', note=note)

    await c.command_text('EXECW 20 cmd /c taskkill /F /IM quake2.exe 2>nul & echo ok', timeout=50)
    if local_glide:
        await c.command_text(r'EXECW 20 cmd /c copy /Y "%s\openglide.dll" "%s\glide3x.dll" >nul & echo ok'
                             % (Q2, Q2), timeout=50)
    else:
        await c.command_text(r'EXECW 20 cmd /c del /f /q "%s\glide3x.dll" 2>nul & echo ok' % Q2, timeout=50)
    await c.command_text(r'EXECW 15 cmd /c del /f /q "%s" 2>nul & echo ok' % LOG, timeout=45)

    # +set logfile 2 is REQUIRED: without it Q2 writes no qconsole.log at all,
    # so there is no GL_RENDERER and no fps line and the cell reports nothing.
    # (Cost a whole run on 2026-09-04.)  Flags mirror deploy171.py's q2run.bat.
    cmd = (r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=0^&^& '
           r'set FX_GLIDE_NO_SPLASH=1^&^& start "" quake2.exe +set vid_ref gl '
           r'+set gl_driver %s +set gl_mode 3 +set gl_bitdepth 16 +set vid_fullscreen 1 '
           r'+set logfile 2 +set s_initsound 0 +set cd_nocd 1 +set gl_finish 0 '
           r'+set gl_swapinterval 0 +set cl_maxfps 1000 +set timedemo 1 +map demo1.dm2') \
          % (Q2, gldriver)
    await c.command_text(cmd, timeout=45)
    await c.close()

    fps = rend = vers = vend = None
    for _ in range(18):
        await asyncio.sleep(10)
        c2 = await C(retries=2, delay=5)
        if c2 is None:
            return dict(cell=tag, status='BOX UNREACHABLE (hard freeze?)', note=note,
                        renderer=rend, version=vers)
        try:
            if await uptime(c2) < base:
                await c2.close()
                return dict(cell=tag, status='BOX REBOOTED', note=note)
            log = await c2.command_text(r'EXECW 25 cmd /c type "%s" 2>nul' % LOG, timeout=60)
        except Exception:
            try: await c2.close()
            except Exception: pass
            continue
        await c2.close()
        # parse the LAST session block only - qconsole.log accumulates
        tail = log.split('GL_RENDERER:')[-1] if 'GL_RENDERER:' in log else log
        r, v, n = REND.search(log), VERS.search(tail), VEND.search(tail)
        if r: rend = log.split('GL_RENDERER:')[-1].splitlines()[0].strip()
        if v: vers = v.group(1).strip()
        if n: vend = n.group(1).strip()
        m = FPS.search(tail)
        if m:
            fps = float(m.group(3)); break

    c3 = await C()
    if c3 is None:
        return dict(cell=tag, status='BOX UNREACHABLE after run (hard freeze?)', note=note,
                    renderer=rend, version=vers)
    await c3.command_text('EXECW 20 cmd /c taskkill /F /IM quake2.exe 2>nul & echo ok', timeout=50)
    await c3.close()

    if fps:
        st = 'ok'
    elif rend or vers:
        st = 'NO FPS - reached GL then stopped (init hang)'
    else:
        st = 'NO GL - never bound an ICD'
    return dict(cell=tag, status=st, fps=fps, renderer=rend, version=vers, vendor=vend, note=note)


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--cells', default='A,C,D,B')
    ap.add_argument('--results', default='/tmp/openstack_matrix.json')
    a = ap.parse_args()
    order = [t.strip().upper() for t in a.cells.split(',') if t.strip()]

    c = await C()
    if c is None:
        print('ABORT: box unreachable'); return 2
    base = await uptime(c)
    print('box up (uptime %ds)\n' % base, flush=True)
    staged = await stage(c)
    await c.close()
    if not staged:
        print('\nABORT: staging did not verify - fix that before trusting any cell.', flush=True)
        return 2

    results = []
    if os.path.exists(a.results):
        try: results = json.load(open(a.results))
        except Exception: results = []

    for tag in order:
        if tag not in CELLS:
            print('unknown cell %r, skipping' % tag, flush=True); continue
        gld, lg, risk, note = CELLS[tag]
        print('\n>>> CELL %s  [risk: %s]  gl_driver=%-9s open-glide=%-5s\n    %s'
              % (tag, risk, gld, lg, note), flush=True)
        try:
            r = await run_cell(tag, base)
        except Exception as e:
            r = dict(cell=tag, status='harness error: %s' % e, note=note)

        results = [x for x in results if x.get('cell') != tag] + [r]
        json.dump(results, open(a.results, 'w'), indent=1)   # persist IMMEDIATELY

        print('    -> %s' % r.get('status'), flush=True)
        if r.get('fps'):      print('       %.1f fps' % r['fps'], flush=True)
        if r.get('renderer'): print('       GL_RENDERER: %s' % r['renderer'], flush=True)
        if r.get('version'):  print('       GL_VERSION : %s' % r['version'], flush=True)
        if 'BOX' in str(r.get('status')):
            print('\n*** BOX IS DOWN. Remaining cells NOT run: %s'
                  % ','.join(order[order.index(tag) + 1:]), flush=True)
            print('*** Power-cycle, then resume with --cells %s'
                  % ','.join(order[order.index(tag) + 1:]), flush=True)
            break
        await asyncio.sleep(12)

    c = await C()
    if c:   # leave the game dir as found
        await c.command_text(r'EXECW 20 cmd /c del /f /q "%s\glide3x.dll" 2>nul & echo ok' % Q2, timeout=50)
        await c.close()

    print('\n================ MATRIX ================', flush=True)
    for tag in ('A', 'C', 'D', 'B'):
        r = next((x for x in results if x.get('cell') == tag), None)
        if r is None:
            print('%s  (not run)' % tag, flush=True); continue
        print('%s  %-42s fps=%-7s %s' % (tag, r['status'], r.get('fps') or '-',
                                         r.get('renderer') or ''), flush=True)
    print('matrix done  (results: %s)' % a.results, flush=True)
    return 0


sys.exit(asyncio.run(main()))
