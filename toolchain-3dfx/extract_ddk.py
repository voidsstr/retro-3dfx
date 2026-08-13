#!/usr/bin/env python3
"""Rebuild the W2K DDK install layout from its CAB/INF pairs (no setup.exe needed).

CABs store files as <CABNAME>_FILE_<N>; each INF's [Files_X] sections map real
filenames to those members, and [DestinationDirs] maps each Files_X section to a
directory relative to the DDK root (LDID 49000). LDID 17 (%windir%\\inf) entries
are installer bookkeeping and are skipped.
"""
import re, subprocess, sys, tempfile
from pathlib import Path

SRC = Path(sys.argv[1])          # CABS/I386 dir
DEST = Path(sys.argv[2])         # w2kddk root
SEVENZ = Path(sys.argv[3])

def parse_inf(text):
    sections = {}
    cur = None
    for line in text.splitlines():
        line = line.strip()
        if not line or line.startswith(';'):
            continue
        m = re.match(r'\[(.+?)\]', line)
        if m:
            cur = m.group(1).strip()
            sections.setdefault(cur, [])
            continue
        if cur:
            sections[cur].append(line)
    return sections

total = skipped = 0
for inf_path in sorted(SRC.glob('*.INF')):
    cab_path = inf_path.with_suffix('.CAB')
    if not cab_path.exists():
        continue
    sections = parse_inf(inf_path.read_text(errors='replace'))
    destdirs = {}
    for line in sections.get('DestinationDirs', []):
        m = re.match(r'([^=\s]+)\s*=\s*(\d+)\s*(?:,\s*"?([^"]*?)"?\s*)?$', line)
        if m:
            sect, ldid, sub = m.group(1), m.group(2), (m.group(3) or '')
            # The XP DDK's INFs write the subdir with a leading backslash
            # (49000,\inc\ddk). Left as-is that becomes an ABSOLUTE '/inc/ddk'
            # and DEST/sub silently discards DEST -- the extractor then tries to
            # write to the filesystem root. Strip the leading separator.
            destdirs[sect.lower()] = (ldid, sub.replace('\\', '/').lstrip('/'))
    copy_sections = []
    for line in sections.get('DefaultInstall', []):
        m = re.match(r'CopyFiles\s*=\s*(.+)', line, re.I)
        if m:
            copy_sections += [s.strip() for s in m.group(1).split(',') if s.strip()]
    if not copy_sections:
        continue
    with tempfile.TemporaryDirectory() as td:
        r = subprocess.run([str(SEVENZ), 'x', '-y', f'-o{td}', str(cab_path)],
                           capture_output=True, text=True)
        if r.returncode != 0:
            print(f'!! extract failed: {cab_path.name}', file=sys.stderr)
            continue
        tdp = Path(td)
        for sect in copy_sections:
            ldid, sub = destdirs.get(sect.lower(), destdirs.get('defaultdestdir', ('49000', '')))
            if ldid != '49000':
                continue  # windows\inf bookkeeping
            for line in sections.get(sect, []):
                m = re.match(r'([^,\s]+)\s*,\s*([^,\s]+)', line)
                if not m:
                    continue
                real, member = m.group(1), m.group(2)
                srcf = tdp / member
                if not srcf.exists():
                    skipped += 1
                    continue
                out = DEST / sub / real
                out.parent.mkdir(parents=True, exist_ok=True)
                out.write_bytes(srcf.read_bytes())
                total += 1
    print(f'{cab_path.name}: done')
print(f'placed {total} files, {skipped} members missing')
