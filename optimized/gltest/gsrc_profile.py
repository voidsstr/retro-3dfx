#!/usr/bin/env python3
"""gsrc_profile.py -- autonomous CPU/GPU profiler for GoldSrc (Counter-Strike)
on the Voodoo5 box (.143), driven through the 3dfx ICD's own instrumentation so
it works under BCShield.

Answers: CPU-bound or GPU-bound, is vsync hurting fps, and where the per-frame
time goes (CPU transform/T&L vs GPU submit vs vsync/swap wait).

Realistic load (NOT a dumb fixed sweep): it fills the server with LOTS of zbots
(the install's gamedll is zbotcz.dll) and puts the profiled client in SPECTATOR
with the auto-director -- an AI camera that chases the bot firefights -- so the
frames contain the "lots of players nearby" scenes that tank the framerate.

Instrumentation (enabled BEFORE launch -- the ICD latches its gate on the first
swap, so it can't attach to an already-running game):
  * C:\\icd_perf.on -> C:\\icd_perf.log : fps10, maxFrame, texDl.
  * C:\\icd_prof.on -> C:\\3dfxprof.log : flush% (GPU submit) / swap% (vsync+swap
    wait) / other% (CPU: game + GL T&L).
Runs the same scene with vsync ON then OFF so the fps delta isolates the
double-buffer vsync penalty. Optionally grabs ONE front-buffer frame at the end
(proof the AI camera is on a bot firefight) -- a single capture, not the heavy
repeated readback.

Usage:  python3 gsrc_profile.py [bots] [secs] [shot]
        python3 gsrc_profile.py 20 35 shot
Exit 0; prints the A/B table + bottleneck verdict; leaves the box clean.
"""
import asyncio, sys, functools, struct
sys.path.insert(0,'/home/voidsstr/development/retro-agent')
from client.retro_protocol import RetroConnection
print=functools.partial(print,flush=True)
try:
    from PIL import Image
except Exception:
    Image=None

HOST='192.168.1.143'
CS=r'C:\Program Files\Bcs16 Romania\Counter-Strike 1.6'
GLIDEKEY=r'HKLM\SYSTEM\CurrentControlSet\Services\3dfxvs\Device0\glide'
BOTS = int(sys.argv[1]) if len(sys.argv)>1 else 20
SECS = int(sys.argv[2]) if len(sys.argv)>2 else 35
SHOT = 'shot' in sys.argv

async def rc(c,cmd,t=45):
    s,d=await c.send_command(cmd,timeout=t); return d.decode('ascii','replace')

def cfg(bots):
    """Fill with bots and JOIN as a player, then noclip through the bot warzone.
    NOTE: the spectator auto-director idea does NOT work on this BCShield build --
    its VGUI team menu ignores menuselect and injected input is blocked in
    full-screen Glide, so the client gets stuck at SELECT TEAM. The proven path
    (same as goldsrc_bench) is chooseteam -> menuselect 5 (auto team) ->
    menuselect 5 (class) -> spawn, then noclip. The bots (AI) create the load;
    the camera sweeps through their firefights."""
    w=lambda n:'\r\n'.join(['wait']*n)
    return ('developer 0\r\nfps_max 1000\r\nhud_draw 1\r\nbot_quota_mode fill\r\n'
            'bot_join_after_player 0\r\nbot_difficulty 2\r\nbot_quota %d\r\n'%bots
            + w(60) + '\r\nchooseteam\r\n'+w(25)+'\r\nmenuselect 5\r\n'+w(25)+'\r\nmenuselect 5\r\n'+w(120)+
            '\r\nsv_cheats 1\r\nnoclip\r\ncl_yawspeed 40\r\n+forward\r\n+right\r\n').encode('latin-1')

def dig(s): return int(''.join(ch for ch in s if ch.isdigit()) or '0')

