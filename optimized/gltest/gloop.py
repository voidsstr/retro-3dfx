#!/usr/bin/env python3
"""Green-world fast test loop. One command: deploy current ICD build (or a
named dll), run a chosen gfix case OR launch CS with fbdump, pull the result,
and print numeric pass/fail. Keeps the box clean (kills procs, restores mode).

Usage:
  python3 gloop.py case <LETTER> <x> <y> <expectR> <expectG> <expectB>
        - run gfix, sample gfix_<LETTER>.raw at (x,y), compare to expected.
  python3 gloop.py cs [seconds]
        - deploy ICD, launch CS de_dust with fbdump on, pull last fbdump PNG,
          sample the near-wall region, report green-ness.
  python3 gloop.py deploy
        - just deploy the current ICD build to all game-local copies.
"""
import asyncio, sys, struct
sys.path.insert(0,'/home/voidsstr/development/retro-agent')
from PIL import Image
from client.retro_protocol import RetroConnection

IP='192.168.1.143'
ICD_SRC='/home/voidsstr/development/retro-3dfx/toolchain-3dfx/prefix/drive_c/3dfx/SWLIBS/OPENGL/GLIDE3X/release/opengl.dll'
GFIX_SRC='/home/voidsstr/development/retro-3dfx/toolchain-3dfx/prefix/drive_c/3dfx/gltest/gfix.exe'
ST=r'C:\RETRO_AGENT\3dfx-driver'
CS=r'C:\Program Files\Bcs16 Romania\Counter-Strike 1.6'
TARGETS=[
 r'C:\Games\Quake2\opengl32.dll', r'C:\Games\Quake2\3dfxogl.dll',
 r'C:\GOG Games\Unreal Tournament GOTY\System\opengl32.dll',
 r'C:\Program Files\Bcs16 Romania\Counter-Strike 1.6\opengl32.dll',
 r'C:\Program Files\Counter-strike\opengl32.dll',
 r'C:\Quake III Arena\Quake3\opengl32.dll', r'C:\Quake III Arena\Quake3\3dfxogl.dll',
 r'C:\GOG Games\Return to Castle Wolfenstein\opengl32.dll',
 r'C:\Program Files\EA GAMES\MOHAA\opengl32.dll',
 r'C:\WINDOWS\system32\3dfxogl.dll', r'C:\RETRO_AGENT\3dfx-driver\3dfxogl.dll',
 r'C:\RETRO_AGENT\gltest\opengl32.dll',
]
async def rc(c,cmd,payload=None,t=30):
    s,d=await c.send_command(cmd,binary_payload=payload,timeout=t); return d.decode('ascii','replace')
async def killwait(c,img,t=12):
    import asyncio as a
    for _ in range(8):
        tl=await rc(c, r'EXEC cmd /c tasklist 2>nul | findstr /i %s'%img, t=t)
        if img.lower() not in tl.lower(): return
        await rc(c, r'EXEC taskkill /f /im %s 2>nul'%img, t=t); await a.sleep(2)
async def deploy_icd(c):
    ICD=open(ICD_SRC,'rb').read()
    await rc(c, r'UPLOAD %s\3dfxogl-stage.dll'%ST, payload=ICD, t=120)
    bad=[]
    for tgt in TARGETS:
        r=await rc(c, r'EXEC cmd /c copy /Y %s\3dfxogl-stage.dll "%s"'%(ST,tgt), t=20)
        if '1 file' not in r: bad.append(tgt)
    return len(ICD), bad
