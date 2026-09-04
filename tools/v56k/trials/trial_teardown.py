#!/usr/bin/env python3
"""Persistent visual trial - TEARDOWN half.  Run ONLY after the operator has
given their verdict."""
import sys, asyncio, json
sys.path.insert(0,"/home/voidsstr/development/retro-agent")
from client.retro_protocol import RetroConnection
IP='192.168.1.191'
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
    if c is None: print('box unreachable'); return 2
    for p in [p['pid'] for p in json.loads((await c.send_command('PROCLIST',timeout=15))[1].decode('ascii','replace')) if p['name'].lower()=='quake3.exe']:
        await c.command_text('EXECW 20 cmd /c taskkill /f /pid %d 2>nul & echo ok'%p, timeout=45)
    await c.command_text('DISPLAYCFG set 1024 768 16 85', timeout=25)
    up=json.loads((await c.send_command('SYSINFO',timeout=12))[1].decode())['uptime_seconds']
    print('torn down; box healthy (uptime %ds). Pokes revert at the next mode set.'%up)
    await c.close()
asyncio.run(m())
