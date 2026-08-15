#!/usr/bin/env python3
"""Publish iqlab.exe to the fleet share, versioned, and install it on a box.

Follows the share convention already in use for retro_agent.exe (retro-agent
CLAUDE.md, "Publishing builds to the share"): publish THREE things so a box can
both grab "the latest" and pin an exact build --

    <share>/Retro Automation/iqlab/iqlab.exe            latest pointer
    <share>/Retro Automation/iqlab/iqlab.exe.ver        version sidecar
    <share>/Retro Automation/iqlab/iqlab-<ver>.exe      versioned archive

The sidecar is written LAST. Version decides the pull, never file size -- the
dosstage bug (size-only comparison silently refusing to ship a same-size fix) is
exactly the failure this ordering avoids.

    publish.py                 # build check + publish to the share
    publish.py --install HOST  # also install onto that box's desktop
    publish.py --install HOST --no-share   # box only (share down)
"""
import argparse, asyncio, hashlib, os, re, sys, functools
print = functools.partial(print, flush=True)
sys.path.insert(0, '/mnt/c/development/retro-agent')
from client.retro_protocol import RetroConnection

HERE  = os.path.dirname(os.path.abspath(__file__))
EXE   = os.path.join(HERE, 'iqlab.exe')
SRC   = os.path.join(HERE, 'iqlab.c')
PORT  = 9898
SECRET = 'retro-agent-secret'

SHARE_UNC = r'\\192.168.1.122\files'
SHARE_DIR = r'Z:\Utility\Retro Automation\iqlab'
STAGE     = r'C:\RETRO_AGENT\iqlab'


def version():
    m = re.search(r'#define\s+IQLAB_VERSION\s+"([^"]+)"', open(SRC, encoding='utf-8').read())
    return m.group(1) if m else '0.0.0'


async def txt(c, cmd, secs=60):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 30)


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--install', metavar='HOST', help='also install on this box')
    ap.add_argument('--no-share', action='store_true', help='skip the share publish')
    ap.add_argument('--relay', default='192.168.1.143',
                    help='box used to reach the share (it has Z: mapped)')
    a = ap.parse_args()

    if not os.path.exists(EXE):
        sys.exit('missing iqlab.exe -- run ./build.sh first')
    data = open(EXE, 'rb').read()
    ver  = version()
    md5  = hashlib.md5(data).hexdigest()
    print('iqlab %s  %d bytes  md5=%s' % (ver, len(data), md5[:16]))

    if not a.no_share:
        host = a.relay
        print('\n== publish to the share via %s ==' % host)
        c = RetroConnection(host, PORT); await c.connect(SECRET, timeout=15)
        await txt(c, r'cmd /c net use Z: %s /persistent:yes 2>nul & echo ok' % SHARE_UNC, 60)
        zc = await txt(c, r'cmd /c if exist Z:\ (echo ZOK) else (echo ZDOWN)', 40)
        if 'ZOK' not in zc:
            print('  share is DOWN -- skipped (rerun when %s is up)' % SHARE_UNC)
        else:
            await txt(c, 'cmd /c md "%s" 2>nul & echo ok' % SHARE_DIR, 40)
            # versioned archive first, then the latest pointer, sidecar LAST
            for name in ('iqlab-%s.exe' % ver, 'iqlab.exe'):
                await c.send_command('UPLOAD %s\\%s' % (SHARE_DIR, name), binary_payload=data)
                back = await c.command_binary('DOWNLOAD %s\\%s' % (SHARE_DIR, name), timeout=300)
                ok = hashlib.md5(back).hexdigest() == md5
                print('  %-22s %s' % (name, 'verified' if ok else '*** MISMATCH ***'))
            side = ('%s\n' % ver).encode('ascii')
            await c.send_command('UPLOAD %s\\iqlab.exe.ver' % SHARE_DIR, binary_payload=side)
            print('  iqlab.exe.ver          %s (written last -- version decides the pull)' % ver)
        await c.close()

    if a.install:
        print('\n== install on %s ==' % a.install)
        c = RetroConnection(a.install, PORT); await c.connect(SECRET, timeout=15)
        # desktop of the logged-on profile
        prof = (await txt(c, 'cmd /c echo %USERPROFILE%', 30)).strip().splitlines()[-1].strip()
        desk = prof + r'\Desktop'
        # a running instance LOCKS the staged exe, so the upload silently keeps the
        # old binary and you then launch the version you thought you replaced
        await txt(c, 'cmd /c taskkill /f /im iqlab.exe 2>nul & echo ok', 40)
        await asyncio.sleep(2)
        await txt(c, 'cmd /c md "%s" 2>nul & echo ok' % STAGE, 40)
        for dest in ('%s\\iqlab.exe' % STAGE, '%s\\iqlab.exe' % desk):
            await c.send_command('UPLOAD %s' % dest, binary_payload=data)
            back = await c.command_binary('DOWNLOAD %s' % dest, timeout=300)
            print('  %-52s %s' % (dest, 'verified'
                  if hashlib.md5(back).hexdigest() == md5 else '*** MISMATCH ***'))
        await txt(c, 'cmd /c echo %s> "%s\\iqlab.ver" & echo ok' % (ver, STAGE), 30)
        print('  installed version %s' % ver)
        await c.close()
    return 0

sys.exit(asyncio.run(main()))