def rimg(d): return Image.frombytes('RGB',(640,480),d).transpose(Image.FLIP_TOP_BOTTOM)
def fb_to_img(d):
    w,h=640,480; img=Image.new('RGB',(w,h)); px=img.load()
    for y in range(h):
        row=struct.unpack_from('<%dH'%w,d,y*w*2)
        for x in range(w): v=row[x]; px[x,y]=(((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31)
    return img

async def run_case(letter,x,y,er,eg,eb):
    GFIX=open(GFIX_SRC,'rb').read()
    c=RetroConnection(IP,9898); await c.connect('retro-agent-secret',timeout=12)
    await killwait(c,'gfix.exe')
    await c.send_command(r'UPLOAD C:\RETRO_AGENT\gltest\gfix.exe', binary_payload=GFIX, timeout=90)
    ln,bad=await deploy_icd(c)
    await rc(c, r'EXEC cmd /c del /f /q C:\gfix_%s.raw C:\gfix.log 2>nul'%letter, t=12)
    await rc(c, r'EXECW 120 cmd /c cd /d C:\RETRO_AGENT\gltest ^&^& gfix.exe', t=135)
    await killwait(c,'gfix.exe')
    log=await rc(c, r'EXEC cmd /c type C:\gfix.log 2>nul', t=12)
    clean='done' in log
    await rc(c, r'EXEC cmd /c C:\RETRO_AGENT\setmode.exe 1024 768 32 85 2>nul', t=12)
    try:
        d=await c.command_binary(r'DOWNLOAD C:\gfix_%s.raw'%letter, timeout=90)
        p=rimg(d).load()[x,y]
    except Exception as e:
        await c.close(); print("FAIL: no dump (crash?) log-tail:", log.splitlines()[-3:]); return
    await c.close()
    ok=all(abs(p[i]-[er,eg,eb][i])<25 for i in range(3))
    green=p[1]>p[0]+25 and p[0]<90
    print("case %s @(%d,%d) = %s  expect (%d,%d,%d)  %s%s (icd %dB, clean=%s)"%(
        letter,x,y,p,er,eg,eb,'PASS' if ok else 'FAIL', ' [GREEN]' if green else '', ln, clean))

async def run_cs(secs):
    c=RetroConnection(IP,9898); await c.connect('retro-agent-secret',timeout=12)
    await killwait(c,'hl.exe')
    ln,bad=await deploy_icd(c)
    await rc(c, r'EXEC cmd /c echo x > C:\icd_fbdump.on', t=12)
    await rc(c, r'EXEC cmd /c del /f /q C:\fbdump_*.raw 2>nul', t=12)
    await rc(c, r'EXEC cmd /c cd /d "%s" ^&^& start "" hl.exe -game cstrike -gl -w 640 -h 480 -full -console -noipx -nojoy +map de_dust'%CS, t=15)
    import asyncio as a; await a.sleep(secs)
    await killwait(c,'hl.exe')
    await rc(c, r'EXEC cmd /c del /f /q C:\icd_fbdump.on 2>nul', t=12)
    await rc(c, r'EXEC cmd /c C:\RETRO_AGENT\setmode.exe 1024 768 32 85 2>nul', t=12)
    dl=await rc(c, r'EXEC cmd /c dir /b C:\fbdump_*.raw 2>nul', t=12)
    names=[x.strip() for x in dl.split() if x.strip().endswith('.raw')]
    if not names: await c.close(); print("no fbdump"); return
    d=await c.command_binary(r'DOWNLOAD C:\%s'%names[-1], timeout=120)
    await c.close()
    img=fb_to_img(d); img.save('/tmp/gloop_cs.png')
    # sample several wall/ground points; report green-ness fraction
    px=img.load(); pts=[(90,200),(300,300),(140,240),(520,300)]
    greens=sum(1 for (x,y) in pts if px[x,y][1]>px[x,y][0]+25 and px[x,y][0]<90)
    print("CS fbdump %s: samples=%s  green %d/%d  (icd %dB) -> /tmp/gloop_cs.png"%(
        names[-1],[px[x,y] for x,y in pts],greens,len(pts),ln))

def main():
    a=sys.argv
    if a[1]=='case':
        asyncio.run(run_case(a[2],int(a[3]),int(a[4]),int(a[5]),int(a[6]),int(a[7])))
    elif a[1]=='cs':
        asyncio.run(run_cs(int(a[2]) if len(a)>2 else 100))
    elif a[1]=='deploy':
        async def d():
            c=RetroConnection(IP,9898); await c.connect('retro-agent-secret',timeout=12)
            ln,bad=await deploy_icd(c); await c.close()
            print("deployed %dB, failed:%s"%(ln,bad or 'none'))
        asyncio.run(d())
main()
