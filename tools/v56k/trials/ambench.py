#!/usr/bin/env python3
"""Automated Q3 timedemo sweep across resolutions. Completion + fps is the
metric, so no visual judgement is needed. Liveness-gated per run."""
import sys, asyncio, json, re
sys.path.insert(0,"/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection
IP='192.168.1.191'; Q3=r'C:\Games\Quake3-TeamArena'
FPS=re.compile(r'(\d+) frames, ([\d.]+) seconds: ([\d.]+) fps')
# r_mode -> label
ALL={'2':'512x384','3':'640x480','4':'800x600','5':'960x720','6':'1024x768',
     '7':'1152x864','8':'1280x1024','9':'1600x1200'}
CFG=sys.argv[1] if len(sys.argv)>1 else '5'
TAG=sys.argv[2] if len(sys.argv)>2 else 'AM-4way'
# argv[3]: comma-separated r_mode list; default = the four standard points
SEL=(sys.argv[3].split(',') if len(sys.argv)>3 else ['3','4','6','8'])
MODES=[(m,ALL[m]) for m in SEL if m in ALL]
GK=r'HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\Glide'
DK=r'HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\D3D'

async def C(retries=5, delay=8):
    for i in range(retries):
        try:
            c=RetroConnection(IP,9898); await c.connect('retro-agent-secret',timeout=10); return c
        except Exception:
            if i==retries-1: return None
            await asyncio.sleep(delay)
    return None

async def run(mode,label,base):
    c=await C()
    if c is None: return None,'box gone'
    for p in [p['pid'] for p in json.loads((await c.send_command('PROCLIST',timeout=15))[1].decode('ascii','replace')) if p['name'].lower()=='quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok'%p, timeout=45)
    await c.command_text(r'EXECW 12 cmd /c del /f /q C:\q3home\baseq3\qconsole.log 2>nul & echo ok', timeout=35)
    cmd=(r'EXECW 15 cmd /c cd /d "%s" ^&^& set FX_GLIDE_REFRESH=60^&^& set FX_GLIDE_SWAPINTERVAL=0^&^& start "" quake3.exe '
         r'+set r_glDriver 3dfxogl +set r_mode %s +set r_fullscreen 1 +set r_colorbits 16 '
         r'+set fs_homepath C:\q3home +set logfile 2 +set s_initsound 0 +set com_introPlayed 1 '
         r'+set r_displayRefresh 60 +set r_swapInterval 0 +set nextdemo quit +set timedemo 1 +demo four') % (Q3, mode)
    await c.command_text(cmd, timeout=40); await c.close()
    for _ in range(20):
        await asyncio.sleep(10)
        c=await C(retries=2, delay=6)
        if c is None: return None,'UNREACHABLE'
        try:
            u=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
            if u<base: await c.close(); return None,'REBOOTED'
            t=await c.command_text(r'EXECW 20 cmd /c type C:\q3home\baseq3\qconsole.log 2>nul', timeout=45)
        except Exception:
            await c.close(); continue
        m=FPS.search(t); await c.close()
        if m: return float(m.group(3)),'ok'
    return None,'no fps in 200s'

async def main():
    c=await C()
    if c is None: print('box unreachable'); return 2
    base=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
    for k in (GK,DK):
        await c.command_text('EXECW 30 cmd /c reg add "%s" /v SSTH3_SLI_AA_CONFIGURATION /t REG_SZ /d %s /f'%(k,CFG), timeout=60)
    await c.close()
    print('=== %s (SLI cfg=%s) ==='%(TAG,CFG), flush=True)
    for mode,label in MODES:
        fps,st = await run(mode,label,base)
        print('  %-10s r_mode %-2s : %s'%(label, mode, ('%.1f fps'%fps) if fps else 'FAIL (%s)'%st), flush=True)
        if st in ('UNREACHABLE','REBOOTED'): break
        await asyncio.sleep(20)
    c=await C()
    if c: 
        await c.command_text('DISPLAYCFG set 1024 768 16 75', timeout=25); await c.close()
    print('sweep done', flush=True)
asyncio.run(main())
