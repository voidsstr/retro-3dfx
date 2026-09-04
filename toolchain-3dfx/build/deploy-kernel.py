#!/usr/bin/env python3
"""Deploy the display driver + miniport to the V5 box and reboot.

WFP-safe names only (3dfxv5d.dll / 3dfxv5m.sys) -- raw-copying the in-box names
into system32 gets reverted by Windows File Protection. A loaded file cannot be
overwritten, so each is moved aside first and that copy IS the rollback. Every
byte is md5-verified on the box BEFORE the reboot; nothing reboots on a mismatch.

  deploy-kernel.py [--ip A.B.C.D] [--src DIR] [--no-reboot] [display|miniport|both]

Run tests/predeploy.sh first -- non-zero exit means do NOT deploy.
Rollback: copy <name>.v56kprev back over <name> and reboot.
"""
import argparse, asyncio, hashlib, json, os, sys
sys.path.insert(0, os.environ.get('RETRO_AGENT_DIR',
                os.path.expanduser('~/development/retro-agent')))
from client.retro_protocol import RetroConnection

STAGE = r'C:\RETRO_AGENT\v56k-kernel'
TARGETS = {'display':  ('3dfxv5d.dll', r'C:\WINDOWS\system32'),
           'miniport': ('3dfxv5m.sys', r'C:\WINDOWS\system32\drivers')}

async def connect(ip, retries=6):
    for i in range(retries):
        try:
            c = RetroConnection(ip, 9898); await c.connect('retro-agent-secret', timeout=10); return c
        except Exception:
            if i == retries - 1: raise
            await asyncio.sleep(8)

async def ex(c, cmd, secs=30):
    return await c.command_text('EXECW %d %s' % (secs, cmd), timeout=secs + 20)

async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('what', nargs='?', default='both', choices=['display', 'miniport', 'both'])
    ap.add_argument('--ip', default=os.environ.get('DEPLOY_IP', '192.168.1.133'),
                    help='box to deploy to (default %(default)s)')
    ap.add_argument('--src', default=os.path.join(os.path.dirname(__file__),
                                                  '../../optimized/v56k-sli-build-20260811'))
    ap.add_argument('--no-reboot', action='store_true')
    a = ap.parse_args()
    src = os.path.abspath(a.src)
    files = [TARGETS[k] for k in (['display', 'miniport'] if a.what == 'both' else [a.what])]

    c = await connect(a.ip)
    up = json.loads((await c.send_command('SYSINFO', timeout=12))[1].decode())['uptime_seconds']
    print('box up, uptime=%ds' % up)
    await ex(c, r'cmd /c mkdir "%s" 2>nul & echo ok' % STAGE)

    # stage + verify by md5 readback
    for name, _ in files:
        p = os.path.join(src, name)
        data = open(p, 'rb').read(); local = hashlib.md5(data).hexdigest()
        await c.send_command(r'UPLOAD %s\%s' % (STAGE, name), binary_payload=data)
        back = await c.command_binary(r'DOWNLOAD %s\%s' % (STAGE, name))
        if hashlib.md5(back).hexdigest() != local or len(back) != len(data):
            print('  !! staged copy mismatch for %s — ABORTING' % name); await c.close(); return 1
        print('  staged %-13s %d bytes md5=%s VERIFIED' % (name, len(data), local[:12]))

    # move the live file aside (that IS the rollback), copy the new one in
    for name, dest in files:
        await ex(c, r'cmd /c if exist "%s\%s.v56kprev" del /f /q "%s\%s.v56kprev"' % (dest, name, dest, name), 20)
        await ex(c, r'cmd /c move /Y "%s\%s" "%s\%s.v56kprev" >nul & echo ok' % (dest, name, dest, name), 25)
        await ex(c, r'cmd /c copy /Y "%s\%s" "%s\%s" >nul & echo ok' % (STAGE, name, dest, name), 25)
        back = await c.command_binary(r'DOWNLOAD %s\%s' % (dest, name))
        want = hashlib.md5(open(os.path.join(src, name), 'rb').read()).hexdigest()
        ok = hashlib.md5(back).hexdigest() == want
        print('  installed %-13s %s' % (name, 'ON DISK MATCHES BUILD' if ok else '!! MISMATCH'))
        if not ok: print('  !! ABORTING before reboot'); await c.close(); return 1

    if a.no_reboot:
        print('\nstaged and installed; NOT rebooting (--no-reboot)')
    else:
        print('\nrebooting...')
        try: await c.send_command('REBOOT')
        except Exception: pass
    try: await c.close()
    except Exception: pass
    return 0

sys.exit(asyncio.run(main()))
