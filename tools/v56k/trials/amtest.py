#!/usr/bin/env python3
"""Automated pass/fail: does the Q3 timedemo COMPLETE (fps line) under a given
SLI config? No visual judgement needed - completion is the metric.
   amtest.py <sliconfig 0|5> <label>"""
import sys, asyncio, json, re
sys.path.insert(0,"/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection
IP='192.168.1.191'; Q3=r'C:\Games\Quake3-TeamArena'
GK=r'HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\Glide'
DK=r'HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\D3D'
CFG=sys.argv[1]; LABEL=sys.argv[2] if len(sys.argv)>2 else CFG
FPS=re.compile(r'(\d+) frames, ([\d.]+) seconds: ([\d.]+) fps')

async def C(retries=5, delay=8, timeout=10):
    for i in range(retries):
        try:
            c=RetroConnection(IP,9898); await c.connect('retro-agent-secret',timeout=timeout); return c
        except Exception:
            if i==retries-1: return None
            await asyncio.sleep(delay)
    return None

async def m():
    c=await C()
    if c is None: print('%s: ABORT box unreachable'%LABEL); return 2
    base=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
    for p in [p['pid'] for p in json.loads((await c.send_command('PROCLIST',timeout=15))[1].decode('ascii','replace')) if p['name'].lower()=='quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok'%p, timeout=45)
    for k in (GK, DK):
        await c.command_text('EXECW 30 cmd /c reg add "%s" /v SSTH3_SLI_AA_CONFIGURATION /t REG_SZ /d %s /f'%(k,CFG), timeout=60)
    await c.command_text(r'EXECW 12 cmd /c del /f /q C:\q3home\baseq3\qconsole.log 2>nul & echo ok', timeout=35)
    cmd=(r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_REFRESH=60^&^& set FX_GLIDE_SWAPINTERVAL=0^&^& start "" quake3.exe '
         r'+set r_glDriver 3dfxogl +set r_mode 3 +set r_fullscreen 1 +set r_colorbits 16 '
         r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1 '
         r'+set r_displayRefresh 60 +set r_swapInterval 0 +set nextdemo quit +set timedemo 1 +demo four') % Q3
    await c.command_text(cmd, timeout=40); await c.close()
    print('%s: launched (SLI cfg=%s), waiting up to 180s for the fps line'%(LABEL,CFG), flush=True)
    for _ in range(18):
        await asyncio.sleep(10)
        c=await C(retries=2, delay=6)
        if c is None:
            print('%s: *** BOX UNREACHABLE - hard freeze ***'%LABEL, flush=True); return 3
        try:
            u=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
            if u<base: print('%s: *** BOX REBOOTED (bugcheck) ***'%LABEL, flush=True); await c.close(); return 3
            t=await c.command_text(r'EXECW 20 cmd /c type C:\q3home\baseq3\qconsole.log 2>nul', timeout=45)
        except Exception:
            await c.close(); continue
        mm=FPS.search(t)
        await c.close()
        if mm:
            print('%s: PASS - timedemo COMPLETED: %s fps (%s frames / %ss)'%(LABEL,mm.group(3),mm.group(1),mm.group(2)), flush=True)
            return 0
    print('%s: FAIL - no fps line in 180s (hung at render start)'%LABEL, flush=True)
    return 1
sys.exit(asyncio.run(m()))
