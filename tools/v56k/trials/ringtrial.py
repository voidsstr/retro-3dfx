#!/usr/bin/env python3
"""Ring-capture a Q3 session AND LEAVE THE GAME RUNNING for the operator's verdict.
   ringtrial.py <mode> <secs> <tag>
Read-only: the ring never writes hardware, so it cannot contaminate later runs."""
import sys, asyncio, json
sys.path.insert(0,"/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection
IP='192.168.1.191'; Q3=r'C:\Games\Quake3-TeamArena'
EXE=r'C:\RETRO_AGENT\v56k-deploy\fxscan2.exe'
MODE=sys.argv[1] if len(sys.argv)>1 else '3'
SECS=int(sys.argv[2]) if len(sys.argv)>2 else 60
TAG=sys.argv[3] if len(sys.argv)>3 else 'run'

async def C(retries=6, delay=6):
    for i in range(retries):
        try:
            c=RetroConnection(IP,9898); await c.connect('retro-agent-secret',timeout=10); return c
        except Exception:
            if i==retries-1: return None
            await asyncio.sleep(delay)
    return None

async def m():
    c=await C()
    if c is None: print('ABORT: unreachable'); return 2
    base=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
    for p in [p['pid'] for p in json.loads((await c.send_command('PROCLIST',timeout=15))[1].decode('ascii','replace')) if p['name'].lower()=='quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok'%p, timeout=45)
    await c.command_text(r'EXECW 15 cmd /c del /f /q C:\fxring.txt 2>nul & echo ok', timeout=35)
    await c.command_text(r'LAUNCH cmd /c %s ring %d 60 C:\fxring.txt' % (EXE, SECS), timeout=40)
    print('ring started (%ds)'%SECS, flush=True)
    await asyncio.sleep(5)
    cmd=(r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_SWAPINTERVAL=1^&^& start "" quake3.exe '
         r'+set r_glDriver 3dfxogl +set r_mode %s +set r_fullscreen 1 +set r_colorbits 16 '
         r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1') % (Q3, MODE)
    await c.command_text(cmd, timeout=40); await c.close()
    print('Q3 launched; recording %ds...'%SECS, flush=True)
    await asyncio.sleep(SECS+8)
    c=await C()
    if c is None:
        print('*** BOX UNREACHABLE - ring file survives on C:\\fxring.txt ***'); return 3
    u=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
    if u<base: print('*** BOX REBOOTED during the run ***')
    d=await c.command_binary(r'DOWNLOAD C:\fxring.txt', timeout=180)
    open('ring_%s.txt'%TAG,'wb').write(d)
    print('ring saved: ring_%s.txt (%d bytes)'%(TAG,len(d)), flush=True)
    await c.close()
    print('\n>>> Q3 IS STILL RUNNING and stays up for your verdict. <<<', flush=True)
    return 0
sys.exit(asyncio.run(m()))