def raw565_to_png(raw,out):
    if Image is None: return None
    w=640; h=len(raw)//(w*2)
    if h<=0: return None
    im=Image.new('RGB',(w,h)); px=im.load()
    for y in range(h):
        row=struct.unpack_from('<%dH'%w,raw,y*w*2)
        for x in range(w): v=row[x]; px[x,y]=(((v>>11)&31)*255//31,((v>>5)&63)*255//63,(v&31)*255//31)
    im.save(out); return out

async def one_run(c, vsync, tag, shot=False):
    await rc(c, r'EXEC taskkill /f /im hl.exe 2>nul', t=20); await asyncio.sleep(4)
    await rc(c, r'EXEC cmd /c reg add "%s" /v FX_GLIDE_SWAPINTERVAL /t REG_SZ /d %d /f'%(GLIDEKEY,vsync), t=15)
    await c.send_command(r'UPLOAD %s\cstrike\listenserver.cfg'%CS, binary_payload=cfg(BOTS), timeout=30)
    gate = r'& echo x > C:\icd_perf.on & echo x > C:\icd_prof.on' + (r' & echo x > C:\icd_fbdump.on' if shot else '')
    await rc(c, r'EXEC cmd /c del /f /q C:\icd_perf.log C:\3dfxprof.log C:\fbdump_*.raw 2>nul '+gate, t=15)
    await rc(c, r'EXEC cmd /c cd /d "%s" ^&^& start "" hl.exe -game cstrike -gl -w 1024 -h 768 -full '
                r'-noipx -nojoy +set _vgui_menus 0 +maxplayers 32 +map de_dust'%CS, t=20)
    await asyncio.sleep(SECS)
    cl=await rc(c, r'EXEC cmd /c wmic cpu get loadpercentage 2>nul', t=18)
    cpu=next((x for x in cl.split() if x.isdigit()), '?')
    perf=await rc(c, r'EXEC cmd /c type C:\icd_perf.log 2>nul', t=25)
    prof=await rc(c, r'EXEC cmd /c type C:\3dfxprof.log 2>nul', t=25)
    png=None
    if shot:
        names=sorted(x.strip() for x in (await rc(c, r'EXEC cmd /c dir /b C:\fbdump_*.raw 2>nul', t=15)).split() if x.strip().lower().endswith('.raw'))
        if names:
            rawd=await c.command_binary(r'DOWNLOAD C:\%s'%names[-1], timeout=180)
            png=raw565_to_png(rawd, '/tmp/gsrc_scene.png')
        await rc(c, r'EXEC cmd /c del /f /q C:\icd_fbdump.on 2>nul', t=10)
    rows=[l for l in perf.splitlines() if l.startswith('f=')]
    fps=[]; hit=[]; texdl=0
    for l in rows:
        d=dict(kv.split('=') for kv in l.split() if '=' in kv)
        fps.append(dig(d.get('fps10','0'))/10.0); hit.append(dig(d.get('maxFrame','0'))); texdl+=dig(d.get('texDl','0'))
    body=fps[3:] if len(fps)>4 else fps
    fmin=min(body) if body else 0; favg=sum(body)/len(body) if body else 0; fmax=max(body) if body else 0
    fl=[]; sw=[]; ot=[]
    for l in prof.splitlines():
        if 'flush%' not in l: continue
        try:
            d={k:v for k,v in (p.split('=') for p in l.replace('%','').split() if '=' in p)}
            fl.append(float(d.get('flush',0))); sw.append(float(d.get('swap',0))); ot.append(float(d.get('other',0)))
        except: pass
    avg=lambda a: sum(a)/len(a) if a else 0
    print("[%s] vsync=%d | fps min%.0f avg%.0f max%.0f | worst %dms | CPU %s%% | "
          "flush(GPU)%.0f%% swap(vsync)%.0f%% other(CPU)%.0f%% | texDl %d (perfN=%d profN=%d)%s"
          %(tag,vsync,fmin,favg,fmax,max(hit) if hit else 0,cpu,avg(fl),avg(sw),avg(ot),texdl,len(rows),len(fl),
            '  scene='+png if png else ''))
    return {'favg':favg,'fmin':fmin,'flush':avg(fl),'swap':avg(sw),'other':avg(ot),'cpu':cpu,'texdl':texdl,'png':png}

async def main():
    c=RetroConnection(HOST,9898); await c.connect('retro-agent-secret',timeout=25)
    print("=== GoldSrc AI-load profile: %d bots + spectator auto-director, %ds/run, vsync A/B (de_dust) ==="%(BOTS,SECS))
    on =await one_run(c, 1, 'VSYNC-ON')
    off=await one_run(c, 0, 'VSYNC-OFF', shot=SHOT)
    await rc(c, r'EXEC taskkill /f /im hl.exe 2>nul', t=20); await asyncio.sleep(3)
    await rc(c, r'EXEC cmd /c del /f /q "%s\cstrike\listenserver.cfg" C:\icd_perf.on C:\icd_prof.on C:\icd_fbdump.on 2>nul'%CS, t=15)
    try: await c.send_command('DISPLAYCFG set 1024 768 32 85', timeout=30)
    except Exception: pass
    print("\n=== VERDICT ===")
    g=off['favg']-on['favg']
    print("vsync OFF vs ON: %.0f -> %.0f fps (%+.0f, %+.0f%%)"%(on['favg'],off['favg'],g,100*g/on['favg'] if on['favg'] else 0))
    print("bottleneck: CPU %s%%, GPU-submit(flush) %.0f%% -> %s"%(off['cpu'],off['flush'],
          "CPU-BOUND (GPU idle-waits)" if off['flush']<15 else "GPU has headroom"))
    print("vsync ON wasted %.0f%% of each frame in vblank wait (the fps penalty)."%on['swap'])
    print("texture streaming (texDl over run): ON %d / OFF %d"%(on['texdl'],off['texdl']))
    await c.close()
asyncio.run(main())
