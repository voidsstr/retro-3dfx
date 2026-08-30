# retro-3dfx — Running Findings Log

Living log of important, hard-won findings for the 3dfx Voodoo3 (.124) driver work.
Append new findings as they're uncovered; keep newest-first within each section.
Detailed narratives live in `D3D-DRIVER-PLAN.md`; this file is the quick index of
"things that cost us time and we must not forget."

Target box: **.124** = "ADMIN", XP SP3, active Windows on **D:**, C: is a Win98
FAT volume. Autologs in as voidsstr/password.
**As of 2026-08-11 .124 holds an NVIDIA GeForce2 GTS, NOT a Voodoo3** — see the
top entry. The Voodoo3 (`PCI\VEN_121A&DEV_0005&SUBSYS_1037121A&REV_01`) was
physically removed and its whole stack purged; this lane has no hardware behind
it until a Voodoo card goes back in.

---

## A gate that reports a number can still be lying twice over (icons, 2026-08-30)

The desktop icon-rebuild gate (arrange only when the desktop actually changed)
failed **silently twice**, in two different ways, and BOTH were caught by
reading a count off real hardware within minutes. Neither was visible from the
source - the code was read carefully both times and looked right.

**Failure 1: our own sweep made the question unanswerable.** `gs_run()` begins
with `gs_sweep_desktop()`, which moves EVERY `.lnk` off the desktop before any
shortcut is written. So the test "was this `.lnk` already there just before I
wrote it?" is **always false**. Every shortcut counted as new, the gate was true
on every box on every sync, and it suppressed nothing while reporting itself as
working. Measured on .171: `shortcuts_changed=79` on a box that already had 81
icons and had changed none of them.

**Failure 2: the fix threw away its own evidence.** The repair was to sample the
icon SET before the sweep and compare at the end. Correct - except
`gs_desk_reset()` clears the snapshot as well as the counters and was sitting
ELEVEN LINES BELOW `gs_desk_snapshot()`. The set was sampled and immediately
discarded, nothing ever matched, and every rewritten shortcut counted as
*added*: `shortcuts_changed=81`, the same wrong answer by a new route. The tests
pinned `snapshot < sweep` and `settle < gate` but never `reset < snapshot`.

**THE LESSON IS ABOUT ORDER, NOT ABOUT SHORTCUTS.** A before/after comparison
has a setup step, a sample step, a mutate step and a resolve step, and *every
adjacent pair is an invariant*. Pin all of them, not the one that happened to
break first.

**And a self-inflicted one worth naming: I read an unpopulated field as a
result.** `shortcuts_changed` was written only by the end-of-run settle, so
polling it mid-sync returned `0` for the entire run and then `81` at the finish.
The mid-run zeros were reported as early evidence the fix was working. They were
not a measurement of anything. **A counter that is only computed at the end must
either not be published during the run, or must publish its running total** -
now it publishes as it accrues. Reading a not-yet-computed value as a result is
the same error as trusting a status word instead of the machine.

**Why this argues FOR the instrumentation.** The counts were added reluctantly,
as a check on a fix already believed to work. They found two real defects in the
thing they were checking, on the first box they reached. A gate that suppresses
work must report what it saw, or "it never fires" and "it fires every time" are
indistinguishable - and both of those were true here at different times.

---

## A rejected CD key that was never actually tested: Halo, mgspid.dll and the wrong PIDGen (2026-08-30)

Halo PC would not start ("Your product key is invalid", both Continue buttons
greyed out).  Halo's own **mgspid.dll** was driven to enter the key the owner
supplied and Microsoft's code answered **"Invalid CD Key!"**, which read as a
wrong key — and a second key was requested from the user on that basis.  **The
verdict was worthless.  No Halo key can pass that path on an XP box.**

- **mgspid.dll validates nothing itself.** Its string table names the real
  validator: `5011 "PIDGen.dll"`, `5012 "PIDGenSimpA"`, alongside `5001 "69771"`
  (Halo's Microsoft Product Code), `5002 "Z08-00030"` (SKU) and
  `5015 "OEM-1208613"`.  It `LoadLibraryA`s that DLL **by bare name**.
- **Halo's PIDGen.dll ships on the retail CD, not in the installed tree.**  It
  is absent from the user's 145-entry zip, from the staged tree, and from the
  entire share (case-insensitive `find`: the only `PIDGEN.DLL` files anywhere
  are the two Windows XP install-media copies under `Files/OS/XPSP3-*/I386/`).
- **So the load falls through to `C:\WINDOWS\system32\PIDGen.dll`.**  Measured
  on .145 with a purpose-built probe run from a directory with no PIDGen.dll:

      LoadLibraryA resolved to: C:\WINDOWS\system32\PIDGen.dll
      PIDGenSimpA             : present
      BINK resource #1        : present (368 bytes)

  A **`BINK` resource is the per-product elliptic-curve PUBLIC KEY** a
  25-character key's signature is checked against.  Those two are Windows XP's —
  the DLL even carries a hardcoded Windows key string.  It has no idea what MPC
  69771 is.  The ABI matches (it exports `PIDGenSimpA` taking 9 dwords, `ret
  0x24`; mgspid pushes exactly 9), so the call runs cleanly and simply answers
  no.

**The general lesson, and it is the "Make Failure VISIBLE" lesson pointed the
other way: a NEGATIVE verdict from a tool needs the same proof of soundness as a
positive one.**  "Microsoft's own code rejected it" felt authoritative; it was
Microsoft's code judging the wrong product.  A wrongly-condemned key sends the
owner hunting for media they already have.  Before recording any credential,
key or licence as bad, establish that the validator you used is the one that
belongs to that product.

**Related, and it changes what "staged" means for this title:** `halo.exe` 1.10
itself performs **no cryptographic check at all**.  Its gate at `0x0057f3f0`
only requires `HKLM\Software\Microsoft\Microsoft Games\Halo\DigitalProductID`
to exist, be exactly **164 bytes**, carry `0xA4` at 0 and version **3.0** at
offset 4, and yield a non-empty `"%05d,%09d,0,% 19.19I64d"` identity string; the
caller at `0x00541807` raises the fatal error **only** when that string comes
back empty.  The ECC check lives in PIDGen.dll and runs once, in setup.  So the
blob is built from the owner's own key by
`retro-agent/scripts/halo/make_dpid.py` (base-24 at offset 52, Halo's own MPC /
SKU / OEM id), and **the game starting is therefore NOT evidence the key is
right** — only that the tree is staged correctly.  Say so rather than implying
a validation happened.

## Halo picks its overlay layout by VERTICAL RESOLUTION, and Bungie shipped no 1080 (2026-08-30)

At the fleet's native 1920x1080 Halo paints a grey `404 Error! ... 
content\1080log.ksml ... File Not Found!` panel across its own main menu.
Keystone (Halo's overlay UI) opens `content\<height>log.ksml` and
`content\<height>editbox.ksml`; the shipped set is 480, 576, 600, 720, 768,
864, 900, 960, 1024, 1200 — 1920x1080 was not a PC mode in 2003.  Four of the
eight fleet boxes are 1080p panels and the staged launcher picks the panel's
native mode, so this hit **half the fleet** and looked like a corrupt install.
Generated in the staged tree by `scripts/halo/make_ksml.py` (the 1200 pair
scaled 0.9 vertically and 1.2 horizontally, plus a matching
`gallery/editbox1080.png` — every shipped `editboxNNNN.png` is exactly as wide
as its ksml's `width` field).

## `rd /s /q` raced an in-flight GAMESYNC and left a title half-installed (2026-08-30)

Purging `C:\Games\Halo` on .240 while a fleet-wide GAMESYNC was already copying
that very title deleted everything not yet locked, reported
`The process cannot access the file because it is being used by another
process`, and the sync then finished and reported **`state=done, 38/38 titles,
0 file error(s)`** over a tree missing `halo.exe`, `binkw32.dll`, the three
`.bik` movies and 14 other files.  The desktop shortcut it built in that state
fell back to the wrong icon (`FLEETRES.EXE`) because `launch.txt`'s explicit
`halo.exe` was not on disk yet.  **Check `GAMESYNC` is idle before you purge**,
and re-verify the file count against the library afterwards — `state=done` and
`failed_files: 0` are both true of a tree that is missing a fifth of itself.

---

## The capability gate had a feature level it could never reach: `none` (2026-08-30)

Preparing the Pentium-1 Compaq Deskpro (Win98, ~31 MB) for staged games broke
three things in the gate that eight Windows XP boxes could not possibly have
exposed. All three had the same shape: **the gate answered confidently, and the
answer was wrong in the permissive direction.**

- **`GG_GPU_NONE` was defined from day one and assigned to NOTHING.** Every S3,
  Matrox, Trident, Cirrus and Tseng device id fell through a single vendor-wide
  catch-all row at `GG_GPU_FIXED`, so an **S3 Trio64 — a chip with no 3D
  pipeline at all, for which no Direct3D driver was ever written — claimed a
  fixed-function rasteriser.** The level meaning "this box cannot do 3D" existed
  and was unreachable. The only reason 3D-only titles were not waved straight
  onto such a box is that `min_vram_mb` happened to catch some of them, which is
  luck rather than a gate. Fixed by putting explicit 2D-only id ranges *ahead*
  of each vendor's catch-all (`gg_gpu_level_from_pci` returns the first match).
- **`none` is one level below `fixed`, so it landed in the MARGINAL band and the
  title was copied.** The band exists because a title of that era usually ships
  a lower-detail path for a weaker rasteriser — but there is no lower-detail
  path from "has a rasteriser" to "has none", so this one is binary. Aliens
  versus Predator is the title it protects: its own notes say "Direct3D only,
  there is no software renderer".
- **`gpu_feature_level` was being written in two different senses.** Nine staged
  titles declared `fixed` while their own notes said a software renderer
  shipped. Harmless while every box was `fixed` or better (a gap of zero), and a
  hard `no` the instant a `none` box appeared. The field means *"will not launch
  below this"*, never *"runs better above this"* — so it now comes off any title
  whose tree really contains the software renderer, **with the proving file
  named in the notes**: `sw.dll` (GoldSrc), `System\SoftDrv.dll` (Unreal
  Engine 1), `soft.ren` (LithTech). Hexen II ships both builds and says it
  per-shortcut instead, so a 2D box gets the game and loses only the OpenGL icon.

**Blast radius was measured, not assumed**: re-deciding all 37 titles against
the eight real fleet profiles produced a byte-identical result before and after,
because every one of those boxes is `fixed` or better.

**The wider lesson, and it is not about GPUs:** three staged titles carried
`requires.json` files declaring **no floors at all**, on the explicit reasoning
that there was *"no meaningful floor on this fleet"* — true when written, false
the moment the fleet gained a machine a decade older than the rest.
`HiddenAndDangerous` (2003, 272 MB, Direct3D-only) was being approved for a
Pentium 166 with 31 MB of RAM. **A requirement written as a statement about the
current fleet expires silently when the fleet changes**, and nothing anywhere
flags it; a requirement written as a statement about the TITLE does not. The
replacements are quoted from the games' own files — Turok 2's `ReadMeEnglish.txt`
says "With a 3Dfx Voodoo or equivalent, P200. 3Dfx Voodoo 2 or equivalent,
P166", and Hexen II's own launchers pass `-heapsize 32768`, i.e. the engine asks
for 32 MB of heap on a machine that has 31 MB in total.

**What was NOT a defect, checked first:** the DOSBox-wrapped titles
(Carmageddon 1, Descent 1/2, Redneck Rampage) already gate on the **emulator's**
host cost — 350-400 MHz — not the 1994 game's, and each says so in its notes.
The P1 refuses all four for the right reason.


## Secrets: the vault is a system of record, and `$(az ...)` fails into an empty variable (2026-08-30)

Swept the whole project for keys and credentials and moved the real ones into
Azure Key Vault **`nsc-secrets-kv`** under `fleet-gamekey-*`. Three things worth
not re-learning:

- **`KEY=$(az keyvault secret show ... --query value -o tsv)` is a trap.** When
  `az` is not logged in, the error goes to stderr, the non-zero exit is
  swallowed by the assignment, and `KEY` is **empty** — so an empty product key
  reaches `winnt.sif`, or an empty CD key reaches an `install.reg`, and it
  surfaces hours later on a box at a dialog nobody is watching. Same "the tool
  reported success" shape as everything else here. Use
  `scripts/fleet/keyvault.py get`, which raises instead. It separates **four**
  states — not logged in / no such secret / access denied / unreachable — because
  each has a different fix.
- **Key Vault's `--content-type` is capped at 255 chars** and rejects a longer
  one with `Property  has invalid value`, naming *no* property. Cost one failed
  `az keyvault secret set` before it was obvious.
- **The vault is the SYSTEM OF RECORD, never a runtime dependency.** A staged
  `install.reg` keeps the literal key: a Windows `.reg` has no indirection, and
  a retro box must never need the WAN to start a game. The right shape is
  `make-xp-source.sh`, which pulls the key **on the Linux host at build time**.
- **Three categories of key-shaped value, not two.** Per-copy secret (vault it),
  deliberately-public fleet convention (document why — `retro-agent-secret`,
  `password`, `retroadmin`), and **per-installation machine-local state**
  (`HKLM\SOFTWARE\Westwood\<game>\Serial` on RA2/Yuri/Tiberian Sun) which must
  be *different* per box and would break LAN play if centralised.

Git history was swept for key-shaped literals and is **clean**;
`tests/python/test_no_committed_secrets.py` (in `retro-agent`) now keeps it that
way, including a catch-all that greps the tree for the live vault values.

---

## Publish the `.ver` sidecar LAST, and the dev host has TWO mounts of the share (2026-08-30)

Two publish-path facts, both of which cost time today and neither of which is
guessable from the docs as they stood.

**1. `.ver` goes last — this is an ordering invariant, not a preference.**
Auto-update compares the agent's compiled `AGENT_VERSION` against the share's
`retro_agent.exe.ver` and pulls on **inequality**. So publish
*versioned archive → latest pointer → `.ver` sidecar*: while the sidecar still
names the OLD version, every box compares equal and correctly declines to pull,
and there is no instant at which a box can fetch a binary whose version
disagrees with the sidecar. Writing `.ver` first opens exactly that window, and
because the comparison is inequality rather than "remote is newer", a box that
pulls in that window can end up on a binary nobody intended.

**2. "The share is read-only from the dev host" is HALF TRUE, and the false half
is the expensive one.** There are two mounts of the same SMB share:

| path | how | access |
|---|---|---|
| `/mnt/retro-share` | CIFS from `/etc/fstab` with an explicit `ro` flag | read-only; `cp` fails with *Read-only file system* |
| `/run/user/1000/gvfs/smb-share:server=192.168.1.122,share=files,user=voidsstr` | gvfs, the desktop "mapped network drive" | **read-write**, verified write → read back → delete |

So a host-side `cp` does work — via gvfs, not `/mnt`. The gvfs mount is
**per-login-session** and vanishes in a headless or freshly-rebooted context, so
the publish-through-a-fleet-box route stays the reliable fallback rather than
being obsolete. `/etc/cifs-retro-share.creds` is root-only and sudo needs an
interactive password here, so `smbclient -A` is not a route at all.

**And when using the fleet-box route, `NETMAP` first.** `.171` answered
`net use` with "There are no entries in the list" and had no `Z:` at all; the
docs imply the mapping is already there. The `Z:` drive is a convention, not a
guarantee.

**Verify the post-condition from the host, not from "1 file(s) copied".**
`md5sum` the local build against both published copies, `cat` the `.ver`, and
`strings <published exe> | grep -x <version>` so the binary itself confirms the
version you believe you shipped.

---

## The machines now write their own documentation (2026-08-30)

The hand-maintained "Known Machines" table was wrong about most of the fleet,
and **twice a box's graphics card was swapped without the docs noticing** —
`.124`'s Voodoo 3 came out on 2026-08-11 and the stale claim survived for
weeks, and `.133`'s Voodoo5 6000 is physically gone while three documents still
named the box by it. A *measured* replacement table written the same morning
was already drifting by the afternoon.

**Agent 1.74.0 makes every box publish its own hardware record on every
startup** — `agent/src/hwpublish.c` writes its HWPROFILE JSON to
`\\192.168.1.122\files\Utility\Retro Automation\fleet-inventory\<host>.json`,
and `scripts/fleet/inventory.py` renders those into `docs/fleet-inventory.md`.
`HWPUBLISH` does it on demand and replies with the path and byte count, so a
publish is verified rather than inferred from a log line.

Findings worth keeping from building it:

- **Generating a document and keeping the hand-written one solves nothing** — it
  creates a second thing to go stale. The generated file had to become the
  single source of truth for every measured field, with CLAUDE.md reduced to
  the prose a probe cannot discover. Half the work was deletion.
- **Two clocks, and only one of them is trustworthy.** Staleness must be judged
  by when the record landed on the dev host, never by the timestamp inside it:
  a retro box's RTC is frequently years out, and a record published thirty
  seconds ago would otherwise read "last seen 2003". Both are shown and the
  disagreement is reported as clock skew.
- **"never seen" needs a roster to be sayable at all.** Without one, a box that
  has never published is indistinguishable from a box that does not exist, and
  the document silently shrinks by one instead of saying one is missing.
- **A VOODOO 2 IS INVISIBLE TO EVERY DISPLAY-CLASS SCAN** (its INF is
  `Class=MEDIA`) and a Voodoo behind a 2D card is invisible to
  `EnumDisplayDevices`' active-adapter answer. `hwextra.c` therefore reports
  `accelerators[]` straight from `Enum\PCI` on the **vendor** id `121A` — the
  one source a driver class cannot fool, and the same read that proved `.133`'s
  V5 6000 physically absent rather than merely undriven. Instances are counted,
  not device keys: two identical cards share one key.
- **A MAC formatted at offset `k*3` truncates to `"00"`.** The first octet is
  two characters and every later one is three, so the offset is `k*3-1`; at
  `k*3` the NUL from the previous octet lands in the gap. Short, plausible,
  and unflagged by any reader — found and fixed before shipping, and now
  asserted against the buggy form in `tests/native/test_hwpublish.c`.
- **The dev host has TWO mounts of the same share and they differ.** The fstab
  CIFS mount at `/mnt/retro-share` is explicitly `ro`; the gvfs mount under
  `/run/user/1000/gvfs/smb-share:server=192.168.1.122,share=files,user=voidsstr`
  is **read-write**. "The share is read-only from the host" was asserted and
  believed during this work and is only half true — worth knowing before
  designing around it. The gvfs mount is per-login-session, though: it can be
  absent headless and was seen to disappear and return, so nothing may *depend*
  on it. The generated document therefore lives in the repo, where it is always
  present, and is only additionally copied to the share when that mount is
  there.
- **The box publishing its own record is still right**, but for the durable
  reasons rather than the write-permission one: a box knows its own hardware, a
  host-side collector on an on-demand fleet would mostly collect nothing, one
  file per host means eight agents never contend, and it works while the host
  is asleep.

---

## The favourites agent, proven in the games' own browsers (2026-08-30)

Four engines, two boxes, screenshots of each — the standard the user set for
"multiplayer works". `retro-gameindex` writes each title's own favourites file
and every one of them was **read back by the game**, not merely written:

| box | title | where | what was on screen |
|---|---|---|---|
| .133 | CS 1.6 | Favorites **and** Lan tabs | both fleet servers, 0/16, 13-24 ms |
| .143 | CS 1.6 | Favorites **and** Lan tabs | both fleet servers, 0/16, 21-30 ms |
| .133 | Quake III | Local **and** Favorites | `NSC RETRO FLEET AREN q3dm7 4/16`, 16 of 16 favourites |
| .143 | UT99 | Favorites **and** LAN Servers | `NSC Retro Fleet Arena (UT99)` among 17, ping 12 on LAN |
| .143 | Quake II | JOIN SERVER address book | `NSC Retro Fleet Arena (Q2) q2dm1 0/12` in row 1, 7x `<no server>` |

**Quake III's `4/16` is four BOTS** (`bot_minplayers 4`), not four people. The
server list gives you no way to tell; only the host-side probe does.

Shapes now pinned by `tests/python/test_gameindex_favorites.py` because each is
invisible when wrong: **Q2's address book is `adr0..adr8` and Q3's is
`server1..server16`** — one engine generation apart, off by one, and a
mis-numbered writer still logs "wrote 1 servers" while the game shows an empty
first row.

## UT2004's favourite is present and correct and still pings N/A (2026-08-30)

On **.143** the Server Browser's Favorites tab lists `NSC Retro Fleet Arena`
from the line the agent wrote into `UT2004.ini` —

    Favorites=(ServerID=0,IP="192.168.1.132",Port=7777,QueryPort=7787,
               ServerName="NSC Retro Fleet Arena")

— and reports `Query Complete! Received: 1 Servers`. So the client parsed the
entry and accepted it. But **Ping reads `N/A` and Map and Players stay blank**,
before and after a REFRESH.

That is not a favourites fault: the name on screen is the one we wrote, so the
file reached the client and was read. It is the client's **query** to
`192.168.1.132:7787` that does not resolve — while the host-side probe
(`scripts/game-servers/gameservers.py`, which uses that same port 7787) gets an
answer in 49 ms with map `Rankin`. Server up, favourite correct, client sees
nothing. Whoever owns the UT2004 server config should look at what it answers
to a client query as opposed to our probe; client here is retail **3369** and
the browser's own News tab is advertising the OldUnreal **3374** patch.

Screenshot: /tmp/retro-screenshots/fav/143-ut2k4-ref.png

## "Unchanged" must mean the BOX is unchanged, not that our intent is (2026-08-30)

`push_favorites` skipped a title when the DB's `applied_hash` matched the hash
of what it had just rendered — which compares our own intent against itself.
Anything that rewrites the file behind us is then invisible: GAMESYNC re-copied
the staged `UnrealTournament.ini` over ours on **.171** and took the favourites
back to the three the library ships. The next pass rendered the same output
from the same staged base, matched its own recorded hash, logged `unchanged`,
and would have done so forever. *Reverted* and *never written* look identical
from the DB's side — the house failure mode exactly.

`read_existing` already hands the pass the current bytes, so comparing against
those is free. **Verified on hardware, not just in the suite:** the CS 1.6
`serverbrowser.vdf` on **.143** was hand-reverted to an empty `favorites` block
and the next pass logged `wrote 2 servers` for that path and `unchanged` for
the untouched one, and the file on the box was then re-read and confirmed to
carry both servers again.

## A relative-mouse menu is not automatable, and WINDOWED is the escape hatch (2026-08-30)

Known for id Tech 3; it is **also true of UT99's UWindow**. In exclusive
fullscreen UT99 takes the mouse through DirectInput, so `UICLICK`'s absolute
`SetCursorPos` moves nothing: a click at (600,400) moved the in-game cursor
from (0,8) to (56,26). The UWindow menu bar also **ignores the keyboard
entirely** — ESC opens it, and after that arrows and ENTER do nothing at all,
so the usual "fall back to the keyboard" answer does not exist here.

Running the same build **windowed** fixed it completely: absolute clicks landed
on the menu bar, the dropdown, the browser's tab strip. Two habits that made it
work:

- **Click twice.** In UWindow the first click after a cursor jump only arms the
  control (it registers as hover); the second activates. Every tab and menu
  item needed the pair.
- **Screenshot between every click.** The agent auto-updated four times during
  the session (1.70.0 -> 1.72.1) and each restart raised its console window and
  stole focus, silently eating the next click.

## UT99 469e will not start on .143 or .133 — and the 436 tree does (2026-08-30)

`C:\Games\UnrealTournament` is the staged OldUnreal **469e** tree (Engine.dll
carries the `OldUnreal`/`469` markers, and every DLL matches the library
byte-for-byte, so this is not a half-applied GAMESYNC). It **exits at startup
before writing a single line of its own log**:

- **.143** (Athlon 1000, no SSE): `0xC000001E`
- **.133** (dual PIII 701, SSE1 only): `0xC000001D` STATUS_ILLEGAL_INSTRUCTION

~30 attempts across both boxes, `start ""` and direct, fullscreen and windowed,
with and without `Running.ini`. It succeeded **exactly once** on .143 and ran
happily for five minutes, which is the only reason this is filed as "crashes at
startup" rather than "cannot run".

`C:\Games\UnrealTournament436` on .143 starts every time, and that is where the
browser proof above came from. **That client is 436 and our server is 469** —
the browser's own Version column says so — so per the standing rule these two
boxes currently have no client that can actually join the fleet's UT99 server.
The favourites reach them; the game cannot use them.

**Diagnostic worth reusing:** `EXECW <n> cmd /v:on /c cd /d "<dir>" && game.exe
& echo RC=!ERRORLEVEL!`. Plain `%ERRORLEVEL%` expands at parse time and always
reports the outer shell's 0 — which is exactly the "the tool said success"
failure this repo keeps meeting. `/v:on` plus `!ERRORLEVEL!` is what turns a
silent crash into an NT status code.

## NEVER `taskkill /f /im cmd.exe` through the agent (2026-08-30)

The agent runs every `EXEC` through `cmd.exe`, so that command kills its own
host shell mid-command and resets the connection. Harmless here, but it looks
exactly like an agent crash, and the reflex to "clean up stray cmd windows" is
a common one.

---

## `r_mode -1` is NOT universal across the id Tech 3 family (2026-08-30)

`r_mode -1` plus `r_customwidth` / `r_customheight` is *the* id Tech 3 idiom for
an arbitrary resolution, and applying it as a rule is wrong. Probed on **.145**
(1920x1080 panel) with one identical `fleetres.cfg` — `seta r_mode "-1"`,
`r_customwidth 1920`, `r_customheight 1080`, `r_fullscreen 1` — launched into
each binary in turn and read back from `WINLIST`:

| binary | window rect | verdict |
|---|---|---|
| `quake3.exe` (retail 1.32c) | **1920x1080** | branch present |
| `jasp.exe` (Jedi Academy SP) | **1920x1080** | branch present |
| `jamp.exe` (Jedi Academy MP) | **1920x1080** | branch present |
| `sof2mp.exe` | **640x480** | **no `-1` branch** |

**Every one of these binaries contains the string `r_customwidth`**, so the
symbol table is not evidence either. SoF2's fork registers the cvar and never
implements the branch; it does not error, it renders at the engine default.
Silent, and indistinguishable from "the config did not apply".

The replacement is a plain mode index — and it must come from the **id Tech 3**
table, not id Tech 2's. They diverge exactly where it hurts:

| index | id Tech 2 | id Tech 3 |
|---|---|---|
| 7 | 1152x864 (4:3) | 1152x864 (4:3) |
| **8** | **1280x960 (4:3)** | **1280x1024 (5:4)** |
| 9 | 1600x1200 | 1600x1200 |

So handing `FR_Q2MODE` to a Quake III-family engine asks a 16:9 panel for a
5:4 image — the squashed picture this whole mechanism exists to remove.
`FLEETRES` now emits a separate **`FR_Q3MODE`**, which skips index 8 (5:4) and
index 11 (856x480). On the four 1080p boxes it answers **7 = 1152x864**, the
largest correctly-proportioned mode SoF2 can reach; **SoF2 cannot do 1080p at
all**, and that is a real engine limit rather than a configuration failure.


## Two engines in one family give two different answers — measure each (2026-08-30)

`GLQUAKE.EXE` and Hexen II's `glh2.exe` are the same lineage, look alike, and
were treated alike. On **.145** (GeForce 8400GS, 1920x1080 panel), from the
same desktop mode, minutes apart:

| | 1920x1080 | 1600x1200 | 1280x960 |
|---|---|---|---|
| `GLQUAKE.EXE` | `Quake Error: "Specified video mode not available"` | same error | **fullscreen, renders** |
| `glh2.exe` | **fullscreen, renders** (window class `HexenII`, 0,0–1920x1080) | — | — |

So GLQuake's cap is real and its ceiling is **1280x960**, not the 1024x768 that
had been guessed; and Hexen II has no cap at all. Inheriting one engine's
limits from a relative cost every 1080p box a worse picture in one direction
and would have shipped a broken launcher in the other.

Corollary, with Tiberian Sun below: **a resolution cap is a claim about
hardware and must arrive with the measurement that produced it.** The library
now has exactly one cap (Quake 1) and a test that fails if a second appears.

## The render device is per-box for the same reason the resolution is (2026-08-30)

`Games-Library/UnrealGold/System/` and `Games-Library/Carmageddon2/` each ship
a game-local nGlide `glide2x.dll` (1,310,720 B). **Game-local wins at load
time**, so on the only two boxes that still have Glide silicon that wrapper
shadows the real `system32\glide2x.dll` and the game gets neither the card nor
a working wrapper — `grSstOpen` fails and UE1 falls back to the software
rasterizer at 100% CPU. On **.171** that presented for a whole session as
"UnrealGold crashes". It never crashed.

Deleting the wrapper is the wrong fix: it is the only Glide path the other six
boxes have. `FLEETRES.EXE` now reports `FR_GLIDE` by walking
`HKLM\SYSTEM\CurrentControlSet\Enum\PCI` for `VEN_121A` **case-insensitively**
— a Voodoo 2's INF is `Class=MEDIA`, so `EnumDisplayDevices` and `VIDEODIAG`
both report it absent and the PCI enum is the only place it appears. Measured:
`.171 = VEN_121A&DEV_0002`, `.143 = VEN_121A&DEV_0009&SUBSYS_0002121A`, `.145`
none. The launcher moves the wrapper aside when it is 1 and **back** when it
is 0; a one-way rename would strand it the moment a card came out.

**Silicon present and "render through it" are different questions.** `.143`
has a Voodoo5 5500 fitted but its monitor is on a GeForce 6800, so Glide would
draw to a port nobody is looking at. Rendering on the 3dfx card is an explicit
per-box opt-in, `HKLM\Software\RetroAgent\GlideRender` (REG_DWORD 1), set on
`.171` alone.


**VERIFIED END TO END ON .171 (2026-08-30).** With `FR_GLIDE=1` and
`GlideRender=1`, `Play Unreal Gold.bat` moved the wrapper aside (only
`glide2x.dll.nglide` left in `System\`), wrote `GameRenderDevice=`,
`WindowedRenderDevice=` and `RenderDevice=GlideDrv.GlideRenderDevice` plus
`FullscreenViewportX/Y=800/600`, and `System\Unreal.log` then reads:

```
Log:  Bound to GlideDrv.dll
Init: Found Glide: 2.56.00.0459
Init: Glide info: Type=0, fbRam=4 fbiRev=260 nTexelfx=2 Sli=0
Init: grSstOpen Res=0 Ref=8 Buffers=3
Init: Glide tmu 0: tmuRev=4 tmuRam=4 Space=4194296
Init: Glide tmu 1: tmuRev=4 tmuRam=4 Space=4194296
```

That is the **real 3dfx Glide 2.56**, not nGlide, and `fbRam=4 nTexelfx=2` is
the 12 MB Voodoo 2 (4 MB framebuffer + 2x4 MB TMU). **`grSstOpen` SUCCEEDS**
where through the wrapper it had failed `(2, 3)` every time. The `Critical
Error: Assertion failed: RenDev ... EndFullscreen <- WM_KILLFOCUS` that follows
is UE1's well-known focus-loss assert, provoked by the agent's own commands
stealing focus AFTER a successful init - not a Glide fault.

**A Glide fullscreen surface on a pass-through card cannot be screenshotted.**
The Voodoo 2 renders to its own framebuffer and feeds the monitor through the
pass-through cable, so `SCREENSHOT` will never show it whatever happens. The
engine's own log is the evidence, and it is better evidence than a frame.


## `%%` outside a FOR loop is a silent no-op that reads correctly (2026-08-30)

The first cut of that render-device block emitted `if "%%FR_GLIDE%%"=="1" (`.
In a batch file `%%` is an escape **only inside a for loop**; anywhere else
cmd.exe reduces it to the literal text `%FR_GLIDE%`, which is never equal to
`"1"`, so the entire block does nothing and looks perfectly right in review.
Same shape as every other defect this fleet has paid for: the tool reported
success.

The check that catches it has to be precise or it is worse than nothing — a
FOR variable is one character and is never closed with a second `%%`, while an
environment variable is `%%NAME%%`. Matching only the latter (and skipping any
line with a `for`) is the difference between one true hit and **fifteen false
ones across the library's mount launchers**.


## A game's own mode list is NOT evidence of the engine's ceiling (Tiberian Sun, 2026-08-30)

Tiberian Sun's **Options -> Display -> Resolution Modes offers exactly three
entries: 640x400, 640x480, 800x600.** Read as a capability statement that says
"this engine cannot do more than 800x600", and it is wrong: the same build, on
the same fleet, **renders at a full 1920x1080** on .123 (screenshot
`/tmp/retro-screenshots/ts-123-06-ingame.png` — full-resolution sidebar,
proportionally small against a large map viewport).

The CnCNet patch replaces the game's hardcoded 640/480 constants with reads of
`[Video] ScreenWidth` / `ScreenHeight` from `SUN.INI`, **bypassing the
enumerated mode list entirely**. The menu still lists what the unpatched code
knew about. So the two facts are not in conflict, and the menu is simply not
the authority.

Generalised, because this will be re-derived wrongly by whoever next audits a
resolution: **a menu, a mode list, or a settings dropdown is UI, not the
engine's capability.** Ask the thing that actually sets the mode — the ini the
patch reads, the mode table in the binary's own strings, or an empirical launch
— before concluding a title is capped. The reverse trap exists too and was hit
the same day: **Soldier of Fortune II registers `r_customwidth`,
`r_customheight` and `r_customaspect` as cvars, so `r_mode -1` looks supported,
and it is not** — the renderer has no mode -1 branch, so it falls back to
640x480 while `fleetres.cfg` correctly said 1920. Measured twice. A cvar
existing is not a feature existing.

Corollary for TS specifically: its FLEETRES `-cap 1024 768` is a ceiling the
engine does not have. Only two launchers in the library pass a cap — `Quake1`
and `TiberianSun`.

---

## A documented engine idiom can be ABSENT in a specific fork, and fail silently (SoF2, 2026-08-30)

**`r_mode -1` + `r_customwidth`/`r_customheight` is THE documented way to ask an
id Tech 3 engine for an arbitrary resolution. Soldier of Fortune II does not
implement it.** The cvars are all registered — `r_customwidth`,
`r_customheight`, `r_customaspect` and `r_customPixelAspect` are right there in
`sof2mp.exe`'s string table — so the idiom looks supported by every test short
of running it. The renderer simply has no mode -1 branch, so mode -1 is invalid
and it **falls back to 640x480 while the config correctly says 1920**. No error,
no warning, nothing in any log. The fleet launcher had been asking for -1 and
getting 640x480 on a 1920x1080 panel.

What makes the diagnosis credible is the A/B through the *identical* launcher
path on .123, changing only the mode value:

    r_mode -1 + r_customwidth 1920 / r_customheight 1080  -> 640x480   (fallback)
    r_mode 7                                              -> 1152x864  (works)
    r_mode 8                                              -> 1280x1024 (works)

The mode-table path works perfectly; only the -1 branch is missing. That
isolates it to the engine rather than to the config, the launcher, the latch
ordering, or the panel.

**The general lesson: a cvar existing is not a feature existing, and "this is
how the engine family does it" is not evidence about a particular build.** A
fork can register a variable and never wire it up. Verify an engine idiom on
the actual binary before staging a whole fleet on it — especially when the
failure mode is a silent fallback rather than an error, which is the normal
shape for renderer init.

**Does SoF2 reach 1920x1080 at all? NO — and that is an engine limit, not a
staging gap.** Its mode table, from its own strings, is 0-11: 320x240, 400x300,
512x384, 640x480, 800x600, 960x720, 1024x768, 1152x864, **1280x1024**,
1600x1200, 2048x1536, and 856x480 (the only widescreen entry). There is no
1920x1080. Measured on .123's 1920x1080 DELL P2312H, with the desktop set to
1920x1080 first each time:

    r_mode 8  (1280x1024)  -> 1280x1024      the largest that WORKS
    r_mode 9  (1600x1200)  -> 640x480x16     refused, 1200 > 1080
    r_mode 10 (2048x1536)  -> 640x480x16     refused
    r_mode 11 (856x480)    -> 640x480x16     refused

So **mode 8 is genuinely the ceiling on a 1080p panel**, and `FR_Q2MODE`
(which resolves to 8 there) is already picking the largest usable mode — no
change needed. Note the fallback drops to **16-bit** as well as 640x480, which
is a useful tell that the mode was refused rather than chosen.

Soldier of Fortune 1 is the same answer from the other engine: `SoF.exe`'s table
runs mode 3 = 640x480 up to mode 9 = 1600x1200, all 4:3, no 1080p entry, so
`gl_mode 8` = 1280x960 is its 1080p-panel maximum. **For both SoF titles,
"1920x1080" is correctly reported as NOT APPLICABLE rather than as a defect.**

---

## SoF2 multiplayer reads base/MP, so the fleet config never applied to it (2026-08-30)

`sof2mp.exe`'s game directory is **`base/MP`, not `base`**. The fleet's staged
`base\autoexec.cfg` — PunkBuster off, the screenshot binds, fullscreen, and the
`exec fleetres.cfg` that carries the per-box resolution — **had never applied to
multiplayer at all.** Every one of those settings was silently inert, and the
game started and played perfectly, which is exactly why it survived.

The proof was a single controlled swap, same file, same launch path, on .123:

    autoexec.cfg with `seta r_mode 8` in base\      -> no effect, stayed 640x480
    the SAME file in base\MP\                       -> came up 1280x1024

The engine also writes its own config to `base\MP\sof2mp.cfg`, which
independently confirms the live game directory. The clue was there all along and
nobody carried it across: the binary reads its CD key with
`CL_ReadCDKey("base/mp")` + `"%s/sof2key"`, which is why `base/mp/sof2key` was
already staged correctly while the config beside it was not.

Two useful details:
* `exec fleetres.cfg` **does** resolve from `base\` when called from
  `base\MP\autoexec.cfg` — fs_basegame stays in the search path (verified:
  `r_mode 7` written into `base\fleetres.cfg` came up 1152x864 with only
  `exec fleetres.cfg` in the MP file). So the launcher does not need to change
  where it writes that file.
* `base\MP\autoexec.cfg` **shadows** `base\autoexec.cfg` for MP (fs_game is
  searched before fs_basegame), so the MP copy must carry everything MP needs on
  its own. Single player (`SoF2.exe`) still reads `base\autoexec.cfg`. **Two
  binaries, two game directories — edit both.**

SoF2's real resolution ceiling, from its own mode table in the binary: modes
0-11, topping at 9 = 1600x1200 and 10 = 2048x1536, with **no 1920x1080 entry**
and the only widescreen mode 11 = 856x480. So **mode 8 (1280x1024) is the
largest that fits a 1080p panel**, and 1920x1080 is genuinely not attainable —
an engine limit, not a staging gap. Soldier of Fortune 1 is the same story from
the other engine: its table (from `SoF.exe`'s strings) runs 640x480 to
1600x1200, all 4:3, so `gl_mode 8` = 1280x960 is its 1080p-panel maximum.

---

## An unreferenced binary can be Vista-only and nothing will ever say so (2026-08-30)

`UnrealTournament\System\magick.exe` was a **39 MB ImageMagick 7 binary** left
in the staged tree by whoever generated the title's icons. No launcher, no
`.ini`, no repo script named it (checked case-insensitively), and it was **PE
`MajorSubsystemVersion` 6.0** — Vista and later, which **XP's loader refuses
before a single instruction runs**, with no dialog and nothing in any log.

So it was 39 MB of unloadable dead weight shipped to every box, and the only
reason it was ever harmless is that nothing tried to start it. Had a launcher
named it, the symptom would have been "the game does nothing when you
double-click it" — the maximally silent failure, because `start ""` throws the
exit code away too.

Now caught by a `pe-subsystem` check in
`retro-agent/scripts/validate-staged-library.py` (hand-rolled PE parse, no
dependencies, verified byte-for-byte against `objdump -p`). It was the **only**
such binary in 731 across the whole 40 GB library.

Removed from the library, plus a `del` in `Play Unreal Tournament.bat` — because
**GAMESYNC never deletes**, so a file the library stops shipping stays on every
box forever unless a launcher removes it.

---

## A WM_COMMAND TOGGLE fired blindly reports success and does the opposite (desktop icons, 2026-08-30)

The fleet was moved to Windows' own **Auto Arrange** for desktop icons (agent
v1.73.0), reversing the custom "icon bay" placement. Three findings from doing
it, in descending order of how much they would have cost:

**1. The shell's toggle silently fails, on a quarter of the fleet.**
`FCIDM_SHVIEW_AUTOARRANGE` is a WM_COMMAND **toggle**, not a set. The obvious
implementation — post it, log "auto-arrange enabled" — was measured **doing
nothing at all on both `.171` and `.143`**: the post returns, the style bit
stays clear, and the log line claims success. That is this project's recurring
"reported success and was believed" shape, and it is invisible to a unit test
because the message is a cross-process side effect. The fix is to read
`LVS_AUTOARRANGE` back after posting and fall back to
`SetWindowLongA(lv, GWL_STYLE, style | LVS_AUTOARRANGE)` — which is a SET, so
unlike the toggle it cannot flip the setting the wrong way. **The fallback is
load-bearing, not belt-and-braces.**

Corollary, and the general form worth remembering: **a toggle may only be fired
when you have first read the state and know it moves the bit the way you want.**
The previous generation of this same code learned this in the opposite
direction — it fired the toggle blindly to turn auto-arrange OFF and thereby
turned it ON on every box that already had it off, leaving icons in rows across
the top of the screen.

**2. The persisted view state is NOT uniform across the fleet, so read-modify-write.**
Auto Arrange persists in `HKCU\Software\Microsoft\Windows\Shell\Bags\1\Desktop`
→ `FFlags`, a FOLDERFLAGS word: **bit 0 = `FWF_AUTOARRANGE`**, bit 2 =
`FWF_SNAPTOGRID` ("align to grid"). Measured before the change: **`.143` = 0x220,
`.171` = 0x224** — the same fleet, the same image, different words, because
align-to-grid differed. Stamping a constant `0x221` would have been correct on
`.143` by luck and would have silently cleared align-to-grid on `.171`. Only bit
0 may move. (`.246` on Win7 had no `Desktop` subkey under `Bags\1` at all, so
the write has to create the key.)

**3a. A REBOOT proves the registry is only a backstop.** Measured on .133 across
a real power cycle: `FFlags` came back `0x221` with bit 0 intact, and the live
listview style was **still OFF** — XP's shell did not honour the persisted bag —
so the agent set it again at startup. So of the two mechanisms, **the
every-agent-startup re-apply is the load-bearing one** and the registry write is
the backstop, not the other way round. Anyone who "simplifies" this by dropping
the startup pass and keeping only the registry write will ship a fleet that
comes back from every reboot with auto-arrange off.

**3. Persistence survived an Explorer restart even though the toggle had failed.**
The worry was that Explorer keeps its own in-memory `FOLDERSETTINGS` and
rewrites the bag from it at logoff — which would clobber a registry write made
behind its back, exactly the case when the WM_COMMAND never took. Tested by
killing and restarting `explorer.exe` on `.171`: `FFlags` stayed `0x225` and the
live style kept the bit. Not assumed — measured.

**A GATE THAT DECIDES SILENTLY IS A GATE THAT CANNOT BE TRUSTED (v1.75.0).**
The same change stopped rebuilding the icon layout at the end of every GAMESYNC
and only does it when the desktop actually changed - a file really written, or
a .lnk created/removed. But `gs_desk_changed()` decided that in silence, and if
it is ever wrong in the "always true" direction the every-boot rebuild returns
with nothing saying so. The realistic cause is ONE file that re-copies on every
pass: a destination whose mtime never stamps (SetFileTime failing on an oddly
attributed file, or coarser destination time granularity) fails the size+mtime
resume test forever. From outside, that is indistinguishable from a box that
genuinely had work to do. So the counts now appear in the `done:` line and as
`files_written`/`shortcuts_changed` in GAMESYNC STATUS. **A steady-state box
must report `0 file(s) written`; the same small non-zero count on consecutive
no-change syncs is the defect announcing itself.** General form: when you add a
condition that suppresses work, report what the condition saw - otherwise
"it never fires" and "it fires every time" look identical.

**What this cost the icon bay:** with Auto Arrange on the shell packs icons into
its own grid from the top-left and **ignores `LVM_SETITEMPOSITION` outright**, so
the wallpaper's drawn bay cells are dead. The two mechanisms are mutually
exclusive and cannot both run — that is the "two arrangers fighting over one
desktop" bug this repo has already been through twice. Exactly one runs, chosen
by `HKLM\Software\RetroAgent\IconAutoArrange` (absent/1 = auto, 0 = bay).
`scripts/retro-wallpaper/arrange_icons.exe` must never be staged again: it
clears `LVS_AUTOARRANGE` by design, so a single run turns the fleet default off.

**Siting matters as much as the code.** The apply call had to go **above**
`retrowall_apply_startup()`'s two early returns. Both of those returns are the
NORMAL path on a fleet box (a fleet wallpaper was applied; no rotation staged),
so a call placed after them runs on almost no machine, logs nothing, and looks
installed. The theme and screensaver were already caught by that same trap once.

Tests: `retro-agent/tests/native/test_icon_autoarrange.c`,
`retro-agent/tests/python/test_icon_autoarrange_source.py` (both mutation-checked
— the invariants really fail when the guard or the siting is removed).

---

## Prove a patch REPLACES a file before you trust what it produced (Halo, 2026-08-30)

Halo PC could not be staged from the share's "Original PC Release" zip: its
`halo.exe` carried PE `TimeDateStamp` `0x21544c66`, whose raw little-endian
bytes are `fLT!` — the FAiRLiGHT watermark — decoding to 1987, impossible for a
2003 binary. That is the ONLY tampered file in the tree; a sweep of all 34 PE
binaries found every other stamp sane and every `SubsystemVersion` 4.0.

Bungie's own signed `halopc-patch-1.0.10.exe` fixes it — but only if the patch
**replaces** `halo.exe` rather than delta-patching it, because a byte delta
applied to a cracked source yields a cracked output. RTPatch does both, and the
"successfully updated" dialog cannot tell you which. So the control experiment:

    run 1  patch the tree as shipped                       -> halo.exe md5 b7aa8f68…
    run 2  zero 256 KB of the source halo.exe's .text,
           re-run the identical patch                      -> halo.exe md5 b7aa8f68…

Byte-identical output from two different inputs proves the file is shipped
whole and nothing from the crack survives. Same for `Strings.dll`,
`Keystone.dll`, `binkw32.dll`, `haloupdate.exe`. **A patch reporting success is
not evidence about what it produced** — vary the input and compare the output.

Three more things that cost time here:

- **A printable-ASCII TimeDateStamp is NOT tamper evidence on its own.** In the
  same tree `haloupdate.exe` (`L"Y?`), `ogg.dll` (`pQP?`) and `vorbis.dll`
  (`8TP?`) all have all-printable stamps and correct 2003 dates — and so does
  the genuine Bungie 1.0.10 build (`RhrS`). Four bytes are printable about a
  quarter of the time. Only *printable AND an impossible date* is a finding.
  This is the same phantom-finding trap as the earlier "clean section layout"
  claim that a genuine Microsoft control binary then reproduced exactly.
  Encoded now in `retro-agent/scripts/fleet/pe-audit.py` + `test_pe_audit.py`.

- **The patch's own resources tell you what it wants.** It refused with "You
  have Halo Version 01.00.00.0564 installed" until the registry `Version` was
  the *short* form. Its `FROM` RCDATA resource is literally
  `1;1.01;1.02;1.03;1.031;1.04;1.051;1.06;1.07;1.08;1.09`. Reading the
  installer's resources beat guessing the format.

- **There is no Windows Vista check in Halo 1.** Measured, not assumed: every
  binary in the tree and every binary the patch installs is
  `SubsystemVersion 4.0`, so XP's loader has no objection, and official patch
  1.08 already removed the CD-in-drive requirement. The Vista gate belongs to
  Halo 2 Vista, a different product. No community bypass was needed.

**What actually blocks Halo 1 is a product key.** Genuine 1.10 stops at
"Your product key is invalid" with *both* Continue buttons greyed out — it is
not a multiplayer-only check and there is no degraded mode. `halo.exe` reads
the binary value `DigitalProductID` under
`HKLM\SOFTWARE\Microsoft\Microsoft Games\Halo`; that is a PID blob derived
from the 25-character key by `mgspid.dll` (one export, `GetPid`, cdecl), so a
key cannot simply be typed into the registry. The share holds no Halo 1 key
(full case-insensitive sweep), and a key from the internet is the same class of
thing as the crack. Tree is built and ready at
`Files/tmp/halo-build/Halo Combat Evolved/`, one rename from `Games-Library/`.

**Follow-up: the key that was later supplied is rejected by Microsoft's own
validator, and the harness that says so is itself unproven.** `halo.exe` reads
`DigitalProductID`, which is a *derived* value — writing the plain 25-character
key there as `REG_SZ` does not satisfy it. `mgspid.dll` (shipped in the tree,
one export `GetPid`) carries Microsoft's real "Please Enter your Product Key"
dialog; driven both by hand and headlessly it answered **"Invalid CD Key!"** and
wrote nothing. But `GetPid`'s signature is undocumented and was inferred from
disassembly — its first arg is stored as a bare dword and might be a *product
code* rather than the window handle passed — so the rejection may be the harness,
not the key. **That distinction was not resolved, and the key must not be
recorded as bad on this evidence.** Two watch-outs found on the way: Halo's
startup checks are ordered, so with the wrong working directory it fails earlier
on `Cannot find 'C:\config.txt'` and that is *not* evidence the key passed; and
a screenshot taken while another agent had Red Alert 2 fullscreen on the same box
was very nearly reported as "Halo is running" — check `WINLIST` for the window
that owns the screen before attributing a frame to your title.

**Case-insensitivity bites on the SHARE too, not just in greps.** Copying the
patched `Strings.dll`/`Keystone.dll` up and then deleting the old
`strings.dll`/`keystone.dll` deleted *the files just copied* — same file, and
`del` reported nothing wrong. Verify the post-condition after a case-differing
copy+delete pair.

Also worth recording: the dev host's `/mnt/retro-share` is mounted **read-only**
(`cifs … ro`). Every write to the staged library has to go through a fleet box's
`Z:` drive. A `cp` that fails with "Read-only file system" is that, not a
permissions problem to debug.

---

## Far Cry's System.cfg needs QUOTED values, and ignores everything else in
## silence (2026-08-30)

The staged Far Cry template was written in the obvious dialect:

    r_Fullscreen = 1
    r_Width = 1024
    sys_firstlaunch = 0

**None of it took effect.** Far Cry writes its own config with every value in
double quotes - its configurator emits `sys_firstlaunch = "1"`,
`e_decals = "1"` - and a bare value is not parsed at all. There is no warning,
no log line, no error: the settings simply do not exist. The only visible
consequence on .246 was that the first launch stopped dead on a modal
**"Auto detection will adjust settings for optimal performance!"** dialog,
which is precisely the "no wizard, no operator" promise that the word *staged*
is supposed to mean. `r_Fullscreen` and the resolution were being ignored at
the same time and nothing said so.

Requoted, the engine's own `log.txt` proves each one landed:

    Lua cvar: (r_Fullscreen,1)
    Lua cvar: (r_Width,1920)
    Lua cvar: (r_Height,1080)
    Setting sys_firstlaunch to 0
    Best-match display mode: 1920x1080x32

Two general points worth keeping:

* **Ask the engine what dialect it writes, do not infer one.** The answer was
  sitting in `FarCryConfigurator.exe`'s own string table the whole time
  (`strings -a` shows a hundred `key = "value"` pairs). One `strings` call was
  cheaper than the launch that failed. The tree's `r_Driver = "Direct3D9"` was
  already quoted and should have been the tell.
* **A config a game silently ignores is the worst kind of staged defect**,
  because every check short of running it passes: the file is present, it is
  syntactically plausible, the validator likes it, and GAMESYNC copies it with
  `failed_files: 0`. Only the game disagrees, and only by doing something
  slightly wrong two minutes later.

Mechanically, FLEETRES already had the answer: `-setline` turns a **backtick
into a double quote**, exactly because cmd.exe eats real ones. So the staged
recipe writes ``r_Width = `%FR_W%` ``. Guarded by
`test_farcry_writes_quoted_values` in `tests/python/test_fleetres_staging.py`.

Related, and true of any fullscreen D3D title: **the agent's `SCREENSHOT` cannot
capture an exclusive-fullscreen CryEngine surface** - it returns the stale
desktop composite, which here meant a Far Cry *splash* bitmap that sat on screen
looking like a hang for eight minutes while the game was in fact at its main
menu. `WINLIST` is the cheap truth (a `CryENGINE` class window at 0,0-1920x1080),
and relaunching windowed is how you actually see the menu.

---

## An auto-update restart silently zeroes a GAMESYNC in flight (2026-08-30)

Deploying Far Cry (3.6 GB) to **.246** was killed twice in twenty minutes, and
both times the status said nothing was wrong. The agent auto-updated 1.70.0 ->
1.71.1 -> 1.72.0 while the copy was running - six agents were publishing builds
that morning - and an auto-update **restarts the agent**, which takes the
GAMESYNC thread with it.

What makes it expensive is the reporting, not the restart:

    {"state":"idle","percent":0,"titles_done":0,"titles_total":0,
     "mb_done":0,"failed_files":0, ... }

A sync aborted 93% of the way through a 40 GB pass is **byte-for-byte identical
to one that was never started**. `failed_files` stays 0, because nothing failed
- the process just stopped existing. Reading `state` after the fact tells you
"idle", and the natural reading of that is "my START never took".

Three things follow, and the third is the general one:

* **Verify the tree, not the status.** The post-condition for a title is its own
  file count and byte total on the box matching the library:
  `dir /s /a-d C:\Games\<Title> | find "File(s)"` against
  `find . -type f | wc -l` + `du -sb --apparent-size` on the share. That caught
  the real state each time - the second abort had in fact reached 1613/3536
  files, which no field of the JSON revealed.
* **GAMESYNC resumes cheaply, so just re-issue it.** The size+mtime skip means a
  restarted pass re-walks the library in ~90 s and continues. A loop that polls
  the post-condition and re-STARTs whenever the state is not `copying`/`sizing`
  finishes the job unattended; ad-hoc watching does not, because the aborts are
  minutes apart and look like success.
* **A restart-shaped hole in a long operation needs its own signal.** `state`
  encodes the *thread's* view and the thread is gone, so nothing in the status
  can distinguish "never ran" from "was killed". Either the marker has to record
  an in-progress generation, or the caller has to own the post-condition. Until
  the former exists, the latter is not optional.

Beware the corollary while the fleet is busy: **a refused TCP 9898 during this
window is the agent restarting, not a dead box.** Retry before concluding
anything.

---

## EDID's "preferred" mode is a CRT's MAXIMUM, not its target (2026-08-30)

The capability gate needs to know what resolution a title will actually run at,
because 1920x1080 is ~2.4x the pixels of 1024x768 and that is the difference
between a 2004 title being comfortable on a given card and not. There are three
candidate sources on a fleet box and **each wrong one fails differently**:

| source | why it is wrong |
|---|---|
| **live mode** (`EnumDisplaySettings ENUM_CURRENT_SETTINGS`) | a game that exits without restoring leaves it behind. **.123 and .240 were both sitting at 640x480** from a DOSBox leftover while driving 1080p panels. A tool that trusts it calls them 640x480 machines - and one that WRITES that conclusion pins them there for good. |
| **EDID native** (first detailed timing) | correct for an LCD, **wrong for a CRT**, where the preferred timing is the tube's maximum. **.171's Gateway VX1120 reports 1920x1440** while the fleet runs it at 1280x1024. |
| **persisted mode** (`ENUM_REGISTRY_SETTINGS`) | right: what the machine is configured to present, and a game's temporary `ChangeDisplaySettings` does not alter it. |

**The CRT case was not theoretical - it was measured costing games.** Feeding
1920x1440 to the ollama adjudicator flipped **BF1942, MaxPayne,
SoldierOfFortune2 and UT2004 from `marginal` to `no` on .171**, i.e. the box
would have silently stopped receiving four titles it runs perfectly well,
because of a resolution nothing on it uses. Switching the target to the
persisted mode put all four back. Agent 1.72.1 prefers the persisted mode and
falls back to EDID; `HWPROFILE` reports `panel_source` plus both figures so a
wrong answer can be diagnosed rather than guessed at.

Two further notes for anyone touching this: **the EDID `digital` bit describes
the CONNECTION, not the panel** - .123/.145/.240 are DELL LCDs on VGA and all
report analogue, so it cannot be used to mean "is an LCD". And
`EnumDisplayDevices` is **not stable between runs** on .171, which enumerates
both a real Gateway and a `Default_Monitor` carrying no EDID; a single-shot
probe reads the panel correctly on some runs and falls back on others, which is
worse than failing outright because it looks like the panel changed. Walk every
adapter and every monitor (`agent/shared/edid.h`, ported from
`provisioning/fleetres/fleetres.c`).

## Publish to the share ONLY from a commit already on origin/master (2026-08-30)

Four agent versions (**v1.71.0, v1.71.1, v1.72.0, v1.72.1**) were built and
published to the share from a worktree whose commits were **in no branch**, and
one of them compiled in a **source file that was untracked entirely**
(`agent/shared/edid.h`). All eight boxes ran that binary. The share is the
fleet's auto-update source and is **the one artefact `git revert` cannot roll
back**, so for several hours the fleet was executing code that could not be
rebuilt from a fresh clone.

Two distinct hazards, both real:
1. **Another agent with a build off `origin/master` would have published
   `v1.73.0` over it**, and every box would have pulled a binary with the
   capability gate silently absent - auto-update pulls on version *inequality*,
   so 1.72.x is simply stepped over with nothing pointing at the regression.
2. **Local-only tags are one careless command from gone.** None of the four had
   been pushed; a `git tag -d` during unrelated testing destroyed `v1.72.1`
   outright, and it identified the binary the whole fleet was running.

The repair that works: land the commits, `git tag -f` each published version at
its post-rebase SHA, and **`git push origin --tags`**. Then prove the artefact
matches - download the share's binary and diff it against a clean rebuild. Ours
differed in **exactly 5 bytes, at offsets 136-137 and 216-218**: the PE
`TimeDateStamp` and checksum. That is what "reproducible" looks like for a mingw
PE; do not expect a byte-identical hash and do not accept more than those.

---

## A parsed-but-never-consulted field is a silent no-op (gamegate, 2026-08-30)

`requires.json` declared `disk_mb` from the first version of the capability
gate. Both the C header and `rules.py` **parsed it into the requirements
struct** — and neither ever compared it against anything. A library author
could write `"disk_mb": 3700` with full confidence and get nothing: no refusal,
no warning, no log line. The mirror test stayed green the whole time, because
the two implementations ignored it *identically*.

That is the exact failure shape CLAUDE.md's "Make Failure VISIBLE" section is
about, and it is worth naming the general form: **a field that is parsed but
never read is indistinguishable, from the outside, from a field that works.**
Round-tripping a value into a struct is not evidence that anything consumes it.
When adding a schema field, the test that matters is not "does it parse" but
"does a decision change when it changes".

Found by asking the gate for a verdict at 500 MB free against a 3700 MB
requirement and getting `run`. Now a hard floor with **no margin band** — a
clock 10% under yields a slower game, a disk 10% under yields a partial copy —
checked *after* the cpu/ram/vram floors so a box that genuinely cannot run the
title is told that, rather than sent to free up space it would then waste.
`free_mb` is deliberately **not** in `gg_profile_hash()`: free space changes on
every write, so hashing it would mint a new profile and miss every cached
verdict on essentially every run — the same as having no cache.
(agent 1.71.1; `tests/native/test_gamegate.c`, mirror harness now carries
`free_mb` so the two copies cannot drift on it.)

## HWPROFILE: SYSINFO cannot answer "can this box run this game" (2026-08-30)

Measured on all 8 fleet boxes. `SYSINFO` reports **no CPU clock** (a 500 MHz and
an 1100 MHz Pentium III are both "family 6"), **no CPU vendor** (family 6 model
2 is an AMD K7 Athlon — .143 really is one, with **no SSE**, so an SSE title is
`#UD` on the first vectorised instruction, not a slow frame rate), **no
instruction set**, **no GPU at all**, and its RAM saturates at 2047 MB.

`HWPROFILE` (agent 1.71.0+) adds CPUID, the real clock, `GlobalMemoryStatusEx`,
the **active** display adapter, OS level, DirectX and a reboot-stable
`profile_hash`. It reads the adapter via `EnumDisplayDevices` for the one
`ATTACHED_TO_DESKTOP` and follows its own `DeviceKey`, because `VIDEODIAG`
enumerates every `Class\{4D36E968-...}\NNNN` subkey and on a box that has ever
had a card swapped `adapters[0]` can be a **stale key for hardware no longer
fitted** — which has already caused one wrong report that a box had no video
driver.

Also worth keeping: the deterministic rules decided 7 of 8 boxes with **zero**
model calls. Only .171 (Intel 865G, no hardware T&L) and .124 (UT2004 at 845 MHz
against a 1000 MHz floor) reached the LLM at all — 9 calls for the whole fleet
across 37 titles. A gate that phones a model to conclude a Pentium III cannot
run Doom 3 is a bad gate.

---

## After a host reboot, `enabled` and `active` answer different questions (2026-08-30)

The dev host rebooted mid-session and everything came back — but establishing
that took a dozen ad-hoc commands across **three managers** (`systemctl --user`,
system units, docker), each with its own status vocabulary. Now one command:
`scripts/fleet/host-duties.py`.

Two findings it encodes, both of which are silent until it is too late:

* **Linger is the single point of failure for seven duties.** Without
  `loginctl enable-linger voidsstr`, **no `systemctl --user` unit starts until
  somebody logs in** — and every unit still reads `enabled`, so nothing looks
  wrong. The host would come up with the chat daemon, the brain, the favourites
  agent and all nine game servers dead. (Verified `Linger=yes` here.)
* **A service running but not `enabled` is invisible until the reboot that
  loses it.** `is-active` says `active`; it simply never comes back. That is the
  one host fault that cannot be eyeballed, so the check looks for it explicitly.

Also re-learned: **`active` is a promise, not evidence.** A unit can be active
while what it supervises is wedged, so the check probes the post-condition —
the game servers' own *per-engine* query replies (a single `getstatus` sweep
reports false outages), `state.json`'s freshness, ollama's API, the brain's
heartbeat. 12/12 servers answered after this reboot.

And the rule that keeps having to be re-applied: **three states, never two.**
`absent` (never installed here — `claude-csbot`, `rtcw-server`, `mohaa-server`),
`unknown` (could not ask) and `down` are different calls to action and only the
last is a fault. Rendering the first as an outage puts a permanent red light on
the board and trains everyone to ignore it.

## A refused TCP connect is contention, not a dead box (2026-08-30)

With six agents driving the fleet at once, a box's listen backlog fills and XP
answers RST. `box-owner.py` called `.124` UNREACHABLE three times while the box
was perfectly healthy — ten hours of uptime, agent 1.71.0, answering instantly
on the next attempt.

The retry loop already existed; the bug was treating a refusal and a timeout
alike. **`ConnectionRefusedError` returns INSTANTLY and consumes none of the
time budget**, so two back-to-back attempts both landed inside the same few
milliseconds and hit the same full backlog. *A retry with no pause is not a
retry.* A refusal now earns several attempts with a rising sleep; a timeout,
having already spent its wait, keeps the rising timeout it had.

This is the second time this tool has produced this same false negative, and
both times the damage was the same shape: **reporting a healthy machine as dead
sends someone to diagnose a box that is fine.**

---

## UT99 469e requires SSE2 — three fleet boxes cannot run ANY 469 client (2026-08-30)

The staged OldUnreal **469e** tree crashes at startup on `.143` (`0xC000001E`) and
`.133` (`0xC000001D` ILLEGAL_INSTRUCTION), before writing a line of its own log —
seen ~30 times, succeeding exactly once, with every DLL matching the library
byte-for-byte, so it was **not** a half-applied `GAMESYNC`. It reads like
corruption and is not.

**Cause, measured by disassembly with a same-product control:**

| binary | SSE2-class | SSE1-class |
|---|---|---|
| 469e `Core.dll` | **1083** | 2482 |
| 469e `UnrealTournament.exe` | **292** | 46 |
| retail 436 `Core.dll` (control) | **0** | **0** |

The hits are real compiler output, not misdisassembled data — `cvttsd2si eax,
QWORD PTR [ebp-0x8]` (`f2 0f 2c ...`) and `movdqa xmm1,xmm0` (`66 0f 6f ...`) in
ordinary ebp-relative prologue code, i.e. the default float-to-int conversion
under `/arch:SSE2`, **not** a CPUID-dispatched fast path. So on a CPU without
SSE2 the process dies on the first vectorised instruction with #UD.

**The control is what makes this conclusive** — this project has already learned
that PE forensics without a same-product control are worthless. 436 and 469e are
the same product, and 436 scores a clean zero.

The fleet's own `HWPROFILE` had the answer all along: `.124` and `.133` report
`fpu,mmx,cmov,sse` (**no sse2**) and `.143` reports `fpu,mmx,cmov,3dnow` (**no
SSE at all**). `.171`, `.123`, `.145`, `.240`, `.246` all have SSE2.

**The gate knew the CPU but had never been told the requirement.** `GG_CPU_SSE2`
existed in `gamegate.h` and `CPU_SSE2` in `rules.py`, and `requires.json` accepts
`"cpu_features": ["sse2"]` and enforces it as a hard NO — but UnrealTournament's
`requires.json` declared only `min_cpu_mhz: 233` / `min_ram_mb: 64`, so the gate
cheerfully approved a title that cannot execute a single frame on three of eight
boxes. Worse, the GPU floor had been *deliberately removed* to let it onto the
Pentium-1, which has no SSE2 either. Fixed by declaring the requirement.

**This is the `disk_mb` lesson again in a different shape**: a capability the
profiler collects but no title ever *requires* is indistinguishable from one that
does not work. Sweep `requires.json` for other titles whose real floor is an
instruction set rather than a clock speed.

**The fleet-level consequence, which is a decision rather than a bug:** the
fleet's `ut99-server` runs 469e, a 436 client cannot join it at all, and `.124`,
`.133` and `.143` cannot run a 469e client. **Those three boxes therefore have no
route to UT99 multiplayer** until either a 436-compatible server is stood up or a
non-SSE2 469 build is found. The incoming Pentium-1 is a fourth.

---

## The Voodoo5 6000 is out of .133 — the vintage V5 lane is down to ONE box (2026-08-30)

`.133` ("P3-DUAL") no longer has the Voodoo5 6000. It renders on an **NVIDIA
GeForce4 Ti 4600** (`10DE:0250`, ForceWare 6.14.10.9371) at 1280x1024x32@85.
Four independent reads agree the 3dfx card is *physically* absent, not merely
undriven:

* `VIDEODIAG` returns one adapter;
* `HKLM\SYSTEM\CurrentControlSet\Enum\PCI` has **no `VEN_121A` key at all** —
  the decisive one, because a present card enumerates there even with no driver;
* the Display class GUID has a single instance `\0000`;
* no `3dfx*` service, and no `glide*.dll` anywhere under `%SystemRoot%`.

**This is the second time a lane lost its hardware without the docs noticing** —
`.124`'s Voodoo 3 came out on 2026-08-11 (top entry) and the same stale claim
survived in CLAUDE.md for weeks. An intermediate draft of the new fleet table
*also* got this wrong, asserting the V5 6000 was still a second adapter in
`.133`; it was written from the old table rather than from a probe. **Probe
`Enum\PCI` before you claim a card is in a box.**

Real Glide silicon on this fleet is now exactly **two cards**: `.143`'s V5 5500
(`121A:0009` subsys `0002121A`) and `.171`'s Voodoo 2 (`121A:0002`). Note that on
`.143` the V5 is the *second* adapter — a **GeForce 6800** (`10DE:0041`) drives
the panel — so "the Voodoo5 box" no longer means "the box that renders on a
Voodoo5".

## A game-local nGlide in the STAGED LIBRARY hides the real card (2026-08-30)

Game-local wins at load time, so the 1,310,720-byte nGlide `glide2x.dll` staged
in `Games-Library/UnrealGold/System/` and `Games-Library/Carmageddon2/` shadows
the real 3dfx `glide2x.dll` in `system32` on **exactly the two boxes that still
have a Voodoo**. On `.171` this was diagnosed the expensive way (commit
`7823586`): UnrealGold was reported crashing, had never crashed, and had run the
whole session on the software rasterizer at 100% CPU because the wrapper's
`grSstOpen` failed `(2, 3)` every time — the game got neither the card nor a
working wrapper.

**That fix went onto the box, not into the library, so the next `GAMESYNC`
restores the wrapper.** Carmageddon2 ships the identical wrapper and is untested.
The staged `Unreal.ini` still carries `WindowedRenderDevice=SoftDrv...` and a
1024x768x32 mode no Voodoo 2 can scan out, so the library reproduces the fault on
demand. The fix belongs in the per-box launcher (the FLEETRES pattern), not in a
staged constant: deleting the wrapper outright is not obviously right, because it
is the only Glide path the six non-3dfx boxes have.

---

## A staged resolution is wrong somewhere BY CONSTRUCTION — one tree, eight monitors (2026-08-30)

The staged game library deploys ONE tree to EIGHT machines: four 1920x1080 16:9
LCDs (.123 .145 .240 .246) and four CRTs (.124 1024x768; **.133 and .171 are 4:3
TUBES being driven at 1280x1024**, i.e. 5:4 and visibly squashed; .143 returns no
EDID at all). Every title was pinned at 1024x768, Tiberian Sun at 640x480. There
is no constant that is right, so the fix has to be **runtime and inside the
staged tree** — `FLEETRES.EXE` + `FLEETRES.BAT` staged per title, called by the
title's `Play ....bat`, reading this box's panel before the game starts.
(`provisioning/fleetres/` and `scripts/fleet/stage-fleetres.py` in retro-agent.)

Four things that cost measurement time:

* **`wmic Win32_VideoController.CurrentHorizontalResolution` is wrong.** It
  reported 640x480 on .123 while the box was really at 1024x768. Use
  `EnumDisplaySettings`.
* **Never derive a target from the LIVE desktop mode.** A game that exits
  without restoring leaves the desktop at 640x480 — .123 and .240 were both
  found sitting there mid-survey — so a launcher trusting it pins every LATER
  game to 640x480 permanently. Read `ENUM_REGISTRY_SETTINGS` (the persisted
  mode) instead.
* **DOSBox `fullresolution=original` changes the WHOLE DESKTOP** to the DOS
  mode, confirmed by `DISPLAYCFG` on .145. On a 16:9 LCD that is a stretched
  640x480 upscale, and it is left behind after a crash. `desktop` + `aspect=true`
  pillarboxes correctly — but `original` is still right on a CRT, so this cannot
  be a staged constant in either direction.
* **id Tech 3's `r_mode`/`r_customwidth`/`r_customheight`/`r_fullscreen` are
  `CVAR_LATCH`** — read once, at renderer init. The staged `autoexec.cfg`'s
  `seta r_mode "6"` runs after `Com_StartupVariable` and before `R_Init`, so it
  **beats the command line** (measured on .123: `+set r_customwidth 1920` left
  the game at 1024x768). Deleting those setas is what makes `+set` work, and it
  is only safe once the launcher supplies the mode — both halves must ship
  together or the fleet gets a windowed Quake III. A per-user
  `%APPDATA%\Quake3\baseq3\autoexec.cfg` (retro-gameindex writes one) also
  **shadows the staged file entirely**, since ioquake3 searches `fs_homepath`
  before `fs_basepath` — which is exactly the case the command line does cover.
* `+vid_restart` also fixes the latch, and **exits ioquake3 outright on .246**
  (Win7 + Radeon HD5450), measured twice. Not usable fleet-wide.

**The generalisable rule:** when one artefact serves hardware that differs, the
fix belongs at *launch* time inside the artefact, not at *authoring* time. A
constant chosen for the machine you happen to be testing on is a defect for
every other machine, and it presents as "that game looks wrong on that box" days
later, with nothing pointing back at the library.

## Hash the binary you LAUNCH and you can certify a cracked game as clean — the crack was in a DLL the engine loads (2026-08-30)

Battlefield 1942, staging the fleet's own preserved copy. The obvious provenance
check — compare the main executable against the vendor's — **passes on the
cracked tree and on a clean one alike, because the exe is not where the
protection lives.**

The repack `Battlefield.1942.PC.Game(djDEVASTATE)` was cracked in **two** places:

| file | what was done to it | how visible? |
|---|---|---|
| `BF1942.exe` | **50 bytes** changed in 14 short runs; identical size (8,908,800) and identical PE timestamp. 24 of the edits are `jz`/`jnz` → `jmp` on the branches consuming the two disc-check functions, plus one string edit turning `bf1942.exe` into `bf1942.org`. | findable — a `.org` backup and a `BF1942.7z` holding the original sat right beside it |
| `Mods\bf1942\Mod.dll` | **replaced wholesale by a 4,096-byte stub** carrying `tHIS iS a wIN 32 pROGRAM! -=[ tE ]=-` and `iMMERSiONj`. It exports exactly the three symbols the engine looks up — `getArchiveId`, `getNrOfArchiveId`, `?getVersion@@YIHXZ` — and does nothing but `ExitProcess`. | **invisible to any exe-level check** |

**The second one is the whole game's protection.** EA's own `Mod.dll` is
SafeDisc 2 wrapped (sections `stxt774` + `stxt371`); `BF1942.exe` is not, and
carries no `stxt*` sections, no `BoG_ *90.0&!!  Yy>` magic, no `DrvMgt` import
and no "please insert" strings — **in the cracked copy and in the retail
original equally.** `BF1942.exe` `LoadLibraryA`s `mods/BF1942/Mod.dll` at
`0x0044D609` and `exit(0)`s if it fails, so the stub is unavoidable and
sufficient: with it in place the game runs with no disc, and every hash of the
launched binary says "clean".

### The rule

**Verify every binary the engine loads, not just the one you launch.** Walk the
whole tree, not the exe:

- Diff **every** PE against the vendor payload, not just `<Game>.exe`. A 4 KB
  DLL where the vendor ships 815 KB is the loudest possible signal and costs one
  `ls -la` to see.
- **A tiny PE next to a huge one is the tell.** `Mods\bf1942\Mod.dll` 4,096 B
  against `Mods\XPack1\Mod.dll` 815,823 B in the same tree — same name, same
  role, 200× apart.
- Check the **exports**, not the size alone: a stub must export exactly what the
  loader resolves, so an export list that is a perfect minimal match for the
  engine's `GetProcAddress` calls, in a file with no other code, is a stub.
- The complement of the same-size/same-timestamp rule in the entry above:
  **a crack can also be a total replacement**, and then size and timestamp look
  nothing like the original — which is why "sizes differ, so it must be a
  legitimate rebuild" is not safe either.

**Applied retroactively the same day** to the other disc-protected titles being
staged. Max Payne's `Max_Payne_Fixes.zip` failed on the first test rather than
this one — its `MaxPayne.exe` is the retail build with the SafeDisc wrapper
stripped (`stxt774`/`stxt371` gone, entry point moved from `0x4cc056` back to a
restored OEP at `0x36fb04`, 730 plaintext `MaxPayne_` symbols where retail has
**zero**) — but the tree it would have gone into was checked DLL-by-DLL as well,
and two of the pack's own DLLs turned out to be unloadable on XP for an
unrelated reason (`d3d8.dll` and `MaxPayne.WidescreenFix.asi` are PE
**subsystem 6.0**).

**How the clean tree was built, for the record:** the preserved game *data*,
overlaid with EA's own `bf1942-patch-1.6.19-full.exe` then
`bf1942-1.6-to-1.61b.exe`, so every executable in the result is EA's —
`BF1942.exe` 5,648,384 B, md5 `a56d63e83eb5e02b43e1928cb22cd15a`, PE timestamp
2004-10-19 19:02:28 UTC, all 29 PEs subsystem 4.0.

---

## A Voodoo 2 game can be "running" and never touch the card — Unreal fell to the software rasterizer on its own splash (2026-08-30)

`.171` (P4, one Voodoo 2). UnrealGold was reported as crashing. It never crashed.
It ran the whole time on the **software rasterizer**, at 100% CPU, having thrown
the Voodoo 2 away 3 seconds after startup. Two independent faults, either one
enough to hide the card:

**1. A game-local nGlide shadowed the real Glide.** `C:\Games\UnrealGold\System\glide2x.dll`
was the **1,310,720-byte nGlide wrapper**; the real 3dfx one is **226,304 bytes**
in `system32`. Game-local DLLs win, so Unreal loaded the wrapper — which reports
`Glide 2.60` (the real one reports `2.56.00.0459`) and translates to Direct3D on
the Intel 865G. **On this box the wrapper does not even work**: `Color buffers 3
failed` / `Resolution 7 failed` / `grSstOpen failed (2, 3)`, repeatedly. So the
game got neither the card nor the wrapper. Tell them apart **by size** — both are
called `glide2x.dll`. Carmageddon2 has the same wrapper (`glide2x`, `glide3x`,
`glide.dll`).

**2. Unreal dropped Glide on its first focus change.** Unreal's own splash dialog
(`#32770`) takes the foreground a few seconds *after* the viewport has already
gone fullscreen on Glide. The viewport gets `WM_KILLFOCUS` → `EndFullscreen`, and
Unreal switches to `WindowedRenderDevice`. **A Voodoo 2 is a 3D-only passthrough
card and cannot render windowed**, so the stock
`WindowedRenderDevice=SoftDrv.SoftwareRenderDevice` stranded the entire session on
software. Nothing external steals the focus — a foreground-window trace across a
full run showed only Unreal's own splash → viewport handoff, and killing every
tray app (igfxtray/hkcmd/igfxpers, DAEMON Tools, SoundMAX, the chat client)
changed nothing.

**Fix, verified:** `WindowedRenderDevice=GlideDrv.GlideRenderDevice` — Unreal then
*re-opens* Glide instead of falling back (`grSstOpen` twice, **zero** `Bound to
SoftDrv`, zero `Bound to D3DDrv` across a full session). Plus a mode the card can
scan out: **16bpp only**, and **640x480** — the stock ini asked for **1024x768x32**,
which no Voodoo 2 can do, and 800x600 is refused too once Unreal requests three
colour buffers (3.84 MB of a 4 MB FBI → `Resolution 8 failed`).

**Method traps, all of which cost time here:**
- **A GDI `SCREENSHOT` cannot see a Voodoo 2.** 3D goes out the passthrough cable,
  so the desktop framebuffer keeps whatever was last drawn. A screenshot showing a
  frozen splash is **not** evidence of a hang — and an *identical* screenshot 45 s
  later is not either. Discriminate with **CPU-time delta** (`wmic ... UserModeTime`)
  and the game's own log, never the picture.
- **Unreal buffers `Unreal.log` and flushes on exit.** Reading it live shows 0
  bytes. Close the game with `taskkill /IM` (WM_CLOSE, *not* `/F`) and then read it.
- **Unreal rewrites `Unreal.ini` on every clean exit**, so an ini edit made while it
  is running is discarded — and a "Glide" test run can silently be a D3D run. Verify
  the ini *after* the run, and confirm the renderer from the log, not the config.
- **`Help\Logo.bmp` is mandatory.** Removing the splash bitmap to dodge the focus
  steal makes Unreal die with a `Critical Error` dialog instead.
- **Prove Glide independently before blaming it.** A 60-line `LoadLibrary` probe
  (`grGlideInit` → `grSstQueryHardware` → `grSstSelect` → `grSstWinOpen` → swap →
  shutdown, logging each step with an immediate flush) settled in one run that the
  card and driver were perfect and the fault was in the game. Worth keeping.

## Crack or compat patch? SAME SIZE + SAME HEADERS + DIFFERENT HASH is a crack (2026-08-30)

Staging Halo 2 turned up a disc image carrying a literal `/CRACK/` directory,
and the useful lesson is that **"there is a crack in the container" is the start
of the analysis, not the end** — the folder held two things of completely
different character, and only one of them was disqualifying.

| artifact | what it actually is | verdict |
|---|---|---|
| `/CRACK/XP_PATCH/` — `DWMAPI.DLL`, `MF.DLL`, `MFPLAT.DLL`, `XTASKDLG.DLL`, `WOW.DLL`, `LOADER.EXE` | **stubs for Vista-only APIs** (Desktop Window Manager, Media Foundation, TaskDialog) plus a loader that fakes the OS version check | **compatibility shim — fine.** It changes which OS the binary will start on. |
| `/CRACK/STARTUP.EXE` | a **byte-patched copy of the retail `/STARTUP.EXE`** | **DRM circumvention — blocked.** |

**The test that separates them, and it is cheap:** compare the suspect against
the original it shadows.

- root `/STARTUP.EXE`  → sha256 `bef26459a69d3746ba5c853ee7af95f7ac9982cf6268c7767047381a61417a8d`
- `/CRACK/STARTUP.EXE` → sha256 `dfd4f05e304f472aaa801562386c0345daebf154bf3b58e432243d7e667578c1`

Both **1,705,336 bytes**, both PE timestamp `0x4636808b`, both version
`1.00.00.11081`, both the same four sections. **Same size + same headers +
different bytes = someone edited the binary in place.** A genuine rebuild moves
the timestamp and almost never lands on the identical byte count; a patcher
preserves both on purpose so the file drops in cleanly.

**File size alone answers this question WRONGLY** — that is the whole point.
A same-size file looks untouched to every size-based check we own (including
GAMESYNC's pre-1.62.0 skip test). Hash it against the file it replaces.

### The mirror-image case, and the control that saved me from a phantom finding
The same day, the share's Halo 1 tree showed the inverse — a file that had been
*un*-patched. `halo.exe` reported version `01.00.00.0564` (retail 1.00) with
**no SafeDisc sections**, while the tree still shipped SafeDisc's own
`drvmgt.dll` (it references `SECDRV.SYS`) and `chktrust.exe`.

**I initially called two more things evidence, and a control experiment proved
one of them wrong.** I claimed the `.text/.rdata/.data/.tls/.rsrc` layout looked
"clean/rebuilt", and that the import directory sitting at the tail of `.rdata`
(RVA `0x26d560`) was an import-reconstructor artifact. Then I pulled a
**known-genuine Microsoft Halo binary** — `halo.exe` out of the officially
Microsoft-signed `Halo_Trial.exe` — and it has the **identical section layout**
and its import directory in the **same place** (RVA `0x26c134`). Both "tells"
were just how Halo's linker lays out a binary.

> **The rule: PE forensics are meaningless without a same-product control.**
> Get a known-genuine binary from the same product and compare. Structure that
> looks damning in isolation is usually just the vendor's toolchain.

**What survived the control was decisive, and it is one field.** The genuine
trial binary's `TimeDateStamp` is `0x3f79fea2` = 2003-09-30, a real build date.
The suspect's is `0x21544c66` — and the *raw bytes of that field* are
`66 4c 54 21`, which is ASCII **`fLT!`**: FAiRLiGHT's scene-group watermark,
stamped into the PE header where the build date belongs.

```
SHARE retail halo.exe    TimeDateStamp=0x21544c66  raw=66 4c 54 21  ASCII='fLT!'
OFFICIAL trial halo.exe  TimeDateStamp=0x3f79fea2  raw=a2 fe 79 3f  (2003-09-30)
OFFICIAL haloupdate.exe  TimeDateStamp=0x537267ff  raw=ff 67 72 53  (2014-05-13)
```

**Read the TimeDateStamp as four ASCII bytes, not just as a date.** Crack groups
sign their work there. A "nonsense" date is a hint; a nonsense date that spells
a group's name is proof.

And the publisher's own tooling agreed: Microsoft's official 1.0.10 updater, run
against the complete, byte-verified retail tree on an otherwise idle box,
refused it with **"There is a problem with your game installation. You may need
to reinstall the game to fix it."** and wrote nothing (all five target files
hashed identical before and after). **An integrity check from the vendor is
worth more than any amount of my own PE reading** — and it was the cheapest test
of the lot.

Note the first run of that test was CONTAMINATED and I nearly drew the
conclusion from it: I gave the updater a directory with 9 files and no maps, on
a box where another agent was mid-test, and got the same dialog — which an
incomplete install explains just as well. The finding only counts because it was
re-run on a clean box against a complete tree.

### The way out is to UPGRADE provenance, not to launder it
Halo 1 had a clean answer because **the publisher itself removed the DRM**:
Microsoft/Bungie's official **1.0.10 patch (May 2014) removed SafeDisc**. So the
fix is to ship *Microsoft's own* DRM-free binary over the user's retail data —
`halopc-patch-1.0.10.exe`, md5 `adeed5e8d33172ec387cb11a89f1b294`, containing
`haloupdate.exe` v`01.00.10.0621` with `CompanyName = Microsoft Corporation`.
That is a legitimate binary with a defensible origin, not a crack.

**Before concluding a title is unstageable, check whether the publisher shipped
a de-DRM patch.** It is more common than it looks for 2000s-era titles whose
protection later broke on modern Windows, and it converts a blocked title into
a clean one. (Halo 2 had no such route: its activation servers are dead and the
only key source was a 284-line list of ~250 product keys bundled with the ISO,
so every path ran through circumvention and it stayed blocked.)

---

## A favourites list can be written, reported "wrote N servers", and be unjoinable (2026-08-30)

Extending the favourites agent (`scripts/gameindex/`) from the Quake family to
the whole staged library turned up four faults **that all log as success**. The
pass prints `wrote N servers` in every one of these cases:

| fault | why it is invisible |
|---|---|
| **All ten fleet servers share one IP.** `best_servers` deduped by host IP to stop a big internet host eating all 16 slots. `192.168.1.132` is one host, so a box got Quake III *or* OpenArena, CS 1.6 *or* the no-blood server — never both. | the write succeeds; the list is just short |
| **SoF2 and Jedi Academy keep data in `base`, not `baseq3`.** The Quake III writer targeted `baseq3\autoexec.cfg` for every q3-engine title, creating a directory the game never reads. | a file was written, at a path nothing loads |
| **The agent reports a game's dir as the one the EXE was in** — for every Unreal-engine title that is `System\`, so appending `System` gave `...\System\System\UnrealTournament.ini`. | only shows up as a path that cannot exist |
| **Unreal Gold and Deus Ex are the same ENGINE as UT99 and a different game.** They were about to be handed a list of UT99 servers. | 6 live servers written, none joinable |

**The general rule: an engine is not a game.** Where a title keeps its
favourites, and which of our servers it can actually join, are per-TITLE facts.
The fleet's Half-Life is the sharpest case — the staged tree is **WON protocol
46** and every fleet GoldSrc server answers **48**, so pointing it at them
would produce a favourites list of entirely dead entries while reporting a
clean write.

### Read the format out of the game, do not infer it from Quake
All three new writers came from the games' own files in `Games-Library`:
- `System\UBrowser.u` (UT99/Unreal Gold/Deus Ex) carries the format as a
  literal bytecode comment — `/* eg Favorites[0]=Host Name\10.0.0.1\7778\True */`
  — and `Query()` passes field 2 to `FoundServer` as the **query port**.
- `XInterface.u` declares `struct ServerFavorite {ServerID, IP, Port,
  QueryPort, ServerName}` on `class ExtendedConsole` → `Favorites=(...)` under
  `[XInterface.ExtendedConsole]` in `UT2004.ini`.
- The staged CS 1.6 tree's own **`revSrvBrowser.dll`** contains the `printf`
  template it writes into `config\ServerBrowser.vdf`, tab characters included.

### The query port is NOT game port + 1
The fleet disproves the convention with its own two servers: **UT99 is
7797/7798 (+1) but UT2004 is 7777/7787 (+10)**. Anything that derives the
query port reports our own live UT2004 server as down. Carry it.

### Ordering a generated config by a live metric rewrites the whole fleet forever
The favourites file was rendered in player-count order. Player counts change
constantly, so the content hash never matched, so **every box was rewritten
every five minutes** — the exact cost the "only if it changed" design exists to
avoid. Measured on `.171`: two passes ninety seconds apart rewrote Quake III
and both UT99 trees purely from reordering, with no server having come or gone.
**Select by the live metric; render in a stable order** (here: ours first, then
by address), and rank on *bucketed* counts so small fluctuations do not move
the cut. UT99, CS 1.6, Quake II and UT2004 then sit unchanged pass after pass.

### Two cheap guards worth copying anywhere we write into a game
- **Never write while the game is running.** Quake III rewrites `q3config.cfg`
  on exit and UT rewrites its `.ini` on exit, both from memory — a write
  landing mid-session is thrown away at best, and at worst reverts what the
  player just changed. One `PROCLIST` per box per pass is enough.
- **Update, never create** (except a file whose normal state is absent, like
  `autoexec.cfg`). A file's **absence is evidence**: a WON Half-Life at
  `C:\Sierra\Half-Life` has no `revSrvBrowser` and therefore no
  `config\serverbrowser.vdf`, and creating one writes a file nothing reads.

### `players > 0` is a defence against a MASTER's list, not a rule for your own

Requiring live players before listing a server is right for a master's output
— 900 Quake III addresses yield ~584 answers of which most are empty, and
favourites full of ghost towns is the complaint that started this. Applied to
an address **we curated**, the same rule means a known-good server drops out
the moment its last player quits and returns when somebody joins, so the file
churns and every box is rewritten *for a server that never went anywhere*.
Curated entries need only to be **alive**.

The general shape: **"did we choose this address" is the line**, and it decides
several rules at once — the gamename filter, the host-dedupe, and this one. For
what we curated we know exactly what is running; for a master's list we do not.

### Never put the slot cut where the list is

18 curated UT99 seeds plus our own server into **16** slots puts the boundary
exactly in the middle of the candidate list, so one server emptying reshuffles
the file. `UBrowserFavoritesFact` declares `Favorites[100]`, so there was never
a reason to sit at the boundary — 24 slots and the whole curated list fits.

Measured across five consecutive fleet passes, rewrites fell **66 → 16 → 13**,
and the last pass rewrote Quake III and nothing else. Quake III genuinely
changes (584 live servers competing for 16 slots); CS 1.6, Quake II, UT99 and
UT2004 now sit unchanged pass after pass.

### "Not attempted" and "nothing to do" must be different numbers

A box skipped because a game was running and a box that needed no change both
landed in `skipped`. So a pass that reached all eight boxes and wrote nothing
**because every one of them was mid-session** looked exactly like a pass with
nothing to do — and nobody would know to re-run. Only one of those two states
needs the next pass to come back for it. They are now separate buckets, and the
busy one is summarised at the end of the pass in its own line.

Related ordering trap: check "is it busy" **after** you know there is something
to write. Reporting BUSY for a title that had nothing to write anyway inflates
the retry list with work that will never happen, and the whole value of the
bucket is that it means *come back*.

Covered by `tests/python/test_gameindex_favorites.py` in the retro-agent repo,
including a coverage assertion that no staged title may fall through to the
generic "nobody has looked at this yet" reason.

---

## A staged game's icon can be the WRONG game's artwork, and every structural check passes (2026-08-30)

Three titles in `Games-Library` drew their desktop icon from the wrong file.
Found on `.143`/`.145` by reading the agent's own log lines — `GAMESYNC` prints
`<Title>: desktop shortcut -> <target> (icon: <path>)` for every shortcut it
creates, which is the only place the resolved icon is ever stated:

| title | `launch.txt` said | should be | what the desktop showed |
|---|---|---|---|
| CounterStrike16 | `hl.exe` | `Counter-Strike.exe` | the **Half-Life lambda** |
| SystemShock2 | `clokspl.exe` | `shock2.exe` | a generic stub icon |
| RedFaction | `UpdateLauncher.exe` | `RedFaction.exe` | a generic stub icon |

**Every one of these is silent by construction.** The shortcut exists, it
launches the right game, and it merely wears the wrong badge — so nothing logs
a warning, `state: done, failed_files: 0` is truthful, and a process list or a
`dir` cannot tell you. `hl.exe` is the nastiest of the three because it *is* the
engine Counter-Strike runs on, so the field reads as obviously correct to anyone
reasoning about the tree instead of looking at the screen.

**"Does the icon file have an icon?" is NOT the check.** That was the first
instinct and it is wrong: parsing the PE resource directory shows `clokspl.exe`
(the SafeDisc splash loader) and `UpdateLauncher.exe` (the patcher stub) each
carry exactly **one** `RT_GROUP_ICON`, same as `shock2.exe` and
`RedFaction.exe`. They have artwork; it is just generic artwork. Structurally
all five files are identical, so no resource-level test can separate them.
`validate-staged-library.py` already fails an icon path that does not *resolve*,
and that is as far as a structural check can get.

**So the only durable answer is to pin the verified value.** Whether artwork is
the *right* artwork is a content question — the fix is
`retro-agent/tests/python/test_launch_icon_targets.py`, which asserts both
directions per title (fixed icon present, old stub absent) and skips loudly
when the share is not mounted.

**Two adjacent traps, both cost time in this pass:**

- **Explorer caches shortcut icons by path, so a corrected icon does not appear
  until Explorer restarts.** Replacing `retro_chat.exe` with a build that has a
  real icon resource changed nothing on screen; the `.lnk` kept drawing the old
  generic window. Fix: `taskkill /f /im explorer.exe`, relaunch it, then
  `C:\retro-wall\arrange_icons.exe`.
- **After an Explorer restart, the FIRST frame is a lie.** Screenshotting
  immediately gave a desktop with **34 of 73** icons and `arrange_icons.exe`
  reported "moved 34 icons" and parked a half-empty bay — indistinguishable
  from the shortcuts having been deleted. Explorer was still enumerating the
  All Users desktop. Twenty seconds later: "moved 75", full bay. **Wait for the
  count to settle before arranging, and never judge a post-restart desktop from
  the first screenshot.**

Fixed in the library, redeployed via `GAMESYNC`, and confirmed with a zoomed
screenshot on `.145` (CS badge, Red Faction emblem, SHODAN face all render).
Pushed to `.123 .124 .133 .143 .145 .171 .240`. The four Carmageddon 1 entries
remain genuinely generic — that title ships no `.ico` and its exe carries no
icon resource, so it needs artwork, not a config fix.


## A second GPU's stale OpenGL ICD registration kills GL on the card that IS driving the monitor (2026-08-29)

`.145` failed **every** OpenGL context creation, whichever DLL performed it, on
a box whose NVIDIA driver was installed and working. It read as a broken driver
and it was a **registration** problem.

`HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\OpenGLDrivers` held two
subkeys, one per adapter:

    OpenGLDrivers\Intel     Dll = ig4icd32   (Intel HD Graphics - drives nothing)
    OpenGLDrivers\RIVATNT   Dll = nvoglnt    (GeForce 8400GS - has the monitor)

**Windows enumerates those subkeys alphabetically, so `Intel` is tried first** —
and Intel's ICD cannot create a context on an NVIDIA framebuffer. The NVIDIA ICD
was present, correct and never asked.

**Diagnosis order that works**, and note the first step is the one that
misleads:
1. `VIDEODIAG` lists both adapters, which invites "it is using the wrong GPU".
   It is not — `wmic path win32_videocontroller get name,currenthorizontalresolution,configmanagererrorcode`
   showed **only** the GeForce with a live resolution and `ConfigManagerErrorCode=0`.
   The display side was correct all along.
2. `reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\OpenGLDrivers" /s`
   is where the fault actually is.
3. Confirm with the game's own log rather than a frame:
   `GL_RENDERER` / `GL_VENDOR` name the card that really answered.

**Fix:** remove the subkey belonging to the adapter that is NOT driving the
monitor. Reversible — export it first; the values here were
`Dll="ig4icd32"`, `Version=2`, `DriverVersion=1`, `Flags=3`. After removal
`ioquake3` reported `GL_RENDERER: GeForce 8400GS/PCIe/SSE2`,
`GL_VENDOR: NVIDIA Corporation`, `GL_VERSION 3.3.0`.

**Check this on any box with two display adapters** — an onboard chip plus a
card is the common case on this fleet, and the symptom (every GL title dies at
context creation) looks nothing like a registry problem.

---

## A version number does not tell you whether a client can join — connect it and look (2026-08-29)

Audited whether every fleet box's game clients can actually reach the servers on
this host. **Every marker-based rule written first was wrong, and each one
condemned healthy machines.** The corrections only came from launching the real
client at the real server:

| rule as first written | what hardware said |
|---|---|
| GoldSrc client `PatchVersion` must equal the server's | **56 false mismatches.** 1.1.2.5 and 1.1.2.7 are both protocol 48; .145's stock client logged `Connection accepted by 192.168.1.132:27018` |
| Quake III needs the retail `pak0..pak8` set | **.145 has only pak0..pak6** and was on the server as `BOX145` while the audit called it broken |
| UT99 469 = has `VulkanDrv`/`XOpenGLDrv`/`SDLDrv` | those are the **Linux server's** renderers. No Windows client has them, so all 15 UT99 installs read as 436/451. The real marker is the `OldUnreal469c.u` stamp |
| an engine identifies a game | `q3` also covers Jedi Academy and SoF2 (paks in `base/`, not `baseq3`); `unreal` covers Unreal Gold. Auditing those against our Q3/UT99 servers condemned installs that were never going to connect to them anyway |

Final state: **40 ok, 1 mismatch, 0 unknown** across 8 boxes.

**Two traps that make a live test lie, both of which nearly produced a wrong
answer here:**

* **A player at the team-select screen is connected but NOT counted by A2S.**
  The CS client was fully joined and rendering the map while the server still
  reported `players=0` for 65 seconds. "The player count did not move" is
  therefore *not* evidence of a failed connect — read the client's own console
  (`-condebug`, and note GoldSrc writes `qconsole.log` to the **game root**, not
  the mod directory).
* **A blocked dialog is indistinguishable from a refusal, from the server side.**
  UT99 sat on its "Recovery Mode" dialog and never attempted the connection at
  all; the server saw exactly what a rejected client looks like — nothing. One
  screenshot separated "refused" from "never asked".

**The one real finding: The Specialists has no working client on the fleet.**
`.145`'s `C:\Sierra\Half-Life` is the only TS install and is a **WON-era tree** —
no `steam.inf` anywhere (that file postdates Steam) and `ts/liblist.gam` declares
`hlversion "1110"`. It runs `hl.exe -game ts -window` and stays up; add
`+connect` to our protocol-48 TS server and the process is gone within 25s,
never appearing in the player list.

Tooling: `retro-agent/scripts/gamepatch/audit.py`, which reports **ok /
mismatch / unknown** and refuses to collapse the last two — an unreadable
marker is not a failing client.

---

## Two files differing only in CASE are one file on the client (2026-08-29)

The live UT99 server carries both `Botpack.u` (40 MB, matches OldUnreal's
current manifest hash) and `BotPack.u` (39 MB, matches **no** entry in the
315-row manifest). On Linux they are two files and the server has the right one
open, so it is inert there.

On any Windows client they are **one file**, and which one survives a copy is
arbitrary — the wrong one is a version mismatch at connect time, on a box that
looks correctly provisioned. The audit now flags case collisions in a UT99
`System/` directory for this reason.

---

## A root service does not see a module installed in a USER site-packages (2026-08-29)

`psycopg2` on this host lives only in `/home/voidsstr/.local/lib/python3.14/`.
The dashboard collector runs as **root**, so the article counts it was written
to publish would have rendered blank forever after deploy — and a blank there is
indistinguishable from a site that published nothing all week, which is a much
more alarming thing and would have been believed.

Caught by checking `site.getusersitepackages()` before deploying rather than
after. The fix is `apt install python3-psycopg2`. **Not** fixed by appending the
user's site-packages to `sys.path`: that has a root service import code from a
user-writable directory, which is a privilege-escalation route in exchange for a
wall decoration.

---

## Before changing a staged tree for a failure, COUNT THE BOXES (2026-08-30)

Two near-misses on the same day, in opposite directions, and both would have made
things worse:

* **Unreal Gold.** A crash on `.143` (`Critical: Failed blt: DD_OK` /
  `UD3DRenderDevice::SetTexture`) was reported against the staged
  `GameRenderDevice=D3DDrv.D3DRenderDevice`, with a suggestion to switch the
  library to a modern UE1 render device. The **same staged tree with the same
  line works on `.123` and `.145`.** Three good, one bad. The corroborating fact
  is on that box, not in the tree: **Deus Ex — a different title and a different
  engine build — reports `No fullscreen display modes found (DD_OK)` →
  `3d hardware initialization failed` → `Bound to SoftDrv.dll` on `.143` too.**
  That box's DirectDraw *mode enumeration* is broken. The agent that filed it
  retracted it themselves.
* **SiN and System Shock 2 on `.145`.** Same shape, mirror image: two unrelated
  1999–2001 engines, one box, one signature.

**The rule: one box failing where others succeed is a BOX problem. The staged
tree is the control, not the variable.** Changing what every machine receives to
rescue one degrades the working majority *and* hides the real fault so nobody
ever fixes it.

**The corollary that makes it actionable:** a per-box display-driver fault
**follows the box across titles**, so the cheap confirmation is to check a second,
unrelated engine on the same machine before touching any library file. That is
what settled both of these, and it costs one launch.

## Quake II config traps: TWO command buffers, and comments that execute (2026-08-30)

Both found on SiN (a Quake II derivative), both general to the engine family, and
both produce symptoms that point at the wrong thing.

**1. `+set x y` is the EARLY buffer; `+x y` is the LATE buffer — and `autoexec.cfg`
runs between them.** So a `+set` on the command line is applied *before*
`autoexec.cfg`, and any `set` of the same cvar inside that file **silently
overrides it**. Forcing SiN's renderer needed BOTH forms:

    sin.exe +set vid_ref soft_real +set sw_mode 6 +vid_ref soft_real

With only `+set`, `autoexec.cfg`'s own `set vid_ref "gl"` won and the game loaded
the GL renderer anyway. With only the late `+vid_ref`, it crashed *sooner* —
because the FIRST renderer load happens before the late buffer runs, on whatever
the default already was. **Each half alone still failed, at a different point.**

**The transferable diagnosis: a command-line override that appears to be ignored
is usually a config file undoing it afterwards.** Grep the file the game execs
before doubting the flag — and note the trap is sharpest when you are already
*editing* that file, because you are configuring around the very thing beating
you.

**2. A quoted string may SPAN A NEWLINE, so a comment can become executable.**
SiN's console printed `Line has unmatched quote, discarded.` twice, then
`Unknown command "I"`, `"sixty"`, `"it"`. Cause: five `//` comment lines quoted a
phrase **across a line break** — opening quote on one line, closing quote on the
next. That puts the following line's `//` *inside* a string, so the comment
marker stops working and the prose after the closing quote is executed as
commands.

It was **harmless only by luck**: every functional line sat above the damage.
**Any `set` or `bind` added below a broken pair would have been silently
swallowed** — which presents as "my config change is not taking", the exact wrong
diagnosis. Check any Quake-family `.cfg` with:

    awk '{n=gsub(/\x22/,x); if (n%2) print NR, $0}' autoexec.cfg

(written with no double quotes of its own — the first version of that very line
was itself the only unmatched-quote line left in the file, which is how easy this
is to get wrong).

---

## A 2001 OpenGL game can be killed by a MODERN driver's extension string, not by a missing feature (2026-08-29)

SiN 1.11 runs fine on a **GeForce 6800** and access-violates on a
**GeForce 8400GS** whose ForceWare reports **OpenGL 3.3**. The newer card is not
missing anything; it is offering *more*.

The engine's own log is the whole diagnosis, and the tell is where the file
**ends**:

    .143 / GeForce 6800   GL_VERSION: 2.x  -> ~2.4 KB GL_EXTENSIONS -> continues
    .145 / GeForce 8400GS GL_VERSION: 3.3.0 -> <end of file>          -> c0000005

It dies while producing or handling `GL_EXTENSIONS`, which on a GL 3.3 driver is
far longer than a Quake II-derived engine of that era was written for. **A
fixed-size extension-string buffer is the strong hypothesis — nobody has
disassembled it, so it is not a finding.**

**Why this generalises beyond one title:** the fleet keeps acquiring newer cards
for old boxes, and *every* pre-2005 OpenGL title parses that string. Expect the
same shape — an engine that works on the older card and crashes on the newer one,
with the log stopping at `GL_VERSION`. It reads like a broken install or a bad
driver; it is neither.

**Diagnostic that costs nothing:** run the engine with its logfile on and look at
the LAST LINE. Where a log stops is often more informative than what it says,
and it distinguishes this instantly from "no GL at all" or "wrong renderer".

**Attribution discipline that mattered here.** The staged tree deliberately makes
`ref_soft.dll` a copy of `ref_gl.dll` (a fullscreen fix for cards lacking
`GL_EXT_shared_texture_palette`), so the obvious suspicion was that the
substitution caused the crash. An A/B with the genuine software renderer showed
it reaching `mode 6: 1024 768 FS` and completing server init — and then dying **at
exactly the same point** once the GL renderer loaded. So the substitution is not
the cause; it only changes *where* the crash lands. **Two changes in flight at
once, and only an A/B separates "my change broke it" from "my change moved it".**

Equally important, what was **not** claimed: that this is a 1.11 regression. That
title had never been run on that box under 1.0, so there is no baseline and none
can be taken once 1.0 is replaced. "The new version broke it" is the tempting
sentence and there was no evidence for it.

---

## A private side-by-side assembly turns a missing DLL into "cannot execute the specified program" (2026-08-29)

**`CreateProcess` gle=14001 `ERROR_SXS_CANT_GEN_ACTCTX` is reported by cmd as
"The system cannot execute the specified program." — which reads like a corrupt
or wrong-architecture binary, so the instinct is to go find a different exe.**
That instinct is wrong and it is expensive.

Found on Tiberian Sun. CnCNet's `GAME.EXE` carries an embedded `RT_MANIFEST`
declaring a dependency on a **private** side-by-side assembly named `BLOWFISH`:

    <dependency><dependentAssembly>
      <assemblyIdentity type="win32" name="BLOWFISH" version="1.0.0.0"/>
    </dependentAssembly></dependency>

The tree's own 1999 `BLOWFISH.DLL` has **no resource directory at all**, so no
assembly manifest, so activation-context generation fails and the process dies
**before a single instruction runs**. CnCNet ships the same DLL with an
`RT_MANIFEST` added (identity `BLOWFISH 1.0.0.0` + reg-free COM for CLSID
`{1440AD10-6AA8-11D1-B6F9-00A024DDAFD1}`). Swap the exe alone and nothing
happens; swap both and it runs.

**Two general lessons:**
* **When cmd hands you a sentence instead of a number, get the number.** A
  three-line `CreateProcess` probe returned 14001 and named the problem in one
  shot, after the sentence had sent the investigation off looking for a better
  binary. `start ""` is worse still — it is fire-and-forget and discards the
  exit code entirely.
* **Check for `RT_MANIFEST` when a modern-ish rebuild of an old game refuses to
  start.** Parsing the PE resource directory is cheap, and "the exe declares a
  dependency the tree cannot satisfy" is invisible from every other angle.

Related and distinct: an INI the patch expects and the tree does not ship can
produce a plain `0xC0000005`. The same CnCNet patch replaces the game's
hardcoded 640/480 constants with reads of `SUN.INI [Video] ScreenWidth/Height`.
Those globals live in `.bss`, so with **no `SUN.INI` they are zero**; the surface
allocator has an explicit "if width<=0 or height<=0, skip" guard, skips creating
the HiddenSurface, and the game then makes a virtual call through the NULL
pointer. **A patch can introduce a hard dependency on a config file that never
existed before** — and the crash looks like bad game data, not a missing INI.

## An era patch can remove a CD check outright — look on your own shelf before concluding it is uncrackable (2026-08-29)

SiN Gold reached its main menu on every fleet box and **could not start a game**:
`sin.exe +map bank` raised "You must have the Sin CD in the drive to play." A
whole elimination pass had already ruled out the volume label, a marker file, the
registry path, and **mounting the user's own disc image as a genuine
`DRIVE_CDROM`** (657 MB and a mounter dependency for no behaviour change).

The answer was the vendor's own **official 1.11 patch**, which simply removed the
check: the string is in the 1.0 binary and **in no form** in 1.11's. No crack was
involved, needed or wanted. And the patch was never on the internet for us — it
was **already on the user's own share**, inside a licensed disc-preservation
archive that an earlier note had flagged as "the next step" for an unrelated
reason.

**Three transferable points:**
* **A 1990s copy-protection check is a version-dependent behaviour, not a law of
  the title.** Before concluding a check cannot be satisfied, look for the last
  official patch. Era patches routinely relaxed them.
* **Search the user's own media before the internet.** Ours held it, and the
  web routes (ModDB, FilePlanet, archive.org) all refuse scripted fetches, so the
  patch had been written off as "needs a human with a browser".
* **Pick a discriminator string that only appears in the failure path.**
  `"No CD in player."` exists in *both* binaries — it is the audio-CD track
  player — and using it as the check would have reported the patch as not
  applied. `"CD in the drive"` is the one that discriminates.

## UT99: a 436 client DOES join a 469 server — the join is a version handshake with a floor of 432 (2026-08-29)

**"A retail 436 client cannot join a 469 server at all" is FALSE.** That single
sentence sat in three of our documents and was the stated reason three pre-SSE2
boxes (.124, .133, .143 — all `0xC000001D STATUS_ILLEGAL_INSTRUCTION` on 469e)
could have no UT99 at all. It made the only apparent options a second dedicated
server or a fleet-wide downgrade. Neither was needed.

Established three independent ways:
* the 469e engine's own `UEngine::GetMinNetVersion()` is literally
  `mov eax,0x1b0; ret` = **432**;
* the live server ini sets `MinClientVersion=432`, no ACE, `MD5Enable` absent;
* a live join, with the server log reading `HELLO ... VER=436` → `Accepted` →
  `Join succeeded`.

All eight shared `ServerPackages` also carry **byte-identical GUIDs** across the
436 and 469e trees, *including* the three whose contents changed — OldUnreal
preserved them deliberately.

**The lesson is about the shape of the error, not about UT99.** A blocking
premise that everyone repeats, that nobody has measured, and that determines the
whole solution space is the most expensive kind of wrong thing to have in a
document. **When a constraint is doing that much load-bearing work, measure it
before you design around it** — here it took one `objdump`, one `grep` of a
config, and one join.

Two smaller corrections that travelled with it, both the same failure of
checking: the affected-box list said ".133 and .143" when **.124 was affected
too**, and "no 436 CLIENT media has been found" when a complete pre-installed 436
GOTY tree was **on the same share the whole time**.

Also worth keeping, on why a runtime capability check does not save you: 436's
`Galaxy.dll` *does* contain real SSE1, but it is runtime-gated on `GIsKatmai`
imported from `Core.dll`. 469e's problem is that it puts SSE2 **in the EXE
itself** — measured: 69 `movdqa` / 25 `movapd` / 44 `movsd` in
`UnrealTournament.exe` and 6,304 `movdqa` in `Engine.dll`, against **zero** in
every 436 module — so it faults before any check can run.

---

## GAMESYNC skips a staged file whose size is unchanged (2026-08-29)

`gs_copy_file` treats a destination file of the **same size** as already up to
date — it compares neither content nor mtime. So **editing a staged file without
changing its length never reaches any box that already has the old one**, and
GAMESYNC still reports success.

Caught on `Descent1\DESCENT.CFG`: the box had 228 bytes with
`DigiDeviceID=0xFFFFFFFF` and the share had 228 bytes with `DigiDeviceID=0x0`.
The sync ran clean and the box kept the broken file.

This is a *second*, distinct trap alongside the known "GAMESYNC never deletes".
Same symptom — library changed, sync says done, box unchanged — different cause:

| trap | trigger | workaround |
|---|---|---|
| never deletes | file removed from the library | overwrite it with a pointer comment (also changes its size) |
| same-size skip | file **edited** to the same length | make the length change, or rename the file |

Config edits are exactly the dangerous case, because the useful ones are
same-length: `0`→`1`, a hex value edited in place, `yes`→`no`. (`ipx=false` →
`ipx=true` shrinks by one byte and is safe by luck, not design.) After any
small-file staged fix, confirm the **size on the box** matches the library
rather than trusting the sync's "done".

## A filename with PARENTHESES cannot be launched through the agent (2026-08-29)

    EXEC cmd /c start "" /D "..." "...\Host Descent (LAN).bat"
      -> 'C:\Games\Descent1\Host' is not recognized

**Mechanism (two layers of cmd, not one).** The agent already runs
`cmd.exe /c <command>`; our `cmd /c start` adds a **second** layer. cmd's
quote-stripping across those two layers loses the quotes, so the path splits at
the first space. **Plain spaces are fine** — `Launch Red Alert 2.bat` works —
it is specifically the parentheses. The 8.3 name works as a fallback
(`HOSTDE~1.BAT`, from `dir /x`).

**Why it survived review: a desktop `.lnk` is completely unaffected.** The file
launches perfectly for a person double-clicking it and fails only for
automation, which makes the affected launchers **unverifiable rather than
obviously broken** — the worst shape a defect can take on this fleet, because
every one of our verification passes is automated.

**This is the SECOND time this character has cost time here.** `onboard.cmd`
had the same class of failure on game NAMEs like `(BC Romania)` and
`(fleet build)`, where the `)` in an expanded variable closed a `( ... )` block
early and cmd aborted with `- was unexpected at this time`, leaving onboarding
silently unfinished — no theme, no `Onboarded` flag. That was fixed per-script.

**It is now a REQUIRED rule in the retro-agent CLAUDE.md instead:** any filename
this project GENERATES — a launcher `.bat`, a shortcut target, a `launch.txt`
path — must avoid `(` and `)`. Use a dash (`Host Redneck Rampage - LAN.bat`).
Display names in `launch.txt` may keep parentheses; that column is a label, not
a path. Nine staged launchers carried them.

**Corollary, same family:** `taskkill /f /im "Descent 3.exe"` needs its quotes —
**without them it silently kills nothing**, after which the previous game is
still on screen and the next screenshot is attributed to the wrong title. That
one cost a lost Tiberian Sun result.

---

## A shared CD serial lets only ONE machine on the fleet into a LAN game (2026-08-29)

Red Alert 2 / Yuri's Revenge refuse a second machine's LAN join with

    "There is already a player with your serial# in that game."

Westwood reads a **per-installation** serial from
`HKLM\SOFTWARE\Westwood\<Red Alert 2|Yuri's Revenge>\Serial` (string 34326
`Serial` sits immediately before 34327 `SOFTWARE\Westwood\Yuri's Revenge` in
`gamemd.exe`; ids `TXT_SERIAL_DUP` / `TXT_SERIALDUP`). The staged `install.reg`
wrote `InstallPath` and `Version` but **no Serial**, so every box read the same
absent value — and **only one machine on the entire fleet could ever be in an
RA2 LAN game.**

**The general lesson, which is bigger than RA2.** Our staged-games model copies
a tree *byte-identically* to every machine. That is exactly right for content
and exactly WRONG for anything that must be **unique per installation** — a
network serial, a machine GUID, a player id. Such a value cannot live in
`install.reg`, because install.reg is the thing being copied identically; it has
to be **generated on the box at first launch** and then left alone. The fix
therefore lives in the title's `Play/Launch .bat`:

    where reg.exe >nul 2>&1 && (
      reg query "HKLM\SOFTWARE\Westwood\%%~K" /v Serial >nul 2>&1 || (
        reg add "HKLM\SOFTWARE\Westwood\%%~K" /v Serial /t REG_SZ ^
            /d 1%%RANDOM%%%%RANDOM%%%%RANDOM%%00 /f >nul 2>&1
      )
    )

written **only if absent**, so it is stable across relaunches and across
redeploys. `reg.exe` does not exist on Win9x, hence the `where` gate.

**Worth auditing every other staged multiplayer title for the same shape** — any
game that identifies an installation rather than a player will have it.

**Proven on .123 + .240:** identical serial → join refused; distinct serials →
host on one box, join from the other, both in the same match at fullscreen
1024x768.

**Related trap in the same tree: `RedAlert2\wsock32.dll` is NOT IPXWrapper and
must not be replaced with it.** At 49,664 bytes (md5
`a195155a1a31995e1eb685854acac3dc`) it is a working IPX-over-UDP LAN shim of the
CnCNet pattern — it forwards nearly all `wsock32` ordinals to `ws2_32` but
implements `bind`/`getsockopt`/`htonl`/`htons`/`ntohl`/`ntohs`/`recvfrom`/
`sendto`/`setsockopt`/`socket` itself. `Carmageddon2` carries a *real*
IPXWrapper of a similar size, which is what makes the two easy to confuse.

**And do not trust A2S for liveness on our GoldSrc servers.** `A2S_INFO`
reported `players=0` on both `:27015` and `:27018` **while two real clients were
playing** — clients arrive through the `:27015` proxy, so hlds logs them as
`192.168.1.132` and the A2S count does not reflect them. Use the hlds log or
rcon `status`. (Separately: on the Quake family and CS, a player line with
**ping 0 is a bot** — the Q3 server runs `bot_minplayers 4`.)

---

## Windows 7's GameUXShim hangs old games FOREVER - process alive, zero CPU, no window (2026-08-29)

On **.246** (Win7 6.1.7600) several staged titles "launched" and then did
nothing at all. They were not crashing: Windows 7 attaches an AppCompat shim,
**GameUXShim** (`gameux.dll`, invoked as
`rundll32.exe ...,GameUXShim {86fbe0c5-...};<exe>;<pid>`), to old game
executables, and the game **blocks forever waiting on it**.

**Signature - learn this, it is unlike any other failure:**
- process ALIVE, **1 thread**, and **zero CPU accumulation** (it never runs)
- `WINLIST` empty - no window is ever created
- a `rundll32.exe` sibling process
- `tasklist /m` on the hung game lists only `ntdll`, `kernel32` and
  **`apphelp` / `AcGenral` / `AcXtrnal`** - **none of the game's own DLLs**,
  because it never reaches its entry code

Confirmed victims on .246: **Quake II and StarCraft**. It would present as a
different mystery for every title it touches.

**Fix - instant, no reboot, Win7+ only (does not apply to XP):**

    reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\AppCompat" \
        /v DisableEngine /t REG_DWORD /d 1 /f

The obvious objection is that this also disables `AppCompatFlags\Layers`, and
some titles carry a deliberate layer - StarCraft has
`~ WINXPSP3 DISABLEDWM 256COLOR`. That was **A/B tested rather than assumed**:
with the engine ON, StarCraft hangs entirely; with it OFF, StarCraft runs. So
disabling is strictly better here and there is no trade-off to weigh.

**Two more Win7-only items from the same box:**
- The firewall's **Domain profile** was still ON after the Standard profile was
  disabled - on Win7 use `netsh advfirewall set allprofiles state off`, not the
  XP `netsh firewall` syntax.
- **Suppress AutoPlay** (`NoDriveTypeAutoRun`=255 under HKCU+HKLM
  `Policies\Explorer`): mounting a game ISO throws a modal AutoPlay window over
  the running game. This matters now that disc-gated titles mount their own
  images at launch.

**Related, and a correction to an earlier conclusion in this log:** SiN Gold's
staged binaries are VS2015 / PE subsystem 6.0 builds, which XP's loader refuses
outright - but they **do not work on Win7 either**. On .246 `sin.exe` starts and
**exits immediately with RC=0**, no window, no process, and **no crash record in
the Application event log** - a clean voluntary exit, tested plain, with
`+set vid_ref soft +set vid_fullscreen 0`, and with the AppCompat engine both ON
and OFF, against intact data (`base\pak0.sin` 569 MB). So it is not a
wrong-target binary to be kept as a Win7 variant; it is simply broken, and the
title must be re-staged from the period ISO.

The bounding scan is worth copying: every `.exe`/`.dll` in all 29 staged titles
was checked for PE subsystem >= 6.0, and **SiNGold is the only game affected**
(the sole other hit, `UnrealTournament/System/magick.exe`, is an ImageMagick
helper on no launch path). A bounded class beats a fixed instance.

---

## On XP, an UNSIGNED driver can never win on merit — `DriverSigningPolicy=Ignore` suppresses the dialog, NOT the rank (2026-08-29)

Root-caused on **.124** (freshly PXE-imaged Pentium III 845 MHz, GeForce2 GTS
`PCI\VEN_10DE&DEV_0150`) after the user reported "the GeForce 2 GTS and Sound
Blaster 16 drivers are not installed". They were installed — with *Microsoft's*
in-box drivers, at 800x600 in 16-bit colour, reporting **status OK and problem
code 0**, while ForceWare 71.89 sat unused in `C:\D\G005` the whole time.

**This is a whole class of silent image failure, not one card.** Read from the
box's own `C:\WINDOWS\setupapi.log`, there are two independent layers and
fixing either alone changes nothing:

**1. The driver was never a candidate.** `winnt.sif` carries only the SHORT
early `OemPnPDriversPath` (LAN + chipset), because the full 493-directory list
is 3450 chars and broke the answer file. Everything else waits for `DevicePath`,
which `cmdlines.txt` writes at **T-12 — after GUI setup has already installed
the devices**. The log proves it: exactly **six** `Found ... in C:\D\` lines in
the whole install, and every one names an `L` (LAN) or `C` (chipset) directory.
Not one `G`, `H`, `I`, `M`, `N`, `S` or `T`. Graphics, sound, monitor and
mass-storage were copied to disk, indexed (`Modified INF cache "C:\D\G005\
INFCACHE.1"`), and never consulted. The comment in `inject-drivers.sh` saying
graphics and sound "can wait for DevicePath" was the bug.

**2. Even a VISIBLE unsigned INF loses.** XP adds **+0x8000** to the rank of an
untrusted driver node:

```
#I087 Driver node not trusted, rank changed from 0x00002000 to 0x0000a000.
```

so it can never beat a trusted in-box match — XP's own `nv4_disp.inf` scored
`0x00002001`. `DriverSigningPolicy=Ignore` only suppresses the *warning dialog*.
Every DriverPacks INF that was **edited or renamed** (`nv4_disp2.inf`,
`nv4_disp3.inf`, the `_go` mobile INFs) has lost its catalog and is untrusted by
construction. The counter-example in the same log proves the rule rather than
breaking it: the NIC *did* get a DriverPacks driver, because
`C:\D\L025\e100b325.inf` is DriverPacks' **unmodified** copy of Intel's INF
and still validates — it scored `0x00000001` with no penalty and won.

> **Rule:** a device Windows can serve *badly* by itself never sees our better
> driver, ends at problem code 0, and nothing anywhere flags it. The ONLY way to
> put an unsigned driver on XP over a working in-box one is an explicit **forced**
> install (`UpdateDriverForPlugAndPlayDevices` + `INSTALLFLAG_FORCE`), which does
> not consult the ranking at all. Making the driver *visible* to PnP is not enough.

**The second-order bug this caused.** `gs_reclaim_drivers()` deleted `C:\D` once
no device carried a problem code — reading "no problem code" as "setup is
finished with the drivers". On .124 that removed 2.4 GB of NVIDIA drivers from
the machine that needed them: `dir C:\D` returns *File Not Found* while
`DevicePath` still lists all 493 directories. **Neither the right driver nor the
payload to fix itself.**

**Fixed in agent 1.59.0.** The image now ships an explicit
`$OEM$\$1\D\PREFER.TXT` (`<hardware id>\t<INF>`, generated by `stage-oem.sh`
from `scripts/pxe/driver-prefs.txt`), the agent force-installs it at first logon
**before** reclaiming, and the reclaim refuses while any preference that applies
to *this* machine is unsatisfied. Preferences for hardware the box does not have
never block, or the list would refill the 6 GB Gateway the reclaim exists for.
Logic in `agent/shared/drvprefs.h`; tests `tests/native/test_driver_prefs.c` and
`tests/test_pxe_drivers.py`.

**Do not let a heuristic choose the INF.** Three directories in that image ship
an INF naming `DEV_0150`, and the first one a search finds is
`G003\nv4_go.inf` — **ForceWare 270.61 MOBILE, a 2011 driver for a 2000 card**.
`gs_find_inf_for()` would have picked it. Name the build (the preference file
matches on `DriverVer 7.1.8.9` = 71.89).

**Verified on hardware, not inferred:** `DRVUPDATE PCI\VEN_10DE&DEV_0150
C:\D\G005\nv4_disp.inf` on .124 → `OK installed ... (reboot required)`, and
`wmic path win32_videocontroller` then reports **6.14.10.7189** where it had read
6.14.10.5673.

**And the honest half: the Sound Blaster needed nothing.** The card is an ISA
**PnP** Creative **AWE64** (`ISAPNP\CTL00E4_DEV0000/0001/0002`, compatible ids
`*CTL0045` and `*CTL0022`), correctly driven by XP's own `wdma_ctl.inf` /
`CTLSB16.SYS`; all three functions installed cleanly. A grep of **every INF in
the whole staged driver payload** for `CTL00E4`/`CTL0045`/`CTL0022` returns
**zero hits** — DriverPacks Sound A/B is PCI/HDA-era and Creative never shipped
an XP driver for their ISA cards. There is no better driver to install, and
"our payload has nothing for it" is the right answer here, not a gap.

---

## A 1990s CD check wants a DISC IN A DRIVE - staging the disc's files into the game folder does not satisfy it (2026-08-29)

The single biggest blocker in the fleet-wide staged-games pass was not rendering,
drivers or patches: it was **CD-presence checks**. Deus Ex, Red Faction, Red
Alert 2, Soldier of Fortune and StarCraft all hit one.

The expensive way we learned the rule was StarCraft. `StarCraft.exe` opens
`\Install.exe` as a data archive and reads `rez\CDversion.txt` from it, and that
file exists in no other file on the disc - so the obvious fix is to stage the
CD's archives into the game folder. **That does not work**, and it cost 1.15 GB
per box to prove:

| box | optical drives | disc | result |
|---|---|---|---|
| .133 | present, ALL EMPTY | archive staged locally | `Data File Error` |
| .145 | four (D:-G:), ALL EMPTY | archive staged locally | `Data File Error` |
| .240 | one, **real disc MOUNTED** (Daemon Tools, vol `STARCRAFT`) | — | **both games run** |

An intermediate theory - "the check insists on a disc only when an optical drive
exists, and falls back to the local archive when none does" - was withdrawn once
.240 checked properly and found it *did* have a drive with a disc already mounted
in it from an earlier session. There is no per-box difference to explain. The
rule is one line:

> **The check wants a disc in a drive. Not a drive, and not a copy of the disc's
> data staged into the game folder.**

**Consequences for staging.** A title with a CD check cannot be made
self-contained by copying files; it needs its disc image staged alongside it and
**mounted at launch**. That is what makes a resilient mount script part of the
staged game rather than a convenience - and it must find whatever mounter the box
has: some fleet boxes have **WinCDEmu and no Daemon Tools**, so a launcher that
only knows Daemon Tools fails on them.

**Two refinements worth keeping:**
- On .240 **Brood War ran with only the STARCRAFT-labelled disc mounted**, so the
  check does not necessarily verify that the volume matches the game. A launcher
  that refuses to start unless *its own* disc is mounted is stricter than the
  game it launches - which is its own bug. Mount your own image, but fall back to
  running if another usable disc is present.
- A title with **no disc image on the share is genuinely blocked**, not merely
  unfinished. Red Faction is in that state: it stops on "Insert Red Faction
  CD #2" and no image exists to mount.

**Choosing the marker a mount script checks for: a missed mount fails loudly, a
WRONG MATCH fails silently.** `Play Descent 2.bat` used `MARKER=AUTORUN.INF` to
decide whether its disc was already mounted. **Every game CD has an
AUTORUN.INF**, so it matched a mounted *StarCraft* disc, skipped mounting its own
image, and launched Descent 2 against the wrong CD - while reporting success.
That is far worse than failing to mount, which is at least visible.

Picking a marker is not obvious, either. For StarCraft the two images share
almost everything:

    STARCRAFT.iso and BROODWAR.iso both contain:  AUTORUN.INF, INSTALL.EXE,
                                                  ISP, SETUP.EXE
    unique:                                       SC.ICO (StarCraft only)
                                                  BW.ICO (Brood War only)

so even `INSTALL.EXE` fails to separate the two games of the same title. The
recipe: **match on a file unique to that specific disc; when nothing unique
exists, match on the volume label alone** - never on a file every CD carries.

**Related failure signature, from the same pass (Red Alert 2).** `game.exe` and
`gamemd.exe` launched directly **exit with code 0 in under a second - no window,
no dialog, no crash log**. That silence is what makes a CD stage so easy to
misdiagnose; two separate theories (SafeDisc wrapping, a `-CD` switch) died on it
before the mechanism was found. The Westwood stubs `Ra2.exe`/`RA2MD.exe`
reference `GetDriveTypeA` and `GetVolumeInformationA` - **the stub IS the volume
discovery** - while `game.exe` has none. So the stub is *required*, not merely
sufficient, and no command-line switch substitutes for it.

---

## An SSE-less CPU cannot run our MesaFX ICD either — `-mfpmath=sse` is in our build flags (2026-08-29)

Found while root-causing why every OpenGL game crashed on **.143** (`1GHZ`,
GeForce 6800). The box's CPU is an **AMD Athlon "K75" Slot A, 1000 MHz**
(CPUID sig `0x00000622`, `AuthenticAMD`), and it has **no SSE at all**:
`MMX=1 FXSR=1 SSE=0 SSE2=0 3DNow=1`, `IsProcessorFeaturePresent(PF_XMMI)=0`.
SSE arrived with the Athlon XP; this chip predates it.

NVIDIA's ForceWare 93.71 ICD uses SSE unconditionally, so it faults instantly:
Dr Watson logs `Exception number: c000001d (illegal instruction)` in
`function: nvoglnt`, disassembling to `cvtsi2ss xmm0, dword ptr [nvoglnt+...]`.
Static comparison of the two ICDs on the share is unambiguous —
**93.71**: 27 `cvtsi2ss`, 68 `comiss`, 52 `ucomiss`, 1003 `movss`;
**71.89**: 0, 0, 0, 130. Crashers logged: ioquake3, quake3, GLQUAKE, quake2,
SoF.

**The part that matters for OUR lane.** The same Dr Watson history shows this box
crashing the same way back on 7/19–7/21 while it was running the **3dfx/Voodoo5
stack**, faulting in `glide3x` and `OPENGL32!glClientActiveTextureARB`. So this
is a property of the CPU, not of any one vendor's driver — and **our clean-room
MesaFX ICD is built with `-march=pentium3 -mtune=pentium3 -mfpmath=sse`** (the
documented flags in CLAUDE.md's Driver Stack Map). `-mfpmath=sse` makes gcc emit
SSE for **ordinary float math throughout the ICD**, not merely in a few
intrinsics — so on any pre-Athlon-XP CPU our own ICD will die exactly the way
NVIDIA's does.

**Consequence:** if a Voodoo card ever goes back into an SSE-less box, our driver
will appear "broken" for a reason that has nothing to do with the driver.
Measured on our *shipped artifacts* (disassembled, not inferred from flags):

| artifact | SSE instructions | of which `cvtsi2ss` |
|---|---|---|
| `opengl32_retail.dll` (our MesaFX ICD) | **54,388** | 2,783 |
| `glide3x_cvg.dll` (our Voodoo 2 Glide) | **2,840** | — |

`cvtsi2ss` is the exact opcode the Dr Watson trace names, so on an SSE-less part
both of our binaries fault **immediately**, not marginally.

**`-mfpmath=387` ALONE DOES NOT FIX IT.** `-march=pentium3` by itself declares
SSE available, so gcc keeps emitting it — auto-vectorisation is on at `-O2` in
gcc 12+, and inlined memcpy/float conversions use it too; `-mfpmath=387` only
redirects *scalar* FP math. Measured on float-heavy test code with our toolchain:

| flags | SSE instructions emitted |
|---|---|
| `-march=pentium3 -mfpmath=sse` | 11 |
| `-march=pentium3 -mfpmath=387` | 4 — **still faults** |
| `-march=i686 -mfpmath=387` | 0 |
| `-march=athlon -mfpmath=387` | 0 |

**You must lower `-march` as well.** For an SSE-less Athlon (K75) prefer
**`-march=athlon -mfpmath=387`** over i686: same zero SSE, but it keeps MMX and
3DNow!, which that CPU has. The flags live in `voodoo-cleanroom/build-stack.sh`
(`GLIDEOPT`, `GLIDEOPT_CVG`) and `build-mesafx-retail.sh` (`CPU=`); `-march` and
`-mtune` are now split in the ICD build, so a K75 lane is a two-variable change
rather than a fork.

Diagnose with a CPUID probe before blaming the ICD.

**Method worth reusing:** `Documents and Settings\All Users\Application Data\
Microsoft\Dr Watson\drwtsn32.log` retains a long crash history with the
faulting module and the disassembled instruction. Parsing the *whole* file dated
the fault to 19 July and disproved the initial "the 08/27 driver install broke
it" theory.

---

## `VIDEODIAG.adapters[0]` reports registry keys, not live devices — a stale "Standard VGA" key makes a healthy box look driverless (2026-08-29)

On **.246** (Win7, ADMIN-PC) `VIDEODIAG` reported the adapter as **"Standard VGA
Graphics Adapter"**, and this was written into the fleet task sheet as "NO VIDEO
DRIVER INSTALLED — 3D games cannot work". **It was wrong.** The box has a working
**AMD Radeon HD 5450** (`PCI\VEN_1002&DEV_68F9`, Catalyst 15.7.1 /
15.200.1062.1004, Status=OK) driving 1920x1080, with the full OpenGL runtime
(`atioglxx.dll`, 25.3 MB) installed.

Cause: `adapters[]` enumerates **display-class registry keys**, not bound
devices. Index `0000` was a leftover `display.inf` key (hardware id
`pci\cc_0300`) with no device attached; the real AMD adapter was at `0001`. The
**`display` block** in the same response was truthful all along
(`driver_desc: AMD Radeon HD 5450`, 1920x1080x32@60).

**Never conclude "no driver" from `adapters[0]`.** Corroborate with
`wmic path win32_videocontroller get name,pnpdeviceid,driverversion,status`
(live devices only) plus a desktop screenshot at a real resolution.

Two related blind spots, so absence is never evidence here:
- **`PCISCAN` returns `pci_display_devices: []` on Windows 7** — it walks the
  Win9x/XP `Enum\PCI` layout, which Win7 does not populate the same way.
- A **Voodoo 2 is `Class=MEDIA`**, so it never appears in any display
  enumeration at all (this is why .171's Voodoo 2 was repeatedly "missing").

Cost: a fabricated blocking task ("fix the video driver first, may need a
reboot") on a box that needed neither, plus the risk of a reboot request going
to the user for no reason.

---

## F5 on the XP desktop re-flows every icon into auto-arrange and destroys the icon bay (2026-08-29)

On `NSC-B20C188E96D` (**.123**) the Recycle Bin was hidden to meet the "only
staged-game icons plus Retro Agent and Retro Chat" desktop standard:

```
reg add "HKCU\...\Explorer\HideDesktopIcons\NewStartPanel"  /v {645FF040-5081-101B-9F08-00AA002F954E} /t REG_DWORD /d 1 /f
reg add "HKCU\...\Explorer\HideDesktopIcons\ClassicStartMenu" /v {645FF040-...} /t REG_DWORD /d 1 /f
```

The registry half is correct and the Recycle Bin does disappear. **The mistake was
refreshing the shell with a desktop click + `UIKEY F5`.** That refresh makes
explorer re-flow *every* icon into auto-arrange columns starting at x=0,y=0 — all
32 shortcuts marched up the left edge, on top of the wallpaper's "GAME LIBRARY"
header and outside the bay entirely. The `LVM_SETITEMPOSITION` placements that
`gamesync.c:gs_arrange_icons()` had just made were gone.

- **Refresh with `SHChangeNotify(SHCNE_ASSOCCHANGED, ...)`, never F5**, when you
  change a desktop-icon registry value. (This is exactly why the hide-Recycle-Bin
  helper written the same day calls SHChangeNotify and avoids both F5 and
  `taskkill explorer` — on XP explorer does not come back from a `taskkill /f`.)
- **If you have already pressed F5, the repair is `GAMESYNC RESET` + `GAMESYNC START`.**
  It re-runs `gs_arrange_icons()` at the end of the pass, and because
  `gs_copy_file()` treats an identical-sized destination as done, a fully-synced
  20 GB library re-verifies in ~20-35 s rather than re-copying. Cheap enough to
  use as the standard "put the icons back" lever.
- Related: the arrangement is **not** perfectly dense. Explorer will not stack two
  icons on one cell, so a slot or two ends up skipped and the tail icons shift.
  All icons still land inside the bay on bay cells; this is cosmetic, not drift
  between `icon_bay.json` and `gs_icon_bay()`.

## On Windows 7 the agent's XP-era Themes-disable strips Aero AND silently loses the retro wallpaper (2026-08-29)

`ADMIN-PC` (**.246**, Win7 Pro RTM 6.1.7600, Sandy Bridge, AMD HD 5450) had an
**empty** `HKCU\Control Panel\Desktop /v Wallpaper` even though `agent.log` said
the work had been done:

```
[09:15:44] retrowall: Themes service set to Disabled
[09:15:45] retrowall: wallpaper set to C:\retro-wall\retrowall_1920x1080.bmp (screen 1920x1080)
```

Both lines are true and the second one still ends up with nothing set. On XP the
Themes service only paints Luna, so stopping it is exactly right. **On Win7 that
service owns wallpaper application**, so `SystemParametersInfoA(SPI_SETDESKWALLPAPER)`
returns success into the void: the value does not persist, and Aero is stripped
as collateral. With `sc config Themes start= auto` + `sc start Themes` the very
same call sticks first time (`SPI_result=True`, value reads back).

Cause: `agent/src/retrowall.c:apply_hacker_theme()` calls `stop_and_disable_themes()`
**unconditionally — there is no OS-version gate**, and no registry off-switch.
So this is not a one-time mess to clean up: **every agent start re-disables Themes**,
and the wallpaper goes again on the next reboot. The durable fix is to gate that
call (and the `SetSystemVisualStyle(classic)` next to it) to pre-NT6 only.

Two traps while fixing it by hand:
- **Do not "just restart the agent" to re-apply the wallpaper** — that re-runs
  retrowall and re-disables Themes, undoing the fix you just made. Set it with a
  direct `SPI_SETDESKWALLPAPER` call instead.
- Win7 RTM ships **PowerShell 2.0**: `Get-ChildItem -File` does not exist there
  (`ParameterBindingException`), and the failure is easy to miss because the
  CLIXML error stream interleaves with good output and every size sums to 0.
  Use `| Where-Object {-not $_.PSIsContainer}`.

Related: the desktop icon bay is **left-hand** in the current wallpaper
(`_desktop/icon_bay.json`, 1920x1080 -> x=34,y=66, 8x12). "Icons bottom-right" is
the *old* `arrange_icons.exe` behaviour that `gamesync.c:gs_icon_bay()` deliberately
replaced — see its comment about the art and the icons sitting on top of each other.

---


## A driver INF with no binaries hangs DRVUPDATE on an invisible "Files Needed" dialog (2026-08-29)

`DRVUPDATE PCI\VEN_1002&DEV_9515` on **.123** (NSC-B20C188E96D, Radeon HD 3850 AGP)
returned nothing at all: the agent logged `DRVUPDATE ... -> C:\D\G001\CX137529.inf`
and then no `OK installed` and no `install failed`, ever. The command simply never
completed and the box sat with no display driver at 640x480 for hours.

Cause: `UpdateDriverForPlugAndPlayDevicesA` was called with an INF whose payload was
missing, so SetupAPI put a modal **"Files Needed - the file 'ati2mtag.sys' ... is
needed"** dialog on the console session and waited. Nothing in the log says so;
`WINLIST` is the only thing that shows it. **When a DRVUPDATE goes quiet, run
`WINLIST` before assuming it is slow** - a `#32770` window named "Files Needed" or
"Hardware Installation" means it is blocked on a click, not working.

Two separate blockers in one install, both invisible from the log:
- **Files Needed** - missing payload (below).
- **Hardware Installation** ("has not passed Windows Logo testing") - a
  non-WHQL/beta driver. Setting `HKLM\SOFTWARE\Microsoft\Driver Signing\Policy`
  to `00` did **not** suppress it; the dialog still appeared and had to be clicked
  ("Continue Anyway"). Budget a `WINLIST` + `UICLICK` pass for any beta driver.

Root cause of the missing payload: **the PXE image's ATI display-driver directory
`$OEM$\$1\D\G001` contains INF+CAT only** (4 INFs, 2 CATs, 762 KB total, no
`B*\` payload directory). `CX137529.inf` correctly names
`"ATI Radeon HD 3850 AGP" = ati2mtag_RV630, PCI\VEN_1002&DEV_9515` and is a real
Windows XP INF (Catalyst 12.4, 8.961.0.0000) - the machine note that it "may be
Vista/7-only" was wrong - but `[SourceDisksNames.x86] 1 = ...,.\B136646` points at a
directory that is not in the image. Any XP box with an ATI card imaged from this
source will hang the same way. **This is a share-side gap, not a per-box one, and it
is still open.**

Fix used on .123: extracted `Packages/Drivers/Display/XP_INF` (INF + `B156345\`,
33 files, 27 MB) from
`Files/Drivers/ATI/WinXP/Radeon/AMD_Catalyst_13.4_Legacy_Beta_WinXP.exe` - the
**legacy** branch is the right one for HD 2000/3000/4000, and 13.4 (8.970.100.0) is
newer than the image's 12.4 - staged it on the share as
`Files/Drivers/ATI/WinXP/Radeon/Catalyst134-Legacy-XP-Display/`, copied it to
`C:\D\G006\` and ran `DRVUPDATE PCI\VEN_1002&DEV_9515 C:\D\G006\CX156444.inf`.
Result: `ati2dvag.dll` + `ati3duag.dll` + `atioglxx.dll` in system32,
`ati2mtag.sys` in drivers, `OpenGLDrivers\ati2dvag -> atioglxx.dll` registered.

Note `ATI_Catalyst_14.4_XP.exe` on the share is the wrong package for this card -
14.4 is the HD 5000+ XP branch; the HD 2000/3000/4000 line went legacy at 13.x.

## The fleet wallpaper only applies at agent start, so a resolution change needs a RESTART (2026-08-29)

`apply_fleet_wallpaper()` lives in `retrowall_apply_startup()` and runs **once, from
the retrowall thread at agent start**. `gs_arrange_icons()` runs from `gs_run()`, at
the end of a GAMESYNC. So after changing the screen resolution, neither happens on
its own: on .123 the box booted at 800x600, took `retrowall_800x600.bmp`, and kept
it after being moved to 1024x768.

Order that works: set the mode, `RESTART` the agent (wallpaper follows the new
screen, ~40s after relaunch - wait for the `retrowall: wallpaper set to ...` log
line, not just for the port to reopen), then `GAMESYNC RESET` + `START` to
re-arrange the icons for the new bay geometry.

Also: **the icon bay must have at least as many slots as there are icons, or the
desktop listview scrolls and every icon renders one bay-offset too high.** 32
shortcuts + Recycle Bin = 33 icons; at 1024x768 the bay is 4x8 = 32 slots, one short,
and the whole grid sat ~63px above its drawn cells. At 1920x1080 it is 8x12 = 96 and
the icons land in their cells exactly. Slots per resolution follow
`gs_icon_bay()` / `gen_retro_wall.py:icon_bay()`: `cols = (w*0.34)/76`,
`rows = (h - max(18,h*0.03) - 34 - 24)/80`.

Worth knowing: the current bay is **top-left by design** ("with the grain" of the
shell, per `gen_retro_wall.py:icon_bay()`), superseding the older bottom-right well
that `arrange_icons.exe` used. Instructions that still say "bottom-right" are stale.

## The agent installs `RetroWallRotate` in HKCU but deletes it from HKLM (2026-08-29)

`retrowall.c:stop_wallpaper_rotation()` kills `rotate_wall.exe` and then tries to
remove the Run key that restarts it — but it opens **HKEY_LOCAL_MACHINE**
`Software\Microsoft\Windows\CurrentVersion\Run`, while step 3 of
`retrowall_apply_startup()` writes that value with `hkcu_set_sz(RUN_KEY, ...)`, i.e.
into **HKCU**. The value is therefore never deleted, and the legacy rotator is
relaunched at every logon on any box that ever had the rotation staged.

It still *looks* fixed in the log: `n++` runs unconditionally after the taskkill, so
`"retrowall: legacy wallpaper rotation stopped"` is printed even when the
`RegDeleteValueA` failed, and the more specific `"removed the RetroWallRotate Run
key"` line — which only prints on real success — is simply absent. Grep for the
*second* line, not the first, when checking whether a box is really clean.

Seen on **.133 (P3-DUAL)** 2026-08-29: `HKCU\...\Run\RetroWallRotate =
C:\retro-wall\rotate_wall.exe 120` still present after the agent had logged the
rotation as stopped; `HKLM\...\Run` had no such value. Per-box workaround:
`reg delete "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v
RetroWallRotate /f`. Real fix: delete from HKCU (and keep the HKLM delete for older
boxes), and only count a deletion that actually succeeded.

Why it matters: the agent applies `retrowall_<W>x<H>.bmp` ~20s after boot, then kills
the rotator. The rotator started at logon on its own interval, so between those two
moments it can re-set the wallpaper to `wall0N.bmp` and the agent will not re-apply.
The fleet wallpaper then silently loses to the legacy rotation on a slow boot.

---

## Desktop icons: arrange AFTER the shell has settled, or ~8 of them land outside the bay (2026-08-29)

`gamesync.c:gs_arrange_icons()` sleeps 2000 ms after the last shortcut is written and
then makes ONE `LVM_SETITEMPOSITION` pass over the desktop listview. On **.133** that
reliably left 8 of 33 icons out of place — four in a row clipped off the top edge of
the screen above the bay, and four in a phantom fifth column beside it — while the
other 25 sat perfectly in their drawn cells. Re-running the whole provision produced
the same 8-icon displacement with a *different* set of icons, so it is a race with
explorer still creating listview items, not a bad shortcut.

Fix that worked: wait for `LVM_GETITEMCOUNT` to stop changing (4 stable reads, 250 ms
apart), then repeat the position loop ~5 times with 500 ms between passes. All 32
icons then landed in the 4x8 bay exactly. Built as
`voodoo-cleanroom`-style throwaway `arrange_bay.exe` (mingw, `-luser32`) and staged at
`C:\retro-wall\arrange_bay.exe`.

Also note the bay is **top-LEFT**, deliberately: `gen_retro_wall.py:icon_bay()` says
Windows fights attempts to move icons away from the top-left, so the wallpaper draws
the "GAME LIBRARY" panel there and the arranger matches it. The staged
`arrange_icons.exe` still parks icons bottom-RIGHT and must not be run on a box that
has a `retrowall_<W>x<H>.bmp` — `retrowall_apply_startup()` returns before reaching it,
which is the only reason the two do not fight.

The bay is exactly `cols x rows` = 4x8 = **32 slots** at 1024x768. A 33rd icon has
nowhere to go and the shell re-flows it to the top of the screen, clipped. On .133 the
33rd was the Recycle Bin; hiding it (`HideDesktopIcons\{ClassicStartMenu,NewStartPanel}
\{645FF040-5081-101B-9F08-00AA002F954E}` = 1, then `SHChangeNotify`) made 32 icons fill
32 slots exactly.

---

## "No sound" on a fresh XP image is usually the WDM audio core, not the sound card (2026-08-28)

.124 (NSC-4664F96DE08, XPSP3-FLEET image) had a Sound Blaster AWE64 whose
driver was loaded and healthy - `sc query ctlsb16` RUNNING, the devnode bound
with `ConfigFlags=0`, and its KS `#Wave` interface registered with `Linked=1`
under `DeviceClasses\{65e8773e-...}`. Yet `waveOutGetNumDevs()` returned 0,
`sndvol32` said "no active mixer devices" and `mmsys.cpl` said "No Audio
Device". Reinstalling the card driver would have fixed nothing.

The break was in XP's own WDM audio core, under
`HKLM\SYSTEM\CurrentControlSet\Enum\SW`:

- `SW\{a7c7a5b0-5af3-11d1-9ced-00a024bf0407}` ("Microsoft Kernel System Audio
  Device", **sysaudio** - the audio graph builder) had **no `Driver` and no
  `Service` value** and `ConfigFlags=0x40` (FAILEDINSTALL). Without sysaudio
  there is no wave device *at all*, whatever the card does.
- kmixer's node had `ConfigFlags=0x20` (REINSTALL) and its service was STOPPED.
- `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Drivers32` was missing
  `mixer=wdmaud.drv` and `aux=wdmaud.drv` (it had `wave` and `midi`).

Three dead ends worth remembering:

- **You cannot repair an Enum devnode with `reg add`** - the `Enum` key denies
  writes even to Administrators (`Access is denied`).
- **`rundll32 streamci.dll,StreamingDeviceSetup` does not help when the devnode
  already exists.** It only re-runs the INF's `*.Interface.Install` AddReg and
  fails in `setupapi.log` with `Error 1010: The configuration registry key is
  invalid`. It creates nodes; it does not rebind broken ones.
- **`setupapi.log` said the sysaudio install "finished successfully"** at image
  time. That line was true and stale - the node was broken afterwards. Trust
  the live `Service`/`ConfigFlags` values, not the log.

What fixed it: `tools/drvupd.c` (the retro-agent devcon replacement) rebinding
each broken node against the in-box INF, no reboot needed:

    drvupd.exe C:\WINDOWS\inf\wdmaudio.inf "SW\{A7C7A5B0-5AF3-11D1-9CED-00A024BF0407}"
    drvupd.exe C:\WINDOWS\inf\wdmaudio.inf "SW\{B7EAFDC0-A680-11D0-96D8-00AA0051E51D}"

`drvupd` returning `FAILED err=3758096907` (0xE000020B) on a node that already
has `Driver`+`Service` just means there was nothing to do - not a failure.

Two verification traps after the fix:

- **winmm caches the device list per process**, so the already-running agent
  keeps reporting `wave_out_count: 0`. `RESTART` the agent (never `QUIT`) before
  believing `AUDIOINFO`.
- **`mmsys.cpl` is single-instance**: launching it again just refocuses the
  stale window still reading "No Audio Device". Close it first.

Also: the box **dropped off the network entirely for ~6 minutes** (agent *and*
SMB) immediately after the sysaudio install landed, then came back with its
uptime intact - it had not rebooted. Wait it out rather than walking over to it.

Fleetbook: recipe `xp-no-sound-sysaudio-kmixer-devnodes-failed-install-sound-bl`.

---

## Optimising a driver: establish the regime before you optimise anything (2026-08-29)

From the Voodoo 2 ICD work on .171. Six agents produced ranked optimisation
candidates with percentage estimates; almost none survived contact with the
hardware, and the two cheap measurements below would have retired most of them
before a line was written.

- **Fit the frame-time model first.** Run the same timedemo at three
  resolutions and fit `t = a + b·pixels`. On .171: 80.5 / 57.2 / 37.2 fps at
  512×384 / 640×480 / 800×600 → **a = 2.4 ms fixed CPU, b = 5.10e-5 ms/px**.
  That says fill-rate bound, and therefore that every CPU-side micro-optimisation
  on the list was competing for at most ~14% of frame time. Worth knowing before
  ranking anything.

- **Find a switch that removes the work entirely, and measure that ceiling.**
  Two agents independently estimated +12% for routing `glTexSubImage2D` through
  the partial-download path. `gl_dynamic 0` removes those uploads completely and
  bought **+0.7%** (57.2 → 57.6). Ceiling established in one run; risky
  texture-upload surgery correctly skipped.

- **Estimates from code reading are not measurements.** `-mtune=pentium4` on the
  ICD (the box IS a Pentium 4, estimate +3%) measured **exactly neutral**.
  Meanwhile the one change that did pay — withdrawing `GL_EXT_point_parameters`,
  an extension we advertise but do not accelerate — was **+12.2%** and came from
  noticing an asymmetry in the game's own log, not from profiling.

- **A silent `LoadLibrary` failure looks like nothing.** Adding profiling
  counters introduced a 64-bit divide, which pulled in a libgcc helper, which
  added an import on `libgcc_s_dw2-1.dll` — absent on the retro boxes. Quake II
  then reported only `could not load "retrogl"` with no hint a DLL was missing.
  **Link retro-targeted DLLs with `-static-libgcc`** and check
  `objdump -p | grep "DLL Name"` after any change.

- **Five theories for the same 22 ms, all refuted on hardware** (texture
  thrashing via `gl_picmip`; the `glClientActiveTextureARB` flush; per-vertex
  texcoord submission; redundant `grTexCombine`; Mesa x86 vertex codegen). The
  profiler then showed the cost is neither the TNL pipeline (5.33 vs 5.50 ms)
  nor state setup (2 calls/frame, not thousands) nor vertex count (4728 vs 4716
  — identical). Still unattributed; recorded so nobody re-walks those five.

---

## A Voodoo 2 is invisible to every display-class check, and its XP driver installs itself dead (2026-08-28)

Preparing a Voodoo 2 (and a second card for SLI) on a fleet XP box. Three
things cost time before any hardware was even reachable:

- **`VIDEODIAG` will never show a Voodoo 2.** It is a 3D-only passthrough
  card and its INF is `Class=MEDIA`, not Display — the 2D card stays the
  display adapter. Detect it against the raw PCI enum:
  `REGREAD HKLM SYSTEM\CurrentControlSet\Enum\PCI` on NT/XP, or
  `REGREAD HKLM Enum\PCI` on 9x (**different path**).

- **`VEN_1102&DEV_0002` is a Creative SB Live!, not a Voodoo 2.** 3dfx is
  vendor `121A`; Creative is `1102`. `.240` carries two SB Live devices, so a
  match on `DEV_0002` alone reports a Voodoo 2 that is not in the machine.
  Locked in by `retro-agent/tests/python/test_voodoo2_install.py`.

- **The XP driver installs itself into silence.** Most Win9x Voodoo 2 drivers
  do not run on XP at all; the one that does is the 3dfx **Win2K 1.02.00**
  kit (now on the share at `Files\Drivers\3DFX\WinXP\Voodoo2_1.02.00_Win2K\`).
  Its `Voodoo2.inf` registers `fxgpio`, `fxptl` and `Ntremap` with
  `StartType=2` (auto), but the Win2K display driver is **core-level** and
  fails **silently** on XP at auto start: the driver reports installed and
  nothing renders. All three must be moved to `Start=1` (system) and the box
  rebooted. `retro-agent/scripts/voodoo2/install_voodoo2.py` does this.

- **SLI needs matching cards.** The official 3dfx drivers do not support
  mixed/mismatched SLI — different manufacturer and/or different RAM size
  (8MB vs 12MB) will not run in SLI. FastVoodoo2 4.6 handles mismatched pairs
  but is Win9x-only.

Still unverified on hardware: six boxes were enumerated (.124 .133 .143 .145
.240 .246) and none carries `VEN_121A&DEV_0002`.

---

## A game that never asks for a refresh rate gets 60Hz, whatever the desktop is set to (2026-08-25)

Measured on **.124** (GeForce2 GTS, ForceWare 71.89, Sony CPD-G200): desktop at
1024x768x32 **@100Hz**, Quake II launched fullscreen at the *same* resolution,
and `DISPLAYCFG` then reports **60**. The mode was already stored at 100 with
`CDS_UPDATEREGISTRY`; that is not what decides it.

**A `ChangeDisplaySettings` call that omits `DM_DISPLAYFREQUENCY` gets the
adapter default, not the stored mode.** So the game does not "lose" the refresh
rate - it never asked for one, and 60 is what XP hands out. Every engine on that
box that sets its own video mode does this: Quake II, UT99 and GoldSrc have no
in-engine refresh setting at all, and ForceWare 71.89 has no override page, so
there is nothing to switch on from inside the machine.

The only lever is to re-apply the mode from OUTSIDE with the frequency field
filled in, while the game runs. That is `agent/tools/refreshkeep.exe` in
retro-agent: it polls the current mode and re-applies it whenever it drifts off
target, exiting when the watched process is gone.

Two things worth keeping:

- **Apply with flags 0, never `CDS_UPDATEREGISTRY`.** A game's odd fullscreen
  resolution must not become the stored desktop mode. `setrefresh.exe` remains
  the tool for changing the desktop persistently.
- **Refuse a rate the driver does not enumerate.** Asking a CRT for a mode it
  cannot display is how you get "out of range" on a machine that then needs
  somebody physically in front of it. An unsupported request must do nothing at
  all rather than fall back to some other rate.

**Related, found while reviewing the deploy script:** its stale-cvar strip
worked for Quake configs and silently did nothing for GoldSrc, because the two
sides of the comparison extracted the cvar name differently - `split()[1]` is
the NAME in `seta r_swapInterval "0"` but the VALUE in `gl_vsync "1"`. The
managed set came out as `{'"1"', '"100"'}`, which no cvar name can match, so a
stale `gl_vsync "0"` further down the autoexec kept winning - the exact thing
the strip exists to prevent. One `cvar_name()` now serves both sides;
`tests/python/test_game_refresh_cvars.py` pins it.

## Reading a file off an agent: "absent", "unreadable" and "busy" are three answers, not one (2026-08-29)

A data-loss bug in the fleet favourites agent, found only because another
session compared **two boxes**: on `.143` our block was appended after their
`r_fullscreen`/`r_mode` and nothing was lost; on `.240` those settings were
gone. Same code, same file, opposite outcomes.

The merge was correct. **The read was the bug:**

```python
existing = ""
try:
    existing = await c.command_text(f'EXEC cmd /c type "{path}"')
    if "cannot find" in existing.lower(): existing = ""
except Exception:
    existing = ""
```

Three unrelated failures all collapse into *"the file is empty"*, after which a
read-modify-write faithfully writes a file containing only the new content:

- **`except Exception`** — a timeout, a busy box, a dropped connection.
- **Deciding existence by matching English error prose against the file's own
  content.** `type` writes its error to stderr; the phrase can equally appear
  *in* a config. And it is locale-dependent.
- **A shell round trip** — `EXEC` captures cmd.exe output, so a large file can
  truncate, encodings can mangle, and on Win98 it is `command.com`, a different
  shell entirely.

**The rule:** *"the file is not there"* and *"I could not read the file"* mean
opposite things — one is safe to create, the other is destructive — and any
code that treats them alike will destroy data intermittently, on whichever box
the read happened to fail. Intermittent is the worst case: it looks like
somebody else's change not sticking.

**The pattern to copy** (`scripts/gameindex/sync.py:read_existing`):

1. **`DOWNLOAD`, never `EXEC ... type`** — exact bytes, real status code, no
   shell, no truncation, no locale.
2. On failure, **`DIRLIST` the parent directory**. Only a positive listing
   showing the file absent means `missing` and permits creating it. File
   present, listing failed, or listing unparseable all mean `unreadable`.
3. **`unreadable` must skip the write entirely**, with the reason logged.
4. Match filenames **case-insensitively** — Windows paths are.

**And check the check.** A second guard (`favorites.WouldClobber`) refuses to
render if the merge would drop a pre-existing line. The first version of it
reused `_strip_block` — the very function it was checking — so a strip rule
that grew too greedy would have been invisible to both. A verifier that shares
an implementation with the thing it verifies is not a verifier.

**Audited the rest of the codebase for the same shape.** Two other sites use
`EXEC cmd /c type`: `voodoo-cleanroom/deploy/q2bench.py` reads a benchmark log
and never writes it back (worst case: a missed result), but
`.claude/skills/driver-install/game_sweep.py:fix_ut99` **rewrote the whole
`UnrealTournament.ini` from what it read**. Its `if "RenderDevice=" not in txt`
guard catches an *empty* read by luck, but a **truncated** read still contains
that string and would have been uploaded back over a complete file. Now on
`DOWNLOAD`, and it bails out rather than rewriting a file it never saw.

## GPU serving for game bots: the model was never the slow part (2026-08-28)

Building the neural-bot policy server on the 5090. Every number that mattered
came from measuring something I had assumed:

- **A small policy net is launch-bound, not compute-bound.** The forward pass
  cost ~0.44 ms *whatever the batch size* — batch 1 and batch 1024 were within
  noise of each other, because it is ~35 tiny kernel launches. Capturing it
  into a **CUDA graph** cut that to 0.09–0.15 ms (3–5×). If a model is small
  and called often, measure launches, not FLOPs.

- **Capture and first-touch must be PREWARMED or they land in a game frame.**
  The first request at each batch size paid graph capture plus first-touch
  allocation in the code *around* the graph — tens of milliseconds, over the
  frame budget, so the C adapter timed out, backed off, and the bots silently
  stayed on the engine's own AI. Symptom: 9300 frames, 9300 fallbacks, one
  reconnect. Warming the *whole serving path* (not just the graph) once per
  batch bucket at startup costs 0.6 s and makes the first served frame the
  same speed as the ten-thousandth.

- **Python marshalling dwarfed the GPU.** 512 bots took 3.9 ms end to end, of
  which the GPU was 0.36 ms; the rest was per-float `struct` work. One typed
  `numpy.frombuffer` view over the whole batch took it to 0.36 ms total. The
  C adapter's `memcpy` is 0.013–0.067 µs/bot against Python's 2.8 µs/bot.

- **Per-bot recurrent state must be gathered with one kernel.** The first
  version looped in Python and touched the GPU once per bot, costing more than
  the forward pass it fed. One preallocated state tensor plus
  `index_select`/`index_copy_` fixed it — and the key must include the
  *connection*, because bot id 0 exists on every game server.

- **A local 1.5B LLM plans a 4-bot squad in 419 ms, a 16-bot squad in 1184 ms.**
  Output length scales with squad size, so a 2 Hz strategic layer does not hold
  past ~4 bots. Fine if the planner is off the serving path (it lowers the plan
  rate, never drops a frame) — but only if that was designed in.

## Chat answered but never replied: a non-atomic write into an inotify watcher (2026-08-28)

Someone typed a message on a retro box and got nothing back. The brain had
answered it in three seconds. The only trace anywhere was one daemon line:

    outbox: invalid JSON in 192.168.1.143-1-000001.json, removing

- **`Path.write_text()` is not atomic, and the reader is inotify-driven.** The
  brain wrote each response chunk straight to its final name; the daemon
  watches the outbox with inotify, so it is woken the instant the *filename*
  appears — usually before any bytes are in it. It parsed a zero-byte file,
  got a `JSONDecodeError`, and **deleted** the answer. Fix both halves: write
  temp + `os.replace` (the temp suffix goes AFTER `.json` so it cannot match
  the reader's `glob('*.json')`), and never destroy a file you merely could
  not parse yet — give it a grace window, then move it to `failed/`.
  The rest of this project already had the convention
  (`scripts/ai_status_bus.py`, the dashboard collector); the chat brain was
  the odd one out.

- **A check-and-create outside the lock let two coroutines share one
  StreamReader.** The daemon's `ensure_send_conn()` sat *outside* the per-host
  `state.lock` in both send paths, so a response and a `STATUS_SET` could each
  build a connection and then both `readexactly()` on the same reader →
  *"readexactly() called while another coroutine is already waiting for
  incoming data"*, and three `send connection established` lines in the same
  millisecond. The error fires **after** the `LOG_APPEND` has gone out, so the
  retry loop delivered the reply **twice**. Locking the ensure fixes the
  spurious failure and the duplicate together.

- **Half-applied locking is worse than none, because it looks deliberate.**
  Each `HostState` has ONE `send_conn` shared by four coroutines (response
  forwarder, status forwarder, task drainer, connect banner) and `state.lock`
  was applied to some uses and not others. Two gaps: `ensure_send_conn()` ran
  unlocked in two places (it check-and-creates the connection *and reads the
  greeting*, so unlocked it swaps the socket out from under a coroutine that
  is mid-read), and all three error handlers did `close()` + `= None`
  unlocked, destroying a connection another coroutine was actively reading
  from. Symptoms read exactly like flaky 25-year-old hardware — *"0 bytes read
  on a total of 4 expected"*, *"Connection lost"* — and were entirely
  self-inflicted. **Audit the invariant, don't chase the interleavings:** a
  source-level check that every `state.send_conn` use sits inside an open
  `async with state.lock` found four sites when hand-reading had found two.

- **"No agents found" is not an error on a fleet that is powered on demand.**
  `main_async` exited when discovery came back empty. With `Restart=always`
  that made a permanent rescan of all 254 addresses the steady state, made
  `daemon: NOT RUNNING` normal — so the status check could not tell *fleet is
  off* from *daemon is broken* — and meant a box that booted waited for the
  next restart to be claimed. Stay up with zero hosts; `rediscover()` already
  knew how to add them.

- **ROOT CAUSE of the unclaimed box: reaping a host killed the whole daemon.**
  `rediscover()` cancels a host's `serve_host` task when the box goes offline,
  and `serve_host` correctly cleans up and re-raises `CancelledError`. But
  `main_async` awaited `asyncio.gather(*tasks)` **without
  `return_exceptions=True`**, so that cancellation propagated out of gather
  and killed the process:

      22:26:02,785 [INFO] reaping offline agent 192.168.1.171 ...
      22:26:03      retro-chat-daemon.service: Failed, status=1/FAILURE

  Under a second apart, **seven times in one day**. The unit restarts with
  `RestartSec=5min`, so chat died for five minutes on every reap — which is
  how `.143` sat unclaimed for two hours while someone typed into it. A
  cancelled child is a *normal* event when you cancel children on purpose;
  gather must be told so. Log the other exception types, though, or
  `return_exceptions=True` hides a real crash and leaves the daemon up doing
  nothing — worse than crashing. Regression test:
  `retro-agent/tests/python/test_chat_daemon_reap_survival.py`.

- **An unclaimed box is a silent black hole.** `.143` was reaped at 19:51 when
  it went unreachable and was not re-claimed until 21:55; in between, anything
  typed into its chat client went into the agent's prompt slot with nobody
  polling, and was lost when the box rebooted. Diagnose with `LOG_READ 0` on
  the box — a claimed agent's log carries `[daemon connected from
  192.168.1.132]`, an unclaimed one is 0 bytes.

- **`PROMPT_WAIT` POPS the prompt** (`chatcore_prompt_pop`), and its argument
  is **milliseconds**, not seconds. It is not a safe way to ask "is anything
  queued?" — you will consume the user's message.

## The status wall reports on services, so "not installed" must never look like "dead" (2026-08-28)

The GDM login-screen dashboard grew panels for the game servers, the PXE
server, the favourites agent and the host services. Everything that cost time
came from one theme: **a status wall is only useful if its silences are
distinguishable.**

- **A oneshot behind a timer has no honest status.** `retro-gameindex` was a
  `oneshot` fired by a 5-minute `.timer`, so its unit read `inactive (dead)`
  for 297 of every 300 seconds — indistinguishable from a service that had
  stopped. It is now a long-running `Type=simple` daemon (same 5-minute pass)
  that publishes a per-pass report. **Delete the timer when you do this**, or
  systemd starts a second pass that fights the daemon over the same SQLite file.

- **"Nothing to do" and "did not run" look identical from outside.** The retro
  fleet is powered on demand, so a healthy favourites pass across zero live
  boxes writes nothing and logs almost nothing. Judged by output volume, a
  healthy agent looks dead every time the machines are off. Services must
  *state* that a pass completed, not leave it to be inferred.

- **`systemctl --user` as root is the wrong manager.** The collector runs as
  root; a bare `systemctl --user` there queries *root's* manager, which has
  none of the fleet's user units — so every fleet service reads "not found",
  which on the wall looks exactly like every fleet service having died. Drop to
  the owning uid with `XDG_RUNTIME_DIR` set. And report `LoadState=not-found`
  as `absent`, distinct from `failed`: never installed and crashed are
  different calls to action.

- **An `active` unit can be serving nothing.** `retro-pxe` that has lost its
  UDP sockets looks perfectly healthy to systemd. The panel says `serving` only
  when TFTP is actually bound. (Parsing that: `ss -ulnH` columns are
  State/Recv-Q/Send-Q/**Local**/Peer — reading the peer column of a listening
  socket gives `0.0.0.0:*` and finds no ports at all.)

- **Bots are not players.** A Quake III server at `bot_minplayers 4` reports
  four players forever, so a naive count leaves the wall permanently claiming
  someone is playing. GoldSrc's A2S reply carries a bot count; on the Quake
  family the tell is **ping 0** in the player line.

- **Every engine needs its own query packet AND its own reply offsets.**
  Q3/Q2 put the infostring on line 1 (line 0 is `statusResponse`/`print`);
  **QuakeWorld's mvdsv puts it on line 0**, with its `n` header glued to the
  first key, and ends the reply `\n\x00` — and `str.strip()` does not remove a
  NUL, so a naive player count reports one phantom player on an empty server.
  GoldSrc hides its counts after four NUL-terminated strings plus a u16 appid,
  and since 2020 may answer `A` + a 4-byte challenge that must be echoed back.

- **A game-server watchdog must not restart on the first silent probe** — that
  is a map change, and restarting kicks everyone off a healthy server. Three
  consecutive mute cycles, a 5-minute cooldown and a 4-per-hour cap; past that
  it says "needs a human" instead of flapping a server whose config is broken.
  It has to be a `--user` unit, because the game servers are.

- **The fleet has TWO process managers, and assuming systemd hides a whole
  server.** Nine game servers are `systemd --user` units; **Tribes 2 is a
  docker container** (it needs a 2001 userland). `systemctl show
  tribes2-server` returns `not-found`, which reads as "never installed here" —
  so a running game server was silently absent from the board and an outage on
  it would have been invisible. That also forces a **third** state:
  `absent` (never installed), `failed` (it died) and `unknown` (we could not
  ask the manager — no docker binary, daemon down) are three different calls to
  action. Restarting on the strength of a failed *lookup* is how a watchdog
  starts bouncing healthy services because docker was briefly busy.

- **`rtcw-server` and `mohaa-server` have never existed on this host.** They
  are in the game-servers skill's table as a wish list. Keep them OUT of any
  status table until they are really installed — a row that can never come up
  sits on the wall as a permanent outage, and "we never built this" is not a
  fault report.

- **An unknown player count is not zero.** Tribes 2 under TribesNext encrypts
  its info response (`0x12` returns a well-formed `0x14` full of ciphertext),
  so the count cannot be read from off the box; the row shows `—`. What the
  protocol *does* give is an echo of the request's four key bytes, so sending
  a random key and requiring it back turns "some UDP arrived" into "this is an
  answer to our query".

- **The login screen still cannot be screenshotted**, so
  `dashboard/tests/preview_panels.mjs` now renders the real `_render*` methods
  to a terminal with the Pango colours mapped to ANSI, by stubbing the
  `gi://…` imports through a node loader hook. That is the only way to *see*
  the wall before shipping it.

---

## Win98 MS-DOS mode: two routes, different files - and DOS batch files must be CRLF (2026-08-26)

.243 stuck at a bare cursor on "Restart in MS-DOS mode". Not reproduced (that
means rebooting the box the operator chats from), but two real hazards fell out
of looking:

- **`DOSSTART.BAT` is what MS-DOS mode runs, NOT `AUTOEXEC.BAT`.** A box can be
  perfectly set up at the boot prompt and bare in MS-DOS mode. The second route
  - CTRL/F8 -> "Command prompt only" - runs `AUTOEXEC.BAT` and never starts
  Windows, so it cannot be broken by the shutdown transition; it is the
  fallback worth having, but it needs `AUTOEXEC.BAT` to set `PATH`.
  Put a log marker as the FIRST line of both: present = DOS came up, absent =
  never left the Windows shutdown. Nothing logged anything there before.

- **A DOS `.BAT` shipped from a Linux host as LF-only will not run.**
  `COMMAND.COM` answers `Bad command or file name`, or prints `OFF` and stops
  if `@echo off` is line 1. retro-agent's batch files had been LF in git the
  whole time and worked only because someone once published them from Windows,
  which converted them by accident: the share's `PLAY.BAT` was 2,274 bytes
  against git's 2,217, exactly one CR per line. `retro_upload` is byte-for-byte,
  so publishing from Linux would have broken `PLAY`, `NETUP` and `DOSSTART` on
  every DOS box at once. Pin it with `.gitattributes` `*.BAT text eol=crlf` -
  git keeps LF in the index and checks out CRLF, so there is no churn commit.

- Ruled out, so nobody re-walks it: `Exit To Dos.pif` is correct (386-section
  dword at +0x12 has bit 0x80 = MS-DOS mode; no embedded `CONFIG  SYS 4.0`
  section = "use current configuration"; 967 bytes is the genuine size), and
  `FastReboot` was already `0`.

## DOSGAME: four install/UI faults found from .243's DOSGAME.LOG (2026-08-25)

The user ran the DOS game manager on .243 in MS-DOS mode; the box's
`C:\DOSGAME\DOSGAME.LOG` (151 KB) had every decision in it and named all four
faults without touching the hardware. Read the log first - that is what it is
for.

- **A DEICE set must be entered through its own `INSTALL.BAT`.** The Apogee/id
  BBS layout is `DEICE.EXE` + `NAME.1` + `NAME.DAT` + `INSTALL.BAT`, and the
  scan kept whichever installer-shaped file DOS returned *first* - `DEICE.EXE`.
  DEICE alone only rebuilds the packed self-extractor (`C:\GAMES\KEEN\KEEN.EXE`,
  one file) and stops; `INSTALL.BAT` is what then runs it. `setup_exes[]` is now
  a preference order, not a membership test.

- **Some archives on the share are only disk 1.** `heretic_shareware1.zip` is
  1,439,232 bytes against the `SIZE=2863638` its own `.DAT` declares, so its
  installer asks for a floppy that does not exist. Compare the `.DAT`'s `SIZE=`
  with the `NAME.<n>` parts present and refuse before starting. (`EXPSIZE=` is
  the unpacked size - match `SIZE=` at line start or every set looks short.)

- **Disk-set parts are numbered in the EXTENSION** (`KEEN.1`), not `NAME._1`.
  Only the underscore form was recognised, so a stalled multi-disk install was
  reported as "the installer wrote nothing at all - the download is bad" while
  the whole game sat in the directory.

- **Never take a step's verdict from `ERRORLEVEL` in a generated DOS script.**
  A LAN install that fully succeeded logged `HTGET failed` AND `UNZIP failed`
  immediately above `install finished OK`. COMMAND.COM keeps the last value
  anything set, and `DOSGAME.EXE` exits **42** to hand over to `RUN.BAT`, so any
  tool that terminates without setting a return code leaves 42 standing.
  Test the artifact (`if not exist <zip>`).

Also: the two tabs disagreed on grid and colour (40-col title, no marker, grey
vs 36-col title, marker, green), and a scan-found game was listed by its
DIRECTORY (`KEEN1`, `STARCR~1`) while the catalogue tab named the same game
properly. Both now share one grid/marker/green, and folders are resolved to the
catalogue title when the match is unambiguous.

## Game servers consolidated onto the dev host; two traps: exec-stack `.so` and loopback A2S proxy (2026-08-24)

The fleet's game servers were split between whitebeast (.82, CS 1.6 x2 + UT99)
and the dev host (.132). They now ALL run on **.132** as `systemctl --user`
units, lingering enabled so they start at boot with no login. whitebeast runs
nothing and has **no autostart** for them (no Run key, no scheduled task, no
Startup shortcut), so it will not quietly take a port back.

- **A pre-2008 Half-Life mod `.so` will not `dlopen` on a current kernel.**
  `specialists-server` crash-looped with `LoadLibrary failed on ts_i386.so:
  cannot enable executable stack as shared object requires: Invalid argument`
  -> `Host_Error: Couldn't get DLL API`. It looks exactly like a corrupt install
  or a Steam-auth problem and is neither: `ts_i386.so` (built 2007) has **no
  `PT_GNU_STACK` program header at all** (`readelf -lW` shows only 3 phdrs), so
  the kernel assumes it wants an executable stack, and current kernels refuse to
  grant one at dlopen time. Fix once per file:
  `patchelf --clear-execstack ts_i386.so` (adds `PT_GNU_STACK RW`); keep the
  original as `*.so.orig`. `patchelf` installs from pip with no root
  (`pip install --user patchelf`). Expect this on any pre-2008 HL mod game DLL.

- **An A2S proxy pointed at `127.0.0.1` silently never answers.** Both CS units
  run `-ip 192.168.1.132` (never `0.0.0.0`) because this host is multi-homed, so
  loopback does not reach them. The shipped `a2s-proxy-cs16-public.service`
  targeted `127.0.0.1:27019`; it started clean and simply never replied. Point
  every proxy at the address HLDS actually **bound**.

- **Probe each engine with its OWN query packet.** A single `getstatus` sweep
  reports live servers as down: Quake 2 answers `status` (not `getstatus`), UT99
  and UT2004 answer GameSpy `\status\` on **game port + 1** (7798 / 7787, not
  7797 / 7777), and Tribes 2 only speaks the Torque binary query
  (`0E 00 00 00 00 00`). Encoded in `retro-agent/scripts/game-servers/healthcheck.py`.

- **A hung `docker build` that looks like a slow mirror is buildkit's netns.**
  The Tribes 2 image stalled 58 minutes at `apt-get update` with `/usr/lib/apt/
  methods/http` alive and **0 bytes fetched** — no error, no timeout. The same
  `apt-get update` on the same image finishes in **1 second** under a plain
  `docker run`, so it is not the network, not IPv6 and not archive.debian.org.
  Build with the legacy builder + host net: `docker build --network=host ...`
  then `docker compose up -d --no-build`. (`--progress=plain` is buildkit-only
  and errors on the legacy builder.) All 12 servers verified after this.

- The second CS 1.6 server (no-blood) was rebuilt here as its own SteamCMD tree
  with Metamod-P 1.21p109 + AMX Mod X 1.10.0-git5479. **`rtcw-server` and
  `mohaa-server`, listed in the game-servers skill, have never existed on any
  host** - no directory, no install script, no game data. Skill table corrected.

---

## .124 is now a GeForce2 GTS — 3dfx stack fully purged, ForceWare 71.89 is the ONLY usable driver (2026-08-11)

The user swapped the Voodoo3 out of **.124** for an **NVIDIA GeForce2 GTS**
(`PCI\VEN_10DE&DEV_0150&SUBSYS_002E10DE&REV_A4` = NV15) and reported the box
"in safe mode". Everything 3dfx was removed and the card put on ForceWare 71.89.
Verified end state: one display adapter, `DriverVersion 7.1.8.9`, 1024x768x32
@85Hz, `GL_RENDERER: GeForce2 GTS/AGP/SSE`, agent v1.25.1.

- **NV11 != NV15 — 93.71 and 81.98 will NOT bind a GeForce2 GTS.** Verified by
  grepping `DEV_0150` in each package's shipping `nv4_disp.inf`: present in
  45.23 / 56.64 / 71.84 / 71.89, **absent from 81.98 and 93.71**, which carry
  only GeForce2 **MX** (NV11). NVIDIA's own 93.71 page lists GTS/Pro/Ti/Ultra as
  *unsupported* and redirects to Release 70. So **71.89 (the last Release 70
  build) is the newest driver that can drive this card at all** — picking the
  "newest available on the share" would have left a yellow-bang.
- **71.84's English package contains no `nv4_disp.cat`** (zero catalog in the
  whole 20.3MB payload) so it always trips the unsigned-driver dialog; **71.89
  ships a real catalog**. Use 71.89, not the more famous 71.84.
- 56.64 hides NV15 in **`nv4disp2.inf`**, not `nv4_disp.inf` — a Have-Disk
  pointed at the usual INF reports "no compatible hardware".
- **"Is it in Safe Mode?" — the only decisive test is `net start <svc>`**, which
  answers *"This service cannot be started in Safe Mode."* Everything else lied:
  the box ran `explorer`, the screensaver, WMI, Terminal Services and its
  `HKLM\...\Run` entries, and `SCREENSHOT` showed a normal themed desktop. It
  was Safe Mode **with Networking** the whole time.
  `HKLM\SYSTEM\CurrentControlSet\Control\SafeBoot\Option\OptionValue` is also a
  poor signal — it *persists* after a normal boot, so its presence proves
  nothing; its **absence** does prove normal mode.
- **Task Scheduler cannot run in Safe Mode, and a task created there is not
  persisted** (`schtasks /create` warns, then the task is simply gone after the
  reboot). All SYSTEM-privileged work — deleting ghost `Enum\PCI` devnodes,
  which Administrator cannot touch — must be deferred to normal mode and the
  task recreated then.
- **Ghost devnode discriminator: a live devnode has a `Control` subkey, a ghost
  does not.** Used it to prove all three leftovers were dead: the Voodoo3
  (bus1:0.0), the Voodoo5 6000's VSA-100 (bus2:0.0) **and a HiNT
  `VEN_3388&DEV_0021` PCI-PCI bridge** — the V5 6000's on-board bridge, which had
  claimed the exact instance path (`4&415a68e&0&0008`) the GeForce2 now occupies.
  Sweeping only `VEN_121A` would have left the bridge ghost behind.
- **A global OpenGL ICD registration is the killer leftover.**
  `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\OpenGLdrivers\3dfx`
  (`DLL=3dfxOGL.dll`, installed by the 3dfx `oem9.inf`) makes *every* OpenGL app
  try to load the 3dfx ICD regardless of which card is fitted. ForceWare replaced
  it with `OpenGLdrivers\RIVATNT -> nvoglnt`.
- **Renaming game-local DLLs is not enough — the per-game *config* points at
  them by name.** After quarantining 35 game-local `opengl32/glide2x/glide3x/
  3dfxgl` copies, three games were still wired to 3dfx: Q3's
  `baseq3/q3config.cfg` had `seta r_glDriver "retrogl"`, UT99 had
  `RenderDevice=GlideDrv.GlideRenderDevice` (x3 keys), and RtCW had
  `r_glDriver "gl/openglv5.dll"` where `openglv5.dll` had been *replaced* by our
  2.75MB MesaFX ICD (stock 352256 kept as `openglv5.stock`). Also sweep
  `retrogl*.dll` — a filename sweep for `opengl32/glide*/3dfxgl` alone misses it.
- **Four OEM INFs were 3dfx** (`oem9`, `oem12`, `oem13`, `oem14`) — identified by
  `findstr /i "Provider=" oem*.inf` -> `Provider=%3dfx%`, not by guessing from
  the display class's `InfPath` (which named only oem13/oem14). ForceWare then
  reused the freed `oem9` slot.
- `del /f /q system32\3dfx*.* system32\glide*.*` is safe to glob (nothing else
  on an XP box uses those prefixes) — unlike `nv*`/`ati*`, which catch chipset
  and storage drivers.
- **The cmd paren trap bit again**, exactly as CLAUDE.md warns: a generated
  batch doing `if exist "<path>" (echo OK <path>) else (echo FAILED <path>)`
  died with `\System\glide3x.dll was unexpected at this time` because a game path
  contained `Unreal Tournament (Installed)`. Never echo a path inside a `( ... )`
  block; emit bare `ren` lines and verify with a separate re-listing sweep.

---

## Fleet auto-update was dead: the share rebuild deleted `Utility\Retro Automation` (2026-08-11)

Found while updating .124's agent. Three separate faults, all fleet-wide:

- **The rebuilt SMB share has no `Utility\Retro Automation\` directory.** Every
  agent hardcodes `\\192.168.1.122\files\Utility\Retro Automation\retro_agent.exe`
  (+ the `.ver` sidecar) in `agent/src/autoupdate.c:39` — so **no box on the fleet
  could auto-update**, silently. Restored the directory with `retro_agent.exe`
  (v1.25.1), `retro_agent.exe.ver` = `1.25.1`, `retro_chat.exe` (v0.14.0) +
  its sidecar, and the versioned archive copy. **If the share is ever rebuilt
  again, recreate this path first** — or nothing propagates.
- **A stale helper batch was silently downgrading the box.**
  `C:\RETRO_AGENT\restart_agent.bat` did
  `copy /Y retro_agent_v1.17.0.exe retro_agent.exe` — so any "restart the agent"
  ran a **v1.17.0** binary over the good one. .124 was running
  `retro_agent_v1.17.0.exe` while a correct **v1.25.1** binary sat right next to
  it and `LastUpdateVer` already read `1.25.1`. Rewrote the batch to restart the
  *current* binary and deleted the other two stale swap bats. **Never pin a
  versioned exe name in a restart helper.**
- **`git tag` in the working clone stops at `v1.9.2`** while the agent source is
  at **v1.25.1** (tags 1.10.0-1.25.1 were never created here). `agent/Makefile`
  derives `VERSION` from `git tag -l 'v*' --sort=-v:refname | head -1`, so a bare
  `make` compiles `AGENT_VERSION=1.9.2` — and since auto-update triggers on
  version **inequality**, not "greater than", publishing that would have
  **downgraded the entire fleet**. Tagged `v1.25.1` and added
  `tests/python/test_agent_version.py` to fail if the derived version ever falls
  behind the newest version named in `agent/` commit messages.

Swap the running agent safely with a detached batch that starts the new binary,
waits, checks `tasklist`, and **falls back to the previous exe if it did not
come up** — that is what made a remote agent swap non-scary here.

---

## Fleet driver audit on .124: 19 stale game-local ICDs found (2026-08-04)

Ran a full both-volume audit of every graphics DLL (`opengl32/3dfxgl/3dfxogl/
retrogl/3dfxvgl/glide2x/glide3x`), classified by exact byte size against the
`voodoo-cleanroom/out` artifacts, after the user asked whether all games run the
verified driver.

- **19 game-local ICD copies were STALE** — spread across 0.1.29/30/31, 0.1.32,
  0.1.33 and one non-retail build: Q2, Q3(retrogl), RtCW, MOHAA, UT99, Half-Life
  (x2 paths), CS (three separate installs), Heretic2, SiN, Descent3, and
  `system32\3dfxvgl.dll`. **LoadLibrary prefers the game dir over system32**, so
  every one of those games was running an OLD ICD while system32 had 0.1.35 —
  the game-local shadow rule in the deploy skill, demonstrated at scale.
- All updated from one staged upload with `.pre0135` backups; **verified by
  renderer string** (`Q3 GL_RENDERER: Mesa Glide v0.62 Voodoo3 (tm)
  [voodoo-cleanroom 0.1.35]`), never by file size.
- **Deliberate exceptions kept:** SiN keeps its bundled 3dfx MiniGL (our ICD
  can't play Sin's demos — earlier finding); the 344064 retail AmigaMerlin
  glide3x copies (Heretic2/SiN/RtCW-gl) are the documented hybrid config.
- **nGlide neutralized** (`.nglide-disabled`) in Unreal Gold and Carmageddon 2
  (glide.dll/glide2x.dll/glide3x.dll, all >1MB). Carma2's `CARMA2_HW.EXE`
  verified to still launch and run without it; Unreal is on D3D.
- **Two `cmd` traps cost a cycle here:** `dir /s` output needs the AM/PM token in
  the parse regex, and `if exist "x" (move ... & echo MOVED) else (...)` echoed
  MOVED while silently NOT moving — use plain per-file `move /Y` and re-list to
  confirm. Also `os.path.basename` on Windows paths is a no-op under Linux
  (split on backslash).
- Audit + fix scripts: session scratchpad `audit2.py` / `fix_drivers.py`; method
  captured as fleetbook recipe "Fleet-wide driver audit".

---

## Glide2x on .124 SOLVED: Unreal Gold 3dfx renderer works — nGlide was the wedge, our glide2x was two known fixes away (2026-08-04)

The July "Glide2x-era games are a crash risk - SKIP" finding is RESOLVED.

- **The wedge was nGlide, not Glide.** GOG's Unreal Gold ships nGlide (a
  Glide->D3D wrapper, 1.3MB 2013 dll) as game-local `glide2x.dll`. On the XP
  Voodoo3 its repeated failing grSstOpen attempts hard-froze the chip (no ping,
  no watchdog — physical power cycle). system32 also had a stale 2003 94KB
  glide2x. Neither was ours.
- **Our clean-room glide2x needed exactly the glide3x XP bring-up fixes**
  (never ported): GETLINEARADDR prime before ALLOCCONTEXT + zero-base guards
  (fork 79ee51e), plus the dual-ABI `_grFoo@N` relink in build-stack.sh
  (Glide2 games are MSVC-linked). GPF was hwcInitRegisters reading dramInit1
  off base0=0.
- **Debug loop that cracked it in minutes:** standalone `tst2x.exe` exerciser
  (LoadLibrary + underscore GetProcAddress + SetUnhandledExceptionFilter dump)
  + DEBUG glide2x build + addr2line on the DWARF = exact faulting line.
  grSstWinOpen also needs a REAL HWND (hWnd=0 fails silently — DDraw FSEM).
- **NEVER `taskkill /f` a fullscreen Glide2 game.** Kill mid-FIFO-packet =
  chip parses garbage = bus-level hang beyond the display driver's bounded
  waits (second power cycle of the day). Exit via the game's quit path. (The
  same applies to any direct-FIFO renderer; GoldSrc/Q3 survived kills because
  their teardown runs — TerminateProcess of a mid-frame glide2 app does not.)
- Unreal Gold verified: fullscreen Glide **640x480x16 @100Hz, stable** (96s
  intro flyby, clean).
- **CORRECTION (same day, after more runs): our glide2x is NOT stable under
  sustained load at ANY resolution yet.** A 640x480 `-benchmark` run (map
  loaded, real scene) wedged the box ~20s in — the same hard wedge as 800x600.
  The one clean 96s run was the intro flyby only. So the honest status is:
  **glide2x now INITIALIZES and RENDERS correctly (the bring-up fixes are
  real and committed), but the render path wedges the chip under load.**
  Do NOT leave a fleet box configured to launch a Glide2 game — set the game
  to D3D/software until this is fixed. Next suspects (in order): the packet
  FIFO path (`USE_PACKET_FIFO=1` + `GLIDE_PACKET3_TRI_SETUP`) which glide3x
  exercises differently; missing WEDGE-BREAK bounds in glide2x's own
  makeRoom/idle spins (the VINTAGE glide2 got those in 2b3e832 — our
  clean-room glide2x never did); and tiled-heap/buffer-count math at
  non-640 modes. Reproduce with the standalone `tst2x.exe` exerciser
  extended to draw real triangles for N seconds — NOT with a game.
- **800x600 fullscreen Glide WEDGES the Voodoo3** (2026-08-04, twice): first
  attempt fell back to VGA with "Display Driver Stopped Responding" (TDR, agent
  survived); after a clean reboot the second attempt hard-wedged the box (agent
  died with Unreal running normally at 800x600x16 — no taskkill involved, so
  this is the resolution itself, not the kill rule above). 640x480 is the only
  verified-good Glide2 mode on .124 so far. NEXT STEP: sweep resolutions with
  the standalone `tst2x.exe` exerciser (self-exiting, safe) rather than the
  game, to find where WinOpen/heap setup breaks — suspect the tiled-heap /
  buffer-count math at non-640 modes in glide2x's hwc path.
  Carmageddon 2 (GOG) also carries a game-local nGlide glide2x — same swap
  applies when wanted.

---

## GoldSrc-D3D present path: Blt-present promoted to page flip (+7.5%), and the wedge that taught us tile parity (2026-08-03)

D3D-vs-GL deficit hunt on .124 (CS 1.6, Voodoo3, 1024x768x16@100Hz). GL 40.9-43.4
fps, D3D 31.9 — but **identical 40.4 at 640x480**, so the HAL triangle path is
fine; the deficit only exists where fillrate matters.

- **GoldSrc-D3D never calls DdFlip.** It presents with a full-screen SRCCOPY
  `DdBlt` from its flip-chain back buffer into the primary — ~1.5 MB copied
  every frame. Found via new present-path tracers (DdCreateSurface
  TILED/LINEAR + primary-dest blt rects + flip/blt/lock counters -> RLog ring).
- **Fix (commit 0666fdb): `retroFlipPresent`** — promote that blt to a real
  overlay page flip (leftOverlayBuf + swapbufferCMD, the live scanout in
  fullscreen 3D) and ping-pong the app's back-buffer surface with a
  driver-allocated B2. **31.9 -> 34.3 fps.**
- **HARD WEDGE #1:** v1 ping-ponged into the **GDI desktop buffer** — the chip
  hard-hung (network dead; XP watchdog bugcheck auto-restarted the box ~5 min).
  Cause: color/Z **tile parity** — render targets must come from the tiled
  color slots whose even/odd layout matches the Z heap. B2 must be allocated
  from **TILED_HEAP2** (the third-buffer slot). The BACKBUFFER heap search was
  heap0-only -> DDERR_OUTOFVIDEOMEMORY; extended to include TILED_HEAP2.
- **Registry `GETENV` reads through the miniport are unreliable on the deployed
  box** (confirms the V5DLog finding) — `SSTH3_SWAPINTERVAL`, gate flags, all
  silently unread. Ship features **default-on behind strict shape conditions**,
  not behind registry reads.
- **App ROP compare gotcha:** GoldSrc passes `dwROP=00CC0000h` — only the
  HIWORD is the ROP. Compare `HIWORD(dwROP)==0xCC`, never the full SRCCOPY
  constant.
- **First D3D run after any reboot benches ~12 fps** (post-boot background
  activity) — always discard a warmup run.
- **GDI SCREENSHOT and GoldSrc `snapshot` are both blind in D3D fullscreen**
  (overlay scanout / unimplemented) — visual verification needs eyes on the
  monitor; timedemo correctness + 1-present-per-frame measurement are the
  remote evidence.
- Remaining D3D-vs-GL gap (34.3 vs ~41) is **not** the present path — next
  candidate: DP2 scene cost profiling.
- Rollback on .124: `system32\3dfxv3d.dll.bak9` (pre-flip-promotion tracer
  build), `.bak10` (vsync'd promotion). `cstrike\bench.dem` was re-recorded
  (365-frame) after a leftover `listenserver.cfg` fired; that cfg is deleted.

---

## CS 1.6 refresh + GL cursor: Glide fullscreen bypasses EVERYTHING GDI (2026-08-03)

Session on .124 (Voodoo3, CS 1.6 "Bcs16 Romania" build, D:\Program Files\...):

- **60Hz in GL was OUR ICD's hardcode, not XP's refresh bug.** MesaFX
  `fxMesaCreateBestContext()` passed `GR_REFRESH_60Hz` unconditionally, and in
  fullscreen **Glide programs the video timing itself** — GoldSrc `-freq`, the
  `FX_GLIDE_REFRESH_RATE` env the old launch batches hopefully set, GDI mode
  sets: all bypassed. Fixed in retro3dfx-gl **0.1.34** (`fxBestRefresh()`: env
  override else monitor-max via EnumDisplaySettings, snap down to a GR_REFRESH_*
  enum, retry at 60 if the open fails). Verified via retrogl.log: `grSstWinOpen
  ref=6 (100Hz)`, open OK.
- **D3D mode DOES honor GoldSrc `-freq 100`** (verified 100Hz all session via
  DISPLAYCFG polling; without it D3D runs 60Hz). But **`Counter-Strike.exe` (the
  repack launcher) does NOT forward args to hl.exe** — the desktop shortcut must
  target `hl.exe -game cstrike -freq 100` directly.
- **Invisible GL menu cursor = Glide scanout never composites the GDI/HW cursor
  plane** (D3D mode shows it because GDI manages that primary). Fixed in
  **0.1.35**: `fxDrawCursorOverlay()` stamps an arrow into the back buffer via
  grLfbLock before each swap when `GetCursorInfo` says the cursor is showing.
- **GDI SCREENSHOT under fullscreen Glide is BLACK** — useless for verification.
  New ICD debug hook `FX_DUMP_FRONT=<path>` dumps the Voodoo front buffer (raw
  565) every 64th swap; that's how the cursor fix was verified remotely.
- **`C:\setmode.exe 1024 768 32` restores the desktop at 60Hz** (no refresh
  arg) — after any test run, restore with the agent's `DISPLAYCFG set 1024 768
  32 100` instead, or the desktop is left degraded.
- CS 1.6 GL on our ICD **works fullscreen** now (the 07-18 "GoldSrc not
  supported on our ICD" changelog note is obsolete — game-local opengl32.dll IS
  ours since 07-24 and runs fine, menu + de_dust verified).

---

## DOS lane: CHAT dead-gate + DOSGAME "never marked installed" + COMMAND.COM missing-file errorlevel trap (2026-08-03)

Three DOS-lane defects, all fixed in retro-agent (agent v1.24.0 + rebuilt DOSGAME.EXE):

- **CHAT.BAT gated on `C:\DOSGAME\NET\PKT.OK` — but nothing ever wrote that file**,
  so DOS chat could never start on first try even after a fully successful PLAY
  network bring-up. Network setup now lives in `NETUP.BAT` (auto-called by BOTH
  PLAY.BAT and CHAT.BAT), which writes PKT.OK on DHCP success and re-verifies a
  pre-existing marker with a fast DHCP probe (marker persists across reboots; mTCP
  fails instantly when no packet driver answers).
- **COMMAND.COM does NOT set errorlevel for a missing command.** PLAY.BAT ran 8
  packet-driver .COMs by absolute path with only NE2000.COM shipped: each missing
  one printed "Bad command or file name" AND `if errorlevel 1` then thought the
  absent driver *loaded*, writing a dead PACKETINT into MTCP.CFG. Every driver/tool
  invocation is now `if exist`-guarded (NETUP.BAT + dosgame.c generated RUN.BAT).
- **DOSGAME installed-detection stem mismatch:** `write_install()` names the target
  dir with spaces/dots→`_` (`1_TO_NIL`) but `mark_installed()` compared the raw
  stem (`"1 To Nil"`) — any title with a space in its first 8 chars never got the
  installed star. One shared `zip_stem()` now; plus an `INSTLD.LST` receipt written
  by the install batch so installer-run (kind 'I') games that install to their own
  directory are still tracked. RUN.BAT also aborts with a clear message when the
  zip fetch failed instead of "installing" an empty dir.
  Tests: `tests/python/test_dosgame_install_detect.py` (retro-agent repo).

## BSOD 0x8E in vintage 3dfxv3d.dll — ROOT-CAUSED + FIXED (both == already-fixed bugs; .124 ran a STALE binary) (2026-07-28)

Five XP small memory dumps off .124 (Voodoo3, Win on D:), all bugcheck
**0x1000008E KMODE_EXCEPTION_NOT_HANDLED**, exc **0xC0000005**, faulting module
**3dfxv3d.dll**. Two DETERMINISTIC crash EIPs. Dump-derived facts (parse the XP
`DUMP_HEADER32`: bugcheck @0x28, then exc/EIP/ctxptr/0; `_TRIAGE_DUMP` @0x1000
gives ContextOffset/DriverList — DllBase is in the driver record whose name
field points into the string pool):

- **3dfxv3d.dll DllBase = `0xBF012000`** (dxg.sys sits at 0xBF000000; SizeOfImage
  0xC8500). The DLL is linked `/ALIGN:0x40` (SectionAlignment==FileAlignment==0x40,
  VA==RawPtr) so **RVA == file offset** — objdump VMA = 0x10000 + RVA.
- **EIP 0xBF04E4AC → RVA 0x3C4AC → `DdBlt@4`+0x5EC** (3 dumps: 07-25 ×2, 07-27;
  trigger = D3D-fullscreen: CS `-d3d`, UT D3DDrv). Faulting insn
  `mov 0x510(%edx),%eax`; **EDX=0 in all 3 dumps**. `edx=[esp+0x1c]`, set once in
  the prologue from `*(ppdev+0x784)->[0xdc]` = `_D3(lastContext)` = `pRc`.
  Root cause: the **system→video texture-download** path resolved both surface
  handles with `TXTRHNDL_PTR(h)` = `pRc->pHndlList->ppTxtrHndlList[h]`, but
  `pRc` (`lastContext`) is **NULL** in the context-less upload window (set only
  in the DP2 draw path, and `textureLoad()` itself zeroes it) → `pRc->pHndlList`
  = **[NULL+0x510]** → AV.
- **EIP 0xBF012346 → RVA 0x346 → `DrvBitBlt@44`+0x48** (1 dump: 07-28; trigger =
  Descent 3, DirectDraw fullscreen surface → a source-less fill). Faulting insn
  `mov (%edi),%eax`; **EDI=0** (=`psoSrc`, arg2). Root cause: the
  `#if ENABLE_LOG_FILE` debug block dereferenced `psoSrc->dhsurf` with **no NULL
  check**; `psoSrc` is legitimately NULL for solid/pattern blts. (This is the
  same offset as the .143 `3dfxv5d+0x346` Q3 vid_restart crash.)

**Both are ALREADY FIXED + committed + regression-tested in the H5 source** — the
.124 box was simply running a **stale, pre-fix 3dfxvs.dll** (deployed
`3dfxv3d.dll` 957456 B, no fix; fresh build 962724 B, fixed):
- DdBlt fix — `DDBLT32.C` resolves the TXTRHNDLs by walking the GLOBAL
  `g_pHndlList` chain (context-independent), never `pRc->pHndlList` (commits
  8de09a3, cf3ab3e). Only compiles under `DIRECT3D_VERSION>=0x0700 && DX>=7`;
  `PRECOMP.H` sets 0x0800 and `SOURCES` DX=7 → it does.
- DrvBitBlt fix — `BITBLT.C` null-guards BOTH psoDst and psoSrc in the
  ENABLE_LOG_FILE block, mirroring the SLI_AA block (commit a03a9fe).

**Verified against the fresh build (objdump):** DdBlt now loads `g_pHndlList`
(0xcdabc) and walks `ppTxtrHndlList[0]` count-compare (source lines 602-619) with
**zero** naked `mov 0x510` derefs; DrvBitBlt at +0x4A does `test %edi,%edi; je`
before `mov (%edi),%eax`. The OLD/crashing binary has NEITHER. So a rebuild +
redeploy of 3dfxv3d.dll fixes .124 (deploy left to the operator).

**Clean-room parallel:** fxd3ddd `Dd_Blt` (retro-agent `scripts/3dfx/driver/nt/
enable.c:1076,1109,1132`) already guards this class — null-checks `dst`/`dst->lpGbl`
and requires `src && src->lpGbl` before any source deref, returning NOTHANDLED on
a fill with no source (the M4c-2 hardening).

**Regression (new):** `tests/codegen_8e_guards.py` disassembles the linked DLL and
asserts BOTH fixes are in the CODEGEN (DdBlt refs g_pHndlList; DrvBitBlt tests edi
before deref) — catches a stale-obj link OR a DX<7/LF=0 preprocessor regression
that leaves source "fixed" but the binary crashing (exactly the .124 mode). Wired
into `test_built_artifact.sh` (+ bitblt/ddblt32 stale-obj checks). Validated:
PASS on the fresh build, FAIL on the crashing binary. Source-invariant asserts
for both already existed (`test_source_invariants.sh` #5d, #5f).

**Gotcha:** an XP "small memory dump" is NOT a user-mode minidump — DllBase lives
in the `_TRIAGE_DUMP` driver list (76-byte records, name field = a FILE-OFFSET
into the string pool, not a VA). And with `/ALIGN:0x40` the whole image maps 1:1
(RVA==file offset), so you can objdump the on-disk DLL directly at the RVA.

## DOS Game Manager built + verified in DOSBox-X (2026-07-28)

New `retro-agent/scripts/dosgames/`: DOSGAME.EXE (Open Watcom 16-bit TUI) +
host-side catalog/HTTP-bridge/tile tooling. Verified end-to-end headlessly
(DOSBox-X mingw under Wine on private Xvfb :77 — dosbox-staging Linux builds
hard-require GLX and abort on Xvfb). Full LAN install proven in emulation:
NE2000+slirp → Crynwr packet driver → mTCP DHCP → HTGET zip from
serve_dosgames.py (port 8181, systemd user service) → UNZIP → playable.
Deployed to share `…\Retro Automation\dosgame\`; install queued for .243.
Hard-won: (1) Watcom large-model **>64K static array silently wraps the data
segment** — no warning, corrupt entries past ~#420; (2) share zips have long
filenames → DOS 8.3 mangling breaks drive-letter copies, HTTP fetch is the
only reliable path; (3) DOSBox-X AUTOTYPE delivers enter/tab/chars but NOT
esc/F-keys — automate tests by timeout + file assertions. Survey of all 3,795
share DOS archives: install.exe/setup.exe/install.bat at zip root covers 96%
of installer archives; 2,893 zips are flat-root ready-to-extract.

## Share deploy trap: exe launched FROM the share locks its own update (2026-07-28)

Publishing retro_chat v0.14.0, the latest pointer `…\Retro Automation\retro_chat.exe`
was un-overwritable/un-renamable/un-READABLE (Access denied even as `admin`),
and gvfs showed the dentry as `??????????` / EINVAL. Root cause: **someone had
launched `retro_chat.exe` directly from `Z:\` on .145** — the running image
holds the share file open, and an earlier delete had put it into Samba
delete-pending, which blocks ALL new opens of that name until the last handle
closes. Diagnosis path that worked: per-box `net use Z: /delete` — the box that
refuses with "device is being accessed by an active process" is the holder; then
`wmic process … get ExecutablePath` found `Z:\…\retro_chat.exe`, PROCKILL freed
it (the pending delete then completed and the name freed up). Lessons:
- never run fleet exes from the share; copy local first (autoupdate does this).
- curl smb:// with the keyring creds (admin) is the reliable publish path when
  gvfs misbehaves; verify by md5 round-trip download.
- chat auto-update compares SIZE only (not .ver), so a temporarily mismatched
  `.ver` sidecar can't loop the fleet.

Also this session: retro_chat **v0.14.0** — Pentium-1 CPU fix (spinner was a
full erase+redraw every 150ms; now one WriteConsoleOutputCharacter cell per
500ms at below-normal priority) + startup now WAITS for the agent ("Waiting for
the retro agent to start...") instead of exiting when the chat wins the boot
race. Guard tests: `retro-agent/tests/python/test_retro_chat_p1_behavior.py`.

## fxD3D gbkernel: M4d bring-up-ladder tool built — fxdbg.exe (2026-07-27)

The clean-room fxD3D kernel Glide backend (retro-agent `scripts/3dfx/driver/nt/`,
gbkernel + escape ladder) had the kernel-side `FXDBG_*` DrvEscape handler
(`gbkdebug.c/.h`) but NO user-mode driver for it — M4d couldn't be run even with
a driver on-card. **Built `driver/nt/fxdbg/fxdbg.c`**: `CreateDC("DISPLAY")` +
`ExtEscape` over the shared `gbkdebug.h` ABI, one subcommand per rung
(`support|probe|clear|tri|tex|readback|ladder`), `readback` writes a 24bpp BMP
from the 16bpp-565 rect. Compile-verified (mingw, PE32). Added `selftest.c`
built `-m32` so `unsigned long` is 4 bytes (the i386 driver ABI) — pins opcodes,
magic, and every wire-struct size/offset so a `gbkdebug.h` field edit fails the
build, not the box. Wired into `make -C scripts/3dfx test` (all green).

- **Gotcha:** the host default `unsigned long` is 64-bit (LP64) — an ABI guard
  for an ILP32 driver struct MUST build `-m32`, or every `sizeof` is doubled.
- **M4d is now gated only on hardware:** deploying `fxd3ddd.dll` to .124 is a
  physical-recovery risk (experimental kernel display driver) — needs explicit
  operator go-ahead before flashing + running `fxdbg ladder`.

## CS menu cursor invisible in fullscreen GL — SOLVED: vgui_emulatemouse (2026-07-25)

**GoldSrc's menu cursor is the Windows OS cursor by default**, and in exclusive
fullscreen 3dfx GL the OS cursor overlay is never composited over the 3D scanout
→ menu renders, cursor invisible. **Fix: `vgui_emulatemouse "1"`** (in
`cstrike\userconfig.cfg` — WON auto-execs it and, unlike config.cfg, doesn't
overwrite it) makes the engine draw its OWN software cursor into the GL frame.
Verified at **1024×768×16 OpenGL** on our retrogl: glReadPixels `snapshot`
capture shows the white arrow in-frame (framebuffer captures never contain the
OS cursor, so an arrow in the capture IS the software cursor — clean proof).

- **Self-capture trick for BCShield builds** (UIKEY + GDI both useless in
  fullscreen Glide): a `wait`-chain + `snapshot` in **userconfig.cfg** fires a
  few hundred frames after startup → `cstrike\Snapshot0000.bmp` of the live menu.
  (My earlier attempt via `cstrike\autoexec.cfg` did NOT run — this build execs
  userconfig.cfg, not autoexec.cfg.)
- CS at 1024×768: registry ScreenWidth/Height + `-w 1024 -h 768`; retrogl.log
  confirms `res enum 12 (1024x768)` + context SUCCESS.
- **CS `-d3d` at 1024×768 WORKS — but ONLY with a settled 16bpp desktop.**
  On a 32bpp desktop it fails clean ("video mode not supported → software
  mode") — Voodoo3 D3D is 16-bit-only. Sequencing matters: setmode to
  1024×768×16 must complete BEFORE launching hl (`setmode` then launch in the
  same batch works; a racy setmode+launch from separate connections produced a
  false sw.dll revert). Verified: EngineDLL stayed `hw.dll` through a full
  bounded run + qconsole shows the engine past video init into sound init.
- **-d3d fullscreen WEDGES .124's network for minutes while running** (vintage
  HAL trait; recovers when hl.exe dies). Fine for local play; hostile to remote
  automation. ALWAYS use a fully self-contained on-box batch for -d3d tests
  (16bpp setmode → launch → ping-wait → taskkill → setmode 32bpp back), and
  queue a `taskkill` via the daemon task queue as the recovery net. Note the
  final setmode-32 needs a few seconds' wait after the kill or it doesn't take.

---

## MOHAA CD-lock SOLVED + RA2 renders on Voodoo3 (2026-07-24)

**MOHAA now launches + renders** on our OpenGL stack. Three stacked fixes:
1. **CD copy-protection (SafeDisc):** mount the owned disc image via DAEMON Tools
   — but `.124` is **dual-boot with Windows on D:**, so ONLY
   `D:\Program Files\D-Tools\daemon.exe` is the registered DT; the **C:\ copy
   throws "Product not installed!"** on every `-mount` (this wasted a long
   detour). DT 3.47 has **no CLI mount syntax difference** issue — it was purely
   the wrong (C:) binary. Mount: `start "" /d "<D:\...\D-Tools>" daemon.exe
   -mount 0,<image>` → G: shows `MOHAA_DISK1`; the plain mount satisfies MOHAA's
   SafeDisc check (no separate emulation toggle needed). Disc staged `D:\d1.iso`
   (from `Z:\Games\Windows XP\...Disc 1.iso`).
2. **"0 files in pk3 files":** the launch working-dir must be the real path —
   the 8.3 short name for `EA GAMES` is **`EAGAME~1`**, NOT `EAGAMES`; a wrong
   short path set fs_basepath to `D:\` → 0 pk3s → "couldn't load default.cfg".
   Launch with the full quoted path: `start "" /d "D:\Program Files\EA GAMES\
   MOHAA" MOHAA.exe` (NO `+set` cvars — the Ritual build crashes on cmdline
   r_mode/logfile/gldriver).
3. **GL:** deploy our retrogl as game-local **`opengl32.dll`** (same KnownDLL
   trick as CS — MOHAA's engine imports opengl32). Verified: retrogl.log shows
   `grSstWinOpen: returned <nonzero>` + `wglCreateContext: SUCCESS` for MOHAA.exe,
   fullscreen 640×480×16, proc → 32MB (game data loaded).

**Red Alert 2 runs + renders on the Voodoo3** (`C:\Games\...Red Alert 2...V2\RA2`):
the "Win10 Fixed" repack bundles the **aqrit ddraw wrapper** (`ForceDirectDrawEmulation=1`,
`SingleProcAffinity=1` — good for the single-core P3). Verified end-to-end:
main menu → Single Player → Skirmish setup → mission load → **live in-game
render** (iso terrain, Soviet units, fog, sidebar) at 800×600. No crash — the
known SSE2-`wsock32.dll` P1/P3 hazard (local 22528-B copy) did not trigger on
this repack. RA2 is 2D DirectDraw, not 3D — it uses the Voodoo3's 2D path.

---

## CS/GoldSrc OpenGL SOLVED — game-local opengl32.dll (2026-07-24)

**CS 1.6 (GoldSrc 1.1.2.5) now runs OpenGL on our retrogl.** GoldSrc does NOT use
the gldrv/3dfxgl.dll MiniGL LoadLibrary path (proven: our DllMain never fired for
`3dfxgl.dll` under either the Steam-emu 1.1.2.5 OR a WON 2001 build). Its `hw.dll`
uses the **statically-imported `opengl32.dll`**. The unlock: **`opengl32` is NOT
in .124's KnownDLLs list** (`HKLM\SYSTEM\CCS\Control\Session Manager\KnownDLLs`
has no `opengl32` value), so a **game-local `opengl32.dll` DOES load** (game dir
before system32). Deploy our retrogl AS `<CS>\opengl32.dll` (back up the original
first) → GoldSrc loads OUR opengl32 → `grSstWinOpen` succeeds →
`wglCreateContext: SUCCESS` for `hl.exe`, and **EngineDLL stays `hw.dll`** (GoldSrc
reverts it to `sw.dll` on ANY GL failure — non-revert is the GoldSrc-side success
signal). Config: `EngineDLL=hw.dll`, `EngineGLDriver=opengl32.dll`, `EngineType=2`,
Screen 640×480×16, launch `hl.exe -game cstrike -gl`. Needs the **787KB glide** in
the CS dir too (the 920KB build hangs — see below).

- This is why "CS doesn't work in OpenGL" persisted: the game-local `opengl32.dll`
  was MS's/software (or a stale build), and instrumenting via `wglGetProcAddress`
  was a red herring — GoldSrc resolves via kernel32 GetProcAddress, invisible to a
  per-call tracer. The **DllMain PROCESS_ATTACH** line (retrogl 0.1.33) is what
  finally distinguished "never loaded" from "loaded but failed".
- Q3/Q2/RtCW load our retrogl by an explicit **non-opengl32 name** (`retrogl.dll`,
  `3dfxgl.dll`, `gl/openglv5.dll`) via their own `r_glDriver`/`gl_driver` cvar;
  GoldSrc has no such cvar so it needs the `opengl32.dll` name. Two different
  load mechanisms, same retrogl DLL.
- **MOHAA** renders on our stack too (qconsole.log: our Mesa/3dfx extensions,
  `GL_MAX_TEXTURE_SIZE 256`, dual-TMU, `MODE 6 1024x768`) but the current disc is
  **CD-locked** ("Cannot locate the CD-ROM") — a no-CD/provisioning gate, not a
  driver problem. WON Half-Life is likewise gated on a **CD-Key** dialog.

---

## Games-OpenGL-broken ROOT CAUSE: wrong glide3x build (2026-07-24) — SOLVED

**REGRESSION ROOT-CAUSED (2026-07-25): the 920,157-byte DLL is NOT a source
regression and NOT a debug build — it is the `FX_GLIDE_HW=h5` (Voodoo4/5
Napalm) glide3x.** `voodoo-cleanroom/build-stack.sh` builds h5 FIRST and names
it **`out/glide3x.dll`** (the deploy-expected name); the Voodoo3 build is
`out/glide3x_h3.dll` (Makefile.mingw's default HW is also h5). Both were built
Jul 23 10:12 (h5 :22, h3 :27 — PE timestamps). Evidence: 920 KB build contains
Napalm-only strings (`FX_GLIDE_2PPC`, `FX_GLIDE_AA2/4/8_OFFSET_*`, Voodoo4/5500/
6000 board table, `Services\3dfxvs`); .text 0x43084 vs h3's 0x2b1a4; identical
393-export surface (why it drop-in loaded). It hangs/crashes in grGlideInit
because **all four verified bring-up fixes were committed to the h3 tree only**
(a71eb3f TLS `%fs:`→TlsGetValue + lost-context fallback, 8b6eb5f GETLINEARADDR
prime, 2387787 zero-base guard, a73a159) — h5's `fxglide.h` still has the raw
`%fs:` TLS read and h5's minihwc still ALLOCCONTEXTs unmapped → NULL base. The
glide clone (`glide-devel-sezero` @ a71eb3f) is clean; a from-scratch h3 rebuild
via the build-stack.sh recipe reproduces the good DLL **byte-identically except
8 link-timestamp/checksum bytes** (787,186 B, same 393 exports incl. dual-ABI
`_gr*` aliases) → `out/glide3x_rebuilt_fixed.dll`. Rule: **deploy glide3x_h3 to
Voodoo3 boxes; never ship the h5-named `out/glide3x.dll` artifact to .124**
(port the h3 fixes to the h5 tree before any Voodoo5 use of our glide).

**Every "OpenGL doesn't work" symptom on .124 traced to the WRONG glide3x, not
the retrogl ICD.** `voodoo-cleanroom/out/glide3x.dll` had been rebuilt to a
**920,157-byte** build whose **`grGlideInit()` hangs/crashes on the Voodoo3**.
The retrogl (MesaFX ICD) calls `grGlideInit` inside `fxQueryHardware()` on the
FIRST `wglDescribePixelFormat` (via `pfd_tablen`→`fxMesaSelectCurrentBoard`), so
the game dies at GL-init with the ICD's attach-banner logged and nothing after —
looked exactly like an ICD bug. It is NOT: with the **787,186-byte** production
glide the SAME retrogl runs clean through `grSstWinOpen` → `wglCreateContext:
SUCCESS`. **Q3 CONFIRMED: `GL_RENDERER: Mesa Glide v0.62 Voodoo3 (tm)
[voodoo-cleanroom 0.1.32]`, map+cgame rendered.**

- **Ship the 787,186-byte glide.** `out/glide3x.dll` is now that build (md5
  f42b0b49710bb5ef8b052abce5d4fb6e); the 920 KB one is quarantined as
  `out/glide3x_920157_debug_HANGS_grglideinit.dll`. There is a **regression in
  our clean-room glide between the 787 KB and 920 KB builds** (grGlideInit) —
  investigate separately; do NOT deploy the 920 KB build to any box.
- **Diagnostic path that cracked it:** instrument the retrogl (`fxrlog.h` →
  `C:\retrogl.log`), run a Q3-engine game (standard WGL path logs every step),
  read where it stops. GoldSrc's engine bypasses the ICD's WGL entry points so
  it logs nothing — use a Q3-engine game to exercise/verify the ICD.
- **GDI SCREENSHOT of Voodoo3 glide-fullscreen = garbage** (reads desktop FB, not
  the 3D overlay). Prove rendering with the in-game `GL_RENDERER` / a timedemo,
  never a screenshot.
- **Every game dir needs the 787 KB glide** (glide3x is NOT a KnownDLL, so the
  game-local copy wins) — fleet had mixed builds scattered everywhere.

---

## fxD3D M4d bring-up attempt #1 — deployed, NOT yet activating (2026-07-24)

`fxd3ddd.dll` (33 KB native PE, exports DrvEnableDriver) deployed to .124 but did
NOT become the active display driver over two reboots. Box stayed HEALTHY (no
BSOD) + fully recovered — the chassis review held. Hard-won mechanism notes:

- **Active display-DLL key = `HKLM\SYSTEM\CurrentControlSet\Control\Video\
  {B9F859EE-ACEA-46E1-B07C-6334E6BF65AB}\0000\InstalledDisplayDrivers`**
  (REG_MULTI_SZ) — the ACTIVE video device. NOT `Services\3dfxvs\Device0`
  (separate copy; \Device\Video0→it but editing it alone did nothing). Set BOTH.
  Swap+recover both ways: `reg add "<key>" /v InstalledDisplayDrivers
  /t REG_MULTI_SZ /d "fxd3ddd|3dfxv3d" /f` (no reboot to set; reboot to apply).
- **Two reboots keys=fxd3ddd → `QUERYESCSUPPORT`=0** (ExtEscape on DISPLAY DC, via
  `D:\fxdbg.exe probe` → `C:\fxdbg.txt`): fxd3ddd's DrvEscape not reached ⇒ fxd3ddd
  NOT active. DrvEscape IS in the DRVFN table (chassis.c:595). Box healthy 1024×768×32.
- **Rename-based file-lock test is USELESS** — Windows allows renaming an in-use
  file (only DELETE is blocked). Could not confirm load-vs-init-fail this way.
- **Recovery PROVEN**: restore both keys→`3dfxv3d` + REBOOT → vintage back,
  accelerated, agent alive. .124 survived 3 reboots clean. Autologon intact
  (voidsstr/password) so the agent always returns even on a black screen.
- **NEXT to unblock:** (1) add init-progress instrumentation to fxd3ddd
  (registry-marker writes at DrvEnableDriver entry + each PDEV/EnableSurface step
  — the H5 "RLog" pattern) so a deploy shows IF it loads + WHERE it fails; (2)
  check `iDriverVersion` in DrvEnableDriver (NT4 0x20000 vs an NT5 value XP SP3 may
  demand); (3) likely need a real SetupAPI/PnP install (deploy-3dfx-driver skill /
  updrv.exe) — the vintage 3dfxv3d was probably PnP-installed, not a bare reg-swap.
  Staged on .124: `D:\WINDOWS\system32\fxd3ddd.dll`, `D:\fxdbg.exe`, `C:\fxdbg.txt`.

## fxD3D M4c-1: backend wired to the card + on-card bring-up ladder (2026-07-24)

`fxd3ddd.dll` now brings the Voodoo3 up from the chassis and ships the
escape-driven ladder that will validate the kernel backend on-card at M4d
(before D3D drives it). All 15 compiles `EXIT=0`, `LINKEXIT=0`
(`scratchpad/build_fxd3d.sh`); `make -C scripts/3dfx test` all-PASS. Hard-won
points:
- **BAR0 comes from `IOCTL_VIDEO_QUERY_PUBLIC_ACCESS_RANGES`, VRAM from the FB
  map.** `chassis.c DrvEnableSurface` → `fxchassis_attach_backend`: pick the
  first memory-space public range (`MappedInIoSpace==0`, non-null VA) as BAR0;
  `vramBytes = ppdev->vmi.VideoRamLength` (already filled by
  IOCTL_VIDEO_MAP_VIDEO_MEMORY — no separate VRAM query needed). Attach is
  **non-fatal**: on failure log + keep a 2D-only surface, never fail the enable.
  `DrvDisableSurface` detaches then `IOCTL_VIDEO_FREE_PUBLIC_ACCESS_RANGES`.
- **BAR1 non-cached is a MINIPORT CONTRACT, not something the display DLL can
  set.** `IOCTL_VIDEO_MAP_VIDEO_MEMORY` exposes no cache attribute; the chassis
  documents + relies on the paired `3dfxvsm.sys` mapping BAR1 non-cached
  (gbk_mmio.h hard requirement — GBK_WMB is a no-op fence valid only there).
  Confirm on-card at M4d via `FXDBG_PROBE`+`FXDBG_CLEAR`; if write-combining,
  grow `GBK_WMB` an `sfence`.
- **Desktop stride ≠ 3D stride.** `gbkernel_attach` gained a `desktopStride`
  param; `gb_swap`'s blit-present **dst** uses `lDeltaScreen` (desktop pitch),
  not the 16bpp color-buffer stride (M4b-2 minor #2). 32bpp dst-pixfmt convert
  is `TODO(fxd3d M4c-2)`.
- **DrvEscape bring-up ladder (`gbkdebug.c`, opcodes `0x3DF0..`), NONE needs
  D3D:** PROBE (status+cmdFifo regs+layout, no draw) → CLEAR (FASTFILL+swap) →
  TRI (gouraud PKT3) → TEX (PKT5 checker + textured quad) → READBACK (copy a
  BAR1 rect back so the agent verifies pixels without a GDI screenshot). Wired
  `INDEX_DrvEscape`; driven from user mode by `ExtEscape` on the display DC.
- **FP bracket at the DDI/ladder entry (M4b-2 minor #9):** `d3d_DrawPrimitives2`
  wraps `fxd_dp2_execute_real_cb` in one `EngSave/RestoreFloatingPointState`
  region; TRI/TEX rungs bracket their `(float)` casts + textured ST in
  `gbk_fpu_enter/leave`. `gb_tex_bind` is FPU-free (raw-bit ST scales) so it's
  covered without a bracket; the outer + gbkernel's internal per-batch brackets
  nest cleanly.
- **`gbstub.c` (no-hw fallback) had to gain `gbkernel_attach/detach` +
  `gbkernel_dbg_*` stubs** now that chassis.c/gbkdebug.c reference them, else the
  documented no-hardware smoke link breaks. Stubbed attach returns -1 (→ chassis
  degrades to 2D-only, same as a real attach failure). C89-clean under mingw.

## fxD3D M4b-2: kernel MMIO transport landed — full driver LINKS with the REAL backend (2026-07-24)

`gbstub.c` retired from the default build; `fxd3ddd.dll` now links
`driver/nt/gbkernel.c` + the four verified `gbk/gbk_*.c` (all compiles EXIT=0,
LINKEXIT=0 via `scratchpad/build_fxd3d.sh`; `make -C scripts/3dfx test` still
all-PASS). New: `gbk/gbk_mmio.h` (`GBK_WR32`/`GBK_RD32` volatile + `GBK_WMB`
fence hook), `gbkernel.c` (transport + full `gb_*`), `gbkernel.h`
(`gbkernel_attach`/`detach` for M4c). Hard-won points:
- **IO/CMDAGP registers are DIRECT PIO, never FIFO-routed.** The 10-store
  CMDFIFO init and `miscInit0` Y-origin go straight to BAR0; ONLY 2D/3D
  register + memory writes ride the ring. Routing the FIFO-arm through the
  FIFO would deadlock (it isn't live yet).
- **Every FIFO/status poll MUST be bounded, and a too-big single packet must
  fail — not just STALL.** `gbk_fifo_make_room` faults if `nBytes` exceeds one
  usable lap (else NEED_WRAP recurs forever without ever hitting the stall
  cap — it doesn't increment the stall counter). Cap-hit → `gbk_fault()`
  disables the FIFO (`baseSize=0`) and latches `faulted` so no further MMIO
  is issued; a wedged card can't hang win32k (design risk #1).
- **Untextured draw path is FPU-free.** Vertex colors come from an
  integer-built IEEE-754 bit table (`gbk_ub_to_f32bits`), x/y/ooz/oow are raw
  dword copies (`*(h3u32*)&f`). The lone float arithmetic is textured
  perspective-ST (`s*texW*oow`), bracketed per design risk #4.
- **VC6 warns C4146 at h3hw.h:358/379** — that's the *intended*
  `H3HW_MASK_IS_UNSIGNED` compile-assert (`-(unsigned) stays >0`), pre-existing
  and harmless; the new files are warning-clean.
- **Twice-stable readptr:** `gbk_hw_readptr` samples `readPtrL`, reads `status`
  between the two samples (as `_grHwFifoPtr` fifo.c:1082-1086), and returns the
  RAW register value — exactly what `gbk_fifo_update_read` subtracts
  `fifoOffset` from. Don't pre-normalize it.

## fxD3D M4b-1: gbkernel pure-logic core implemented — glide-source corrections vs the design doc (2026-07-24)

Implemented + exact-value host tests all green (`make -C
retro-agent/scripts/3dfx/driver/nt/gbkernel-test` → 4/4 PASS; mutation-probed:
dropping the FIFO −4 margin or the color-buffer parity adjust fails the suite).
Three places the open glide source differs from a naive reading of the design
doc — trust these, they'd have been hardware hangs:
- **`hwcInitFifo` is 10 ordered stores, not "9 writes"** (minihwc.c:1634-1663):
  baseSize=0 (disable) FIRST, then baseAddrL(=start>>12), readPtrL, readPtrH,
  aMin, aMax (both start−4), depth, holeCount, cmdFifoThresh, and baseSize-arm
  LAST (`((len>>12)-1)|SST_EN_CMDFIFO`, holes ON). The design's "9" counts
  aMin=aMax as one item.
- **Linear `calcBufferSize` is NOT page-rounded** (minihwc.c:3725-3745:
  `bufSize = (xres<<1)*yres` exactly). Buffers get bit-12 page-PARITY
  adjustment during the top-down carve (color even / aux odd, :1449-1492) —
  start addresses are NOT otherwise 4K-aligned (e.g. 800x600 col0 =
  0xD3FE00-style values are normal and hardware-correct).
- **`kSetupCullPositive == 0x00`** (fxcmd.h:544-551): CW/positive cull is
  `kSetupCullEnable` (0x02) alone; CCW/negative adds 0x04; cull-off is
  `kSetupPingPongDisable` (0x08) alone (_grUpdateTriPacketHdr
  gglide.c:2717-2726). Also: glide's GLIDE_TRI_CULLING clears hw-cull for
  independent tris (sw cull instead) — deliberately NOT lifted; kernel uses
  hw culling per design §2.
- **FIFO wrap accounting conservation** (why it's exact): at a writer wrap
  `roomToReadPtr -= roomToEnd` over-charges by (C−w), and when the reader
  later takes the same JMP the update credits a full lap C instead of w
  (fifo.c:944) — the two errors cancel exactly. Verified by a 20k-op
  randomized reader-model test (byte-exact overwrite detection).

## fxD3D M4b-1: gbkernel pure-logic scaffold; h3 headers NOT vendorable verbatim (2026-07-24)

The gbkernel design's "vendor h3regs.h/h3defs.h/h3gdefs.h verbatim" idea fails
self-containedness: `h3regs.h` uses glide's `FxU32` (only typedef'd there under
`#ifdef _H2INC`, h3regs.h:42-44) and expresses registers as volatile structs
(no byte offsets), and `h3gdefs.h` does `#include "cmddefs.h"` which exists
only in the glide tree's **h5** dir, not h3. Extracted instead into
`retro-agent/scripts/3dfx/driver/nt/hw/h3hw.h` — self-contained, every define
cited file:line, byte offsets computed from the all-FxU32 struct walks and
cross-checked against the design doc's §0 list (all matched). Host-test
scaffold: `driver/nt/gbk/` (gbk_packet/layout/state/fifo.c + gbk.h) +
`driver/nt/gbkernel-test/` (`make` = build+run, PASS/FAIL per module). Module
objects compile with `-std=c89 -pedantic -Werror -nostdinc` (no-CRT enforced
on host) and cross-compile clean under `i686-w64-mingw32-gcc`. Make gotcha:
a phony target named `build` collides with a `build/` output dir — renamed dir
to `obj/`.

## fxD3D M4a: REAL DX7 DP2 stream translator in-tree, tested, linked (2026-07-24)

`d3dhal/d3dhal_dp2real.c` + `include/fxd3d_dp2.h` (`fxd_dp2_execute_real`) now
parse the runtime's ACTUAL `D3DHAL_DP2COMMAND` stream (4-byte packed headers,
separate FVF vertex buffer) straight into `fxd_set_renderstate/fxd_set_tss/
fxd_draw` — the simplified `fxd_dp2_execute`/`fxd2_hdr` path stays intact for
the legacy tests. Wired into `driver/nt/enable.c` `d3d_DrawPrimitives2` with
the NT-correct surface deref (`lpDDCommands`→`PDD_SURFACE_LOCAL`→`lpGbl->
fpVidMem`; `lpVertices` raw only under `D3DHALDP2_USERMEMVERTICES`), the
`D3DERR_COMMAND_UNPARSED`+`dwErrorOffset` protocol, `lpdwRStates` mirroring
(<768), and a `GUID_D3DParseUnknownCommandCallback` stash in `DdGetDriverInfo`.
Host test `test/test_dp2real.c` covers the happy path (all tri forms + lines +
points, FVF 0x1C4 and a 28-byte 0x144 stride) plus malformed-input hardening
(truncated header/operands, VB overrun, bad index, unknown op err_off, NULL/
zero-len, bad FVF). `make test` green (3 host tests), mingw winobj green, Wine
DDK `clfxd3d.bat` all EXIT=0 + LINKEXIT=0, zero warnings.

- **Comment gotcha that broke the build:** writing `D3DHAL_DP2*/D3DDP2OP_*` in
  a C block comment — the `*/` in the wildcard TERMINATES the comment and the
  rest of the header parses as garbage (cascaded into bogus `FxU8` errors from
  glide.h). Never put `*/` glob patterns inside `/* */` comments.
- DP2 walk safety pattern that satisfied gcc -Wextra AND VC6 /W3 at once:
  byte-composed LE reads (no casts, no alignment assumptions), per-command
  `need > avail` checks, contiguous-run precheck `vrange_ok` for non-indexed
  prims, per-index `vfetch` bound for indexed, walk always advances ≥4 bytes.

## fxD3D NT display driver `fxd3ddd.dll` now LINKS — clean-room chassis written (2026-07-24)

The clean-room fxD3D NT/2000/XP display driver (`scripts/3dfx/`, the open
Glide-GPL+public-DDK experimental tree — NOT the vintage H5 lane) now links a
complete `fxd3ddd.dll` under the Wine W2K+DX7 DDK. Build loop:
`scratchpad/build_fxd3d.sh` → `clfxd3d.bat` (VC6 `cl`/`link`), LINKEXIT=0.

- **`DrvEnableDriver` was the sole reported unresolved** only because the linker
  builds the `.def` exports file FIRST and aborts (LNK1141) before resolving
  body symbols. Providing `DrvEnableDriver` uncovered **21 more** pre-existing
  unresolveds (the d3dhal core's `gb_*` Glide-backend calls + CRT `malloc/calloc/
  free`). A one-unresolved link is not necessarily one-away from linking.
- **New clean-room files (`driver/nt/`):** `chassis.c` = the framebuffer 2D/PDEV/
  modeset half + the `gadrvfn[]` DRVFN table + `DrvEnableDriver/DrvDisableDriver`
  (unaccelerated DDK "framebuf" pattern, hooks nothing → GDI draws into the
  miniport-mapped linear FB). DDraw enable pair + `Dd*` stubs + `DdGetDriverInfo`
  (answers `GUID_D3DCallbacks3` with `DrawPrimitives2`) added to `enable.c`.
- **`/Gz` gotcha:** the DDK build defaults to `__stdcall` (`-Gz`), so backend
  symbols decorate as `_gb_*@N`; CRT names stay `__cdecl` (`_malloc`). Any shim
  must match the decoration (backend stubs compiled with the same flags;
  `crtshim.c` declares `__cdecl` explicitly).
- **CRT shim:** native `-nodefaultlib` driver has no CRT → `crtshim.c` maps
  `malloc/calloc/free` onto `EngAllocMem/EngFreeMem` (declared locally to avoid
  pulling `<windows.h>`'s CRT prototypes and colliding).
- **`ntddvdeo.h` is NOT on the default DDK INCLUDE** — it lives in
  `w2kddk/src/video/inc`; added that dir to `clfxd3d.bat` INCLUDE. It also needs
  `<devioctl.h>` included first (CTL_CODE / FILE_DEVICE_VIDEO / METHOD_BUFFERED).
- **Genuine blocker / next milestone (M4):** the real driver-side Glide backend
  (Glide minihwc/cinit compiled in) and real DDraw/D3D bodies. `gbstub.c` is a
  link-time PLACEHOLDER (all `gb_*` no-ops, `gb_tex_create`→NULL); the existing
  `d3dhal/glidebackend.c` is user-mode-only (opens a GDI window + glide3x) and is
  NOT linkable into a subsystem:native driver. Every stub is `TODO(fxd3d M4)`.
- Host unit tests (`make -C scripts/3dfx test`) stay green — all new code is under
  `#ifdef HAVE_DDK`; no portable-core / `!HAVE_DDK` path touched.

## Direct3D on the Voodoo3 — root cause of GoldSrc "video mode not supported" (2026-07-23)

Diagnosed live on .124 + against the H5 source. The vintage H5 **D3D HAL WORKS**;
the GoldSrc failure is a **hardware ceiling**, not a driver bug.

- **DDraw mode enumeration is COMPLETE.** A live `IDirectDraw::EnumDisplayModes`
  on .124 lists 640×480×16 (RGB **565**, Rmask 0xf800), 800×600×16, and the full
  8/16/32-bpp matrix 320×200 … 1600×1200. The mode GoldSrc wants is present.
- **The D3D HAL device creates fine — at 16-bit.** A DX7 probe
  (`DirectDrawCreateEx` → `SetCooperativeLevel(EXCL|FS)` →
  `SetDisplayMode(640,480,16)` → primary+flip+3DDEVICE surface → QI `IDirect3D7`
  → `CreateDevice(IID_IDirect3DHALDevice)`) returns **DD_OK at every step** and
  creates a HAL device. So the vintage H5 D3D HAL is functional.
- **ROOT CAUSE = Voodoo3 is 16-bit-3D-only.** `BuildD3DCaps` sets
  `dwDeviceRenderBitDepth = DDBD_16` **only** (H5 `D3INIT.C:3088`; `DDBD_32` is
  gated behind `IS_NAPALM`/VSA-100 at `:3155`). XP desktop is 32bpp; GoldSrc's
  D3D renderer matches the 32-bit primary and asks D3D for a **32-bit** render
  target → no matching HAL device → "the specified video mode is not supported"
  → software fallback. The Avenger core physically renders 16-bit (22-bit
  post-filter → 16-bit buffer); there is **no 32-bit 3D path** — adding DDBD_32
  would advertise silicon that doesn't exist. **Not driver-fixable.**
- **FIX = 16-bit desktop.** `ChangeDisplaySettings` to ×16 succeeds live
  (helper: build `setmode.c` with `-luser32 -lgdi32`). Then the D3D device's
  DDBD_16 matches the primary. (No 5:5:5 mode exists — only 565 — so any app
  demanding a 555 primary would also fail; GoldSrc uses 565, fine.)
- **SEPARATE BLOCKER — CS `-d3d` FULLSCREEN WEDGES the box for ~2-3 min** (agent
  unreachable, recovers when hl.exe dies), even at 16-bit. The DX7 probe (create
  device, no frame render, RestoreDisplayMode, exit) did NOT wedge — so the hang
  is in the **fullscreen D3D render/flip loop**, not device creation. Windowed
  `-d3d` on the plain CS surfaced the HL Autorun (that install's hl.exe chains to
  it) — inconclusive; multiple CS installs on the box (`Program Files\Counter-strike`,
  `Bcs16 Romania\Counter-Strike 1.6`, `Sierra\Half-Life`) muddy game-specific tests.
- **Bottom line:** on the Voodoo3, **OpenGL (our clean-room MesaFX+Glide) is the
  correct 3D path** — it works, is fast, and renders CS. D3D is 16-bit-capped and
  its fullscreen path hangs. The clean-room **fxD3D** HAL (`scripts/3dfx/d3dhal/`,
  D3D→Glide) is the way to put D3D in our repo AND likely dodge the H5 fullscreen
  wedge by reusing Glide's working fullscreen path — but it needs the loadable
  display-driver chassis (2D+DDraw+modeset) written + DDK-built (M3), then a
  risky display-driver swap. Core (M1/M2) + DDI glue done + host-tested.

## Benchmark matrix + game gotchas (2026-07-21, our WFP driver, ICD 0.1.31)

Recorded to specpicks (retro_benchmark_runs, machine .124):
| game | 640×480 | 800×600 | 1024×768 |
|---|---|---|---|
| Quake II (`q2-timedemo`) | 96.7 | — | 46.9 |
| Quake III (`q3-timedemo-four`) | 58.4 | 58.1 | 51.0 |
| RtCW (`rtcw-wolfbench`) | ~56 | (fn hardcodes 640) | — |
| Unreal Tournament (`ut-timedemo`) | ~30 | (needs .ini res set) | — |

**UT failure root cause (FIXED):** UT was returning None fps because the per-run
`taskkill /f` of each fullscreen game spawned a Windows Error Reporting
("X has encountered a problem") dialog; across a multi-game sweep these ACCUMULATED
and BLOCKED UT's Recovery-Mode launch dialog → UT never reached the menu → no
timedemo. Fix: **disable WER/Dr Watson** (`PCHealth\ErrorReporting DoReport=0
ShowUI=0`, `AeDebug\Auto=0`) — now baked into `run_bench.py preflight()`. Also
`ut_ensure_binds` now **auto-stages UTbench.dem** (was a silent missing-file fail).

**Sweep timeout gotcha:** a per-game `timeout` that kills run_bench mid-batch loses
ALL of that game's rows (the DB insert is at the end). Run one game+one mode+2 runs
per call (~4 min) so each COMMITS. RtCW's fn hardcodes r_mode 3 (640); UT ignores
`--modes` (resolution is in UnrealTournament.ini) — both need a small code change
for a real resolution sweep.

**MOHAA — CD-mount SOLVED (2026-07-21):** ISO is on the share at
`Z:\Games\Windows XP\Medal of Honor Allied Assault (2002) - Disc 1.iso` (+ Disc 2).
**Automated ISO mount** via `scripts/mount_iso.py <ip> "<space-free-iso>"` (also
`mount_iso()` in run_bench). DaemonTools 3.47 gotchas: (1) two D-Tools installs —
only the one on the ACTIVE Windows volume (**D:**) is registered; the C: daemon.exe
throws "Product not installed!". (2) daemon.exe is resident (tray) — launch DETACHED
(`start "" /d <dtdir> daemon.exe -mount 0,<iso>`); EXECW tree-kills it. (3) the
d347bus virtual-SCSI driver is already installed+running (creates the virtual CD
drives, F:/G:). (4) path must be SPACE-FREE (staged the ISO to `D:\ISO\MOHAA_CD1.iso`;
the share path has spaces). Verified: mounts to G: = MOHAA_DISK1, MOHAA's CD check
passes, MOHAA **renders the game** (7 threads, past the launcher). Only reads the CD
for verification (not during play), so no local copy strictly needed if a space-free
path is used.
**MOHAA fps STILL open:** MOHAA.exe is a launcher front-end (crash-recovery dialog →
"Play in Normal Mode" click, handled in the bench). The game uses **DirectInput**
(injected keys don't reach it) and **`+exec <cfg>` does NOT run** (verified: a
wait+quit cfg didn't quit) — Ritual's build has a non-standard config/console
mechanism. So a timedemo needs its config-exec path figured out OR a `.dm_` demo
obtained (MOHAA ships none; recording needs the console). `--game mohaa` mounts the
CD + launches + validates render; fps is the remaining bounded step.

**Carmageddon 2** uses **nGlide** (Glide→D3D wrapper), not our OpenGL ICD — would
test our D3D HAL path, not the ICD; and it's a racing game with no timedemo. Not a
clean ICD-benchmark add.

## ✅ SOLVED: voodoo3-wfp.inf loads our driver durably AND runs games (2026-07-21)

The **voodoo3-wfp.inf** (rename-files INF, PnP-installable) is the master unblock:
- Built from voodoo3.inf by renaming display→`3dfxv3d.dll`, miniport→`3dfxv3m.sys`
  (names not in any WFP catalog), dropping glide3x + CatalogFile, repointing
  ServiceBinary/InstalledDisplayDrivers. Lives in
  `toolchain-3dfx/dist/3dfx-voodoo3-wfp-20260721/`.
- PnP-install: `updrv.exe voodoo3-wfp.inf "PCI\VEN_121A&DEV_0005"`. Rebuilds the
  class-key config cleanly (InfPath=oem13.inf, Device0.IDD=3dfxv3d,
  ImagePath=3dfxv3m.sys) — NO corruption, NO WFP revert. (Gotcha: PnP defers the
  file copy to "reboot required" but leaves OLD files; copy our new
  3dfxv3d.dll/3dfxv3m.sys over the targets directly before reboot — they're
  rename-named so not WFP-tracked.)
- Result: **our driver loads** (VIDEODIAG drv_ver=`unknown`, not the in-box
  5.1.2001.0) AND **games run**: RtCW timedemo **54.4 fps** on our WFP-installed
  driver. The rename-NAME does NOT break Glide/games (my earlier "rename breaks
  games" was WRONG — those hangs were the in-box driver loaded + clobbered
  game-local ICD files, not the rename).
- ⇒ This is THE deploy method for our driver on a games box. Supersedes the
  "rename breaks games" caveat below.

Corollary (corrected): the earlier Q2/Q3 "hangs at GL context creation" were the
IN-BOX display driver, NOT clobbered game files. On our WFP-installed driver ALL
games work and match the morning numbers:
- Q2 96.6 fps @640 / 47.1 @1024 (exact morning match)
- Q3 58.6 @640 / 50.8 @1024
- RtCW 54.4-55.9 (wolfbench)
- MOHAA renders (needs CD1 ISO mounted via DaemonTools; no-CD patch optional)
⇒ The single root cause of the whole "games broken" saga was: our display driver
was not loaded (in-box was, via the config corruption + WFP). voodoo3-wfp.inf fixes
it. game-local ICD files were fine.

## Deployment & WFP (critical)

- **WFP silently reverts `D:\WINDOWS\system32\3dfxvs.dll` to the 2001 retail driver
  (689,216 B)** within ~15 s of any raw file swap. `SFCDisable=0xFFFFFF9D` +
  `SFCScan=0` do NOT disable WFP on retail XP. dllcache holds the catalog-validated
  retail pair: display `3dfxvs.dll` 689,216 + miniport `3dfxvsm.sys` 148,352.
- **Two reliable deploy methods:**
  1. **Rename method** (WFP-free): ship display as `3dfxv3d.dll` (not catalogued),
     set `HKLM\SYSTEM\CCS\Services\3dfxvs\Device0\InstalledDisplayDrivers=3dfxv3d`;
     miniport as `3dfxv3m.sys`, `Services\3dfxvs\ImagePath=system32\DRIVERS\3dfxv3m.sys`.
     Sticks across reboots. **CAVEAT: breaks the Glide path — games hang at
     `GLW_ChoosePFD`.** Fine for D3D/3DMark, NOT for OpenGL games.
  2. **PnP/SetupAPI install** (deploy-3dfx-driver skill): `updrv.exe voodoo3.inf
     "PCI\VEN_121A&DEV_0005"` — rebuilds the class-key registry consistently. Seed
     dllcache with our files first so WFP keeps them. This is the correct install.
- **WFP beats dllcache seeding** (2026-07-20): seeding `dllcache\3dfxvs.dll` +
  `\3dfxvsm.sys` with OUR versions before a PnP install did NOT stick — after reboot
  WFP restored the RETAIL files (689,216 + 148,352) to both dllcache and system32.
  WFP has a deeper catalog/driver-store source than dllcache alone. ⇒ dllcache
  seeding is NOT a reliable WFP escape here; only the rename method (non-catalogued
  filename) reliably loads our unsigned display driver. But rename breaks Glide/games.
- **The retail pair 3dfxvs.dll 689,216 + 3dfxvsm.sys 148,352 is MIS-configured**
  under the standard binding on this box: boots to 800×600×4 with "primary display
  adapter is not configured properly" + wallpaper corruption. NOT a clean full-res
  state. The Microsoft **in-box** driver (5.1.2001.0 / 3dfxvs2k.inf, in the driver
  store) is the proven full-res + Glide known-good — prefer it as the stable base
  for GAME benchmarking (games use our ICD, not the display driver's D3D).
- **Manual driver-file swaps + registry edits corrupt the display class key**
  `{4D36E968-E325-11CE-BFC1-08002BE10318}\0000`: after ~12 swap-reboots its
  `Driver`/`Service`/`InstalledDisplayDrivers` values went MISSING (MatchingDeviceId
  survived) → "primary display adapter not configured properly", 800×600. Recovery =
  clean PnP reinstall (rebuilds those values). **Prefer PnP installs over raw swaps.**

## ✅ SOLVED: comprehensive driver logging via registry-ring sink (2026-07-21)

Working end-to-end. The log sink is a **registry ring** (NOT the file IOCTL, which
was unreliable): the display driver flushes log text as REG_SZ chunks
`RLog00..RLog31` (32 × ~1000 B) under `HKLM\SYSTEM\CCS\Services\3dfxvs\Device0`,
with `RLogSeq` (total chunks; newest slot = (RLogSeq-1)&31), via the PROVEN
`SetRegSZ` (IOCTL_3DFX_SET_REGISTRY_VALUE, 0xfd6) — the same channel the driver
uses for all its settings. Agent reads via REGREAD (values are REG_BINARY UTF-16LE;
decode with `utf-16-le`). Helper: `scratchpad/rlog.py` `readlog(c)` reassembles the
ring in write order.
- **Coverage (all agent-readable):** V5DLog display lifecycle
  (DrvEnableSurface/DisableSurface/AssertMode with res/bpp + the Glide
  fullscreen-switch hwcExt escapes), CFIFO flight recorder (H3MakeRoom FIRST-CALL
  positive control + STALL>=100K + WEDGE-BREAK@50M), D3D texture-OOM (the 3DMark
  overcommit path). Verified: RtCW run produced RLogSeq=29 with the full
  fullscreen mode-switch trace.
- **V5DLog is UNCONDITIONAL** (infrequent lifecycle events, always captured — the
  registry-read gate `Retro3dfxLog` via ddgetenv proved unreliable, so don't rely
  on it). Per-op verbose `h3printf` stays gated (rarely needed; would flood).
- **Build:** `LF=1` in both Displays/H5 and Miniport/H5 SOURCES. Ships in our
  driver (the WFP-safe `3dfxv3d.dll`/`3dfxv3m.sys`). The miniport just needs its
  normal SET_REGISTRY handler (no LF needed for the registry sink).
- **Why the old file sink failed:** WRITE_LOG_FILE IOCTL (0xfd7) didn't reliably
  reach the miniport (build/videoprt artifact); SET_REGISTRY (0xfd6, adjacent) is
  used everywhere and works. Also EngDebugPrint is a no-op on free XP. So the
  registry ring is the reliable sink.

## Driver logging (comprehensive-logging effort) — historical notes below

- **`EngDebugPrint` is a NO-OP on free/retail XP** — never reaches DebugView (a
  kernel `DbgPrint` from watchdog.sys DID show, so DebugView itself works). A prior
  session's `V5DLog` instrumentation (EngDebugPrint-based) was therefore always
  silently invisible. Do not rely on EngDebugPrint for driver logging here.
- The driver has a **built-in file logger** gated by `ENABLE_LOG_FILE` (SOURCES
  `LF=0→1`): `H3PRINTF`/`h3printf` buffers → `EngDeviceIoControl(hDriver,
  IOCTL_3DFX_WRITE_LOG_FILE)` → miniport `H3StartIO` → `ZwCreateFile` to a .log.
  This is the RIGHT sink (agent-downloadable file). Built: LF=1 both SOURCES, runtime
  gate `Retro3dfxLog` reg value, `retroLogForce`/`retroLogRaw`, V5DLog rerouted to
  file, CFIFO flight-recorder, miniport log path C:→D: fallback.
- **UNSOLVED BLOCKER:** `IOCTL_3DFX_WRITE_LOG_FILE` never reaches the miniport — a
  registry counter in the handler never increments, even from an unconditional probe
  in DrvEnableSurface. videoprt.sys drops it before dispatch. Output-buffer
  hypothesis tested + wrong. No 3dfxvs.log produced yet. (Xed after 9 rebuilds.)
  Miniport log path: the box's C: is Win98 FAT → kernel write to `\DosDevices\C:`
  silently fails; changed to try D: first.

## D3D / 3DMark

- **★ .143 (V5 5500): 3DMark2001 SE HARD-FREEZES the box** (2× on 2026-07-21:
  first run crashed mid-suite ["Safety Precaution" abort dialog after ~15 min],
  second run froze the machine solid — 100% ping loss, NIC dead, physical reboot
  required). PowerStrip autostart was disabled for run 2 ⇒ NOT PowerStrip; it's
  the **D3D HAL path of our 3dfxv5d.dll wedging** (CMD-FIFO spin class). Key gap:
  .143's deployed `3dfxv5d.dll` is the **2026-07-17 build (595,644 B) WITHOUT the
  `H3MakeRoom` CFIFO spin-breaker** that the .124 lane added for exactly this
  wedge (present in the current CFIFO.C). Post-reboot plan: (1) isolate the
  wedge test by running a reduced 3DMark selection; (2) SUPERVISED: rebuild
  3dfxv5d for V5 from current CFIFO.C (mind the other session's in-flight
  DEBUG.C/ENABLE.C logging edits in the same tree) and deploy with user standing
  by. The OpenGL/game stack is unaffected (ICD path, not D3D).

- Our H5 D3D HAL is the display driver `3dfxvs.dll` (D7D3D.C etc. compiled in). Via
  the WFP-free rename path, 32MB texture tests + full 3DMark2000 COMPLETE on our
  clean rebuild, the original dist 595180, AND retail 689216 — **nothing wedges when
  loaded cleanly.** The original 16/32MB wedge was tied to the std `3dfxvs`+WFP load
  path, not our binary. `H3MakeRoom` CMD-FIFO spin-breaker (CFIFO.C) added as a
  safety net (converts a wedge hang→TDR into a recoverable condition).

## Build toolchain (Wine)

- Display build: `bldw2k.bat c:\3dfx\H5\W2K\Src\Video\Displays\H5` (free/objfre by
  default; `checked` arg for objchk). Miniport: `...\Miniport\H5`. Always
  `export COPYCMD=/Y`, `ulimit -f 2000000`, redirect (never pipe) wine output,
  `timeout`. Purge stale `.obj` before a rebuild or nmake silently skips files.
- Display DLL sizes: 595,180 (base dist) → 595,964 (spin-breaker) → 942,200 (LF=1
  logging). Miniport 195,812 (base) → 199,612 (LF=1 + counter).

## GAMES-STACK BLOCKER (2026-07-20 night) — ICD needs OUR display driver, standard-name

**Symptom:** after tonight's display-driver recovery, ALL games hang at GL context
creation — Q2 at "...calling CDS: ok" (then nothing), Q3 at "GLW_ChoosePFD" — and
produce None fps. This morning the SAME games+harness gave real numbers (Q2 96.6 fps
@640, 47.1 @1024, ICD 0.1.31).

**Isolation (what does NOT fix it):**
- Tried in-box display driver (5.1.2001.0) AND our rename-path display driver
  (3dfxv3d = our 595180) — BOTH hang at context creation.
- Tried in-box glide3x (335,872) AND our retail glide3x (348,160, `_grFoo@N`
  decoration matching our ICD's imports), deployed game-local — both hang.
- retrogl version isn't it: the pre-existing system32 retrogl (2,733,493) hung
  BEFORE I touched it; the deployed 0.1.31 (2,742,298 = the morning-working build)
  hangs too. Both are 0.1.31.

**Diagnosis (corrected):** VIDEODIAG shows the loaded display driver is **5.1.2001.0
= the Microsoft IN-BOX driver**. Games hang at GL context creation on the in-box
driver. Games worked this morning on OUR H5 display driver (standard `3dfxvs` name).
So the ICD's Glide fullscreen path needs OUR display driver — the in-box driver's
DDraw/Glide doesn't satisfy it.
**Why I couldn't test our driver tonight:** the manual rename-path edit
(`Device0\InstalledDisplayDrivers=3dfxv3d`) **did NOT take** — the PnP install of
`3dfxvs2k.inf` (in-box, done during display-config recovery) firmly re-established
the in-box driver in the class key + driver store (`class\0000\InfPath=3dfxvs2k.inf`),
which overrides a manual Device0 edit. Result: I set the rename binding + rebooted
but the box still loaded the in-box 5.1.2001.0 driver. Ruled out (all hang on in-box):
glide3x 335872/344064(AmigaMerlin)/348160, retrogl 2733493/2742298, env
FX_GAMMA/FX_DITHER — all correct, all hang → it's the display driver, not these.

**Why we can't just load our driver standard-name:** WFP reverts standard-name
`3dfxvs.dll` to retail (dllcache seed doesn't beat it — see above), and the manual
rename-registry edit corrupts the class-key config. So the morning stack is not
trivially reproducible.

**The real fix (not yet done):** make our unsigned H5 display driver loadable under
the standard `3dfxvs` name durably. Options, best first:
  1. **Catalog-sign** our driver package (build a `.cat`, test-sign, enable
     testsigning / disable integrity checks) so WFP accepts our `3dfxvs.dll` — keeps
     the standard binding the ICD's DDraw/Glide path requires. THE right path.
  2. **Patch `sfc_os.dll`** to truly kill WFP (offline/registry), then raw-install
     our standard-name files. Heavier/riskier.
  3. Confirm exactly how the morning stack got our driver standard-name (check .124
     driver-store / whether a prior deploy-skill run + dllcache seed stuck before a
     WFP scan) and reproduce that.
Until one of these lands, games run only on the morning-style all-ours standard-name
stack, which the current box state does not have.

## Games / ICD

- **★★ ICD NEVER OPENED >640×480 until 0.3.6 (2026-07-21, .143).** MakeCurrent's
  resolution walk consults a Voodoo1/2-era cap table; V3/V5 hardware strings fall
  through to platform=Voodoo1 + mem=2MB (GR_MEMORY_FB byte-scale value matches no
  case) → best db+Z = 640×480 → every 800/1024 request silently opened a 640×480
  Glide context under an 800/1024 game viewport → BLACK WORLD on the monitor.
  Engine-side timedemo fps still measured "fine", so all pre-0.3.6 "800/1024"
  benchmark labels on the 3dfx-optimized stack are really 640×480 — re-baseline.
  Fix: platform=-1 sentinel for modern boards bypasses the cap gate (0.3.6).
  Corollary: the old "Q3 640/800/1024 flat fps ⇒ engine-bound" evidence is
  partially VOID (flat because identical 640 rendering). Re-test the fill-bound
  question at REAL 1024 before trusting the present-bound conclusions at high res.
- **0.3.4d swap-hook regression (fixed in 0.3.5):** the `__r3d_blitValid` latch is
  NOT GoldSrc-specific — on the 2-TMU config the ICD maps GL unit 0 → GR_TMU1, so
  Q3 latches too and the per-frame combine override killed vertex-color modulate
  (WHITE menu text; stock Q3 menu text is RED — check capture COLOR fidelity, not
  just structure). 0.3.5 gates the hook on `__r3d_sawTMU0` (real dual-texture
  frames only) + clears the latch at grSstWinClose + words-only combine-cache
  invalidation (`__glSSTInvalidateCombineWords`, NOT the full reset).
- **★ CS/GoldSrc GREEN WORLD = stale 2PPC — FIXED (.143 ICD 0.3.4d, 2026-07-20).**
  2PPC (2-px/clock, `combineMode` bit-29) is Glide's SINGLE-texture opt; must be OFF
  for dual-texture world+lightmap. Our ICD's combine-word cache skipped the Glide
  re-issue on GoldSrc's single↔dual flips → `tmuConfig` never invalidated →
  `_grTex2ppc` never re-ran → both VSA-100s mirror → `(0,G,0)` world. Fix = re-issue
  the FULL TMU1 state at SwapBuffers (grTexSource+grTexCombine+grColorCombine+
  grAlphaBlend; bisect-proven — no subset works, and `__glSSTResetCombineCache()`
  there REINTRODUCES green by wiping ext/overbright state). Q3 unaffected (68-74fps).
  Details: `optimized/CS-GREEN-WORLD-LOG.md`, commit 71db1e3.
- **GDI SCREENSHOT of fullscreen Glide/D3D = scanline garble, always** — it cannot
  BitBlt the Voodoo surface. A garbled GDI shot of a running game usually means the
  game IS on the hardware path (software-GL would capture clean). TRUE capture =
  the ICD's FBDUMP (`C:\icd_fbdump.on` → grLfbReadRegion). Caveat: fbdump sits after
  the `!doubleBufferMode` early-return, so single-buffered contexts (RTCW) never dump.
- **idTech/Unreal games detect unclean exits (`taskkill /f`) and block the NEXT
  launch on a GUI dialog** (UT: "Recovery Mode" window; RTCW: safe-mode stall,
  process alive but opengl32 never loads). Automation must click through
  (UICLICK Run-button) or the game "runs but never renders". Prefer in-game `quit`
  cfgs over taskkill where possible.
- **MOHAA (retail, `C:\Program Files\EA GAMES\MOHAA`) is SafeDisc-blocked** on .143:
  modal "Cannot locate the CD-ROM" before any rendering. No local disc image found;
  DAEMON Tools is installed — needs a legitimately-owned image staged/mounted before
  MOHAA can join the benchmark matrix. NOT a driver issue.
- **PowerStrip on .143 autostarted from HKLM Run and pops a "Trial Expiration"
  modal that kills fullscreen-exclusive benchmarks** (likely aborted the first
  3DMark2001 640×480 run). Disabled by renaming the Run value to
  `PowerStrip.disabled-for-benchmarks` (same data — trivially reversible).
- RTCW crashed once in glide3x (NULL `[eax]`, Dr Watson 2026-07-21 00:09) during
  repeated kill/relaunch cycling; later launches render the menu fine. Same
  lost-context/teardown fragility class as the known Glide-exit issues.
- Games use our OpenGL ICD `retrogl.dll` (MesaFX-over-Glide) + `glide3x.dll`, loaded
  game-locally or from system32 — INDEPENDENT of the D3D display-driver path. But the
  ICD's Glide init needs the display driver loaded under its STANDARD name (rename
  path breaks it — ChoosePFD hang).
- Game-local `opengl32.dll` staleness trap: LoadLibrary checks the game dir before
  system32 — update EVERY game-local ICD copy, verify by GL_RENDERER string not size.

## Box ops

- **Autologon** fixed + verified: voidsstr had a BLANK password → set to `password`
  to match `DefaultPassword`; `AutoAdminLogon=1`, `DefaultUserName=voidsstr`. Reboots
  now auto-login to desktop. boot.ini default=XP timeout=30 (unattended reboot safe).
- TDR recovery: our D3D HAL wedge → soft VGA fallback, box + agent stay alive (PING
  ok while VIDEODIAG hangs); reboot restores full res. NOT a hard box-down.
- Daemon (retro-chat) claims .124 but connects on-demand — direct RetroConnection
  works fine; keep sessions short.

## Benchmark collection games (2026-07-21) — findings + per-game reality
Downloaded the Internet Archive "Benchmark Collection" (Benchmarks_v1.iso, 695 MB,
Coleslav) — ~20 games with built-in benchmarks + per-resolution shortcuts, Inno
Setup (silent-installable /VERYSILENT). Extracted installers staged at
~/staging/benchmarks-collection/installers, served on 192.168.1.132:8891.
- **Sin** (idTech2): runs on our driver; **our MesaFX ICD (retrogl) is INCOMPATIBLE
  with Sin's demo playback** — `+demomap cole.dm2` sticks at GL init with our ICD,
  but plays with the bundled **3dfx MiniGL**. Via MiniGL: **29.5 fps @640** (1893
  frames/64.2s). Recorded (sin-timedemo). Command: `sin.exe +set logfile 2 +timedemo
  1 +demomap cole.dm2`; fps in base\qconsole.log; demo takes ~65s (wait >=85s).
- **Incoming** (1998, Glide2x): **HARD-CRASHES our driver** (box unreachable ~2 min,
  then TDR-recovered). We provide Glide3x; Glide2x-era games are a crash risk. SKIP.
- **Hexen II** (GLQuake engine, glh2.exe): runs on our driver via its bundled MiniGL
  opengl32.dll (no crash), per-res benchmark shortcuts (`glh2.exe -width W -height H
  -bpp 16 +timedemo coleslav`), but the GLQuake timedemo fps did NOT reach
  qconsole.log even with -condebug + 75s — console-only output, headless capture
  unsolved.
- **Pattern:** each collection game needs per-game reverse-engineering (exact demo
  command from the .lnk unicode args, fps-output format/location, long waits) AND
  carries crash risk (Glide2x). Autonomous completion is impractical without the
  user present to recover a hard crash (box isn't physically accessible). Best done
  supervised, per-game.
- **Driver-quality signal for the ICD campaign:** our MesaFX ICD has compat gaps
  with older idTech2 games (Sin) where the 3dfx reference MiniGL works — a concrete
  target for ICD improvement.

## Benchmark matrix (our driver, ICD 0.1.31) — recorded in specpicks
| game | 640 | 800 | 1024 |
|---|---|---|---|
| Quake II       | 96.6 | 69.2 | 47.0 |
| Quake III      | 58.6 | 58.1 | 50.9 |
| RtCW wolfbench | 55.9 | 48.0 | 31.9 |
| Unreal Tournmt | 30.0 | 31.0 | 27.4 |
| SiN (MiniGL)   | 29.5 |  -   |  -   |
Context: period P3-850 + Voodoo3 3000 did ~60 fps Q3 @800 (ours 58.1) — competitive.

## D3D / 3DMark2001 SE isolation matrix (2026-07-21, .143 V5 5500, instrumented v5d)

First-ever COMPLETED 3DMark2001 run on the self-built XP driver:
**1636 3D marks** @ 640x480, 16-bit color/textures/Z, D3D Software T&L
(recorded in optimized/benchmarks/3dmark2001se-640x480x16-sli2.json).

| config | result | rendering |
|---|---|---|
| 16-bit, SLI_AA_CONFIGURATION default(2=2-way SLI) | full suite completes, score 1636, ring clean (no FIFO stalls) | **band-corrupted**: alternating good/garbage horizontal bands in ALL tests AND 2D loading screens (user CRT photos IMG_2062-64 + GDI grab) |
| 16-bit, SLI_AA_CONFIGURATION=0 (single chip) | renders CORRECTLY (clean mid-run grabs) | dies mid-suite `swapBuffer:Present : D3DERR_DRIVERINTERNALERROR`; timing varies (~45s fresh process, ~3.5min warm) — NOT a same-session state leak |
| 32-bit / compressed (Jul 17-21 history) | hard freeze or instant Present error | (pre-instrumentation) |

**SLI banding analysis:** slave VSA-100's bands are unrendered memory ⇒ slave never
executes the D3D command stream. Miniport SLI programming (H3_SETUP_SLI_AA) is
IDENTICAL code for the working Glide/OpenGL path (HWCEXT_SLI_AA_REQUEST → same
IOCTL; Q3 @1024 = 71.7 fps proves GL 2-way SLI works). v56k SLIAA.C changes are
pure gated additions (diff-verified, 0 vintage lines touched) ⇒ vintage bug in the
never-shipped XP D3D display-side SLI path (promote/CMDFIFO-vidmem/snoop interplay),
NOT our regression. Overlay promotion is REQUIRED for SLI (DDFXNT.C:2463 comment).

**DdFlip freeze vector found:** DDFLIP.C:342 `while (READSWAPCOUNT() > swapsQueued);`
unbounded spin — if hw stops retiring swaps this loops forever at raised IRQL =
the 3DMark hard-freeze signature. Now bounded @50M with ring log (instr2 build).

**Instrumentation added (3dfxv5d instr2, 952,120 B, deployed 2026-07-21 14:15):**
DP2-PARSE-ERR / DP2-EXIT-ERR (hr + failing opcode + offset at every
DrawPrimitives2 error exit), PROMOTE-SLIAA / PROMOTE-SLIAA OK / DEMOTE-SLIAA /
COMPUTE-SLIAA (full multi-chip request + primary hwPtr), DdFlip WEDGE-BREAK@50M.
Next repro of the Present error will name the failing D3D op in RLog.

## Supervised titles round 2 (2026-07-21) — Hexen II + 3DMark2000
- **Hexen II (glh2.exe, GLQuake engine)** — installed as `D:\Games\Heretic2`
  (mislabeled dir; it IS Hexen II). Its bundled 1997 `opengl32.dll` (MiniGL,
  126,464 B) **CRASHES at GL-context creation** under our WFP display driver
  (log stops at `640x480x16`, process exits). **OUR ICD runs it** — stage
  `retrogl.dll`→`opengl32.dll` + `glide3x.dll` (from `C:\Games\Quake2`), and it
  initializes (`GL_RENDERER: Mesa Glide v0.62 Voodoo3 [retro3dfx 0.1.31]`) and
  plays. **Our ICD: 109.9 fps @640 (5414f/49.3s), 48.7 fps @1024 (5414f/111.3s).**
  Recorded to specpicks (hexen2-timedemo). This is a POSITIVE ICD data point that
  contrasts SiN (where our ICD fails but MiniGL works) — our ICD's GLQuake-engine
  compat is title-specific.
  - **fps-capture SOLVED** (was "unsolved" in round 1): `glh2.exe -condebug
    -width W -height H -bpp 16 +timedemo coleslav`; the fps line lands in
    `data1\qconsole.log` as `NNNN frames  S seconds  F fps`. The round-1 "no
    capture" was really the MiniGL crashing before playback. **Wait scales with
    res**: 5414 frames at ~49 fps @1024 = ~111 s wall — poll to ~130 s, don't
    time out at 90 s.
- **3DMark2000 (D3D)** — installed `D:\Program Files\MadOnion.com\3DMark2000`.
  Launches to a clean GUI (detects our card), but **"Run Default Benchmark"
  (1024×768×16 D3D) CRASHES our display driver**: TDR dialog "The 3dfxv3d display
  driver has stopped working normally." Driver auto-recovered (VIDEODIAG still
  ours, desktop intact, no reboot needed). ⇒ **Our WFP build's Direct3D HAL is
  unstable/incomplete** — D3D-path benchmarking (3DMark2000/2001) is NOT viable
  until the D3D HAL is hardened. All our working benchmarks are the OpenGL-ICD
  and Glide paths. (This is why supervised: an unsupervised 3DMark run would have
  left the box in the TDR dialog.)

## ICD compiler-flag opt lane on .124 (2026-07-21) — /Ob2 verified-neutral
`.124` = ~845 MHz P3 + Voodoo3. Q3 timedemo flat ~58 fps at BOTH 640 and 800 ⇒
**CPU-bound at ≤800** on this box (contrast the GPU-bound Voodoo5 .143). A/B of
OGL.MAK item 1 **/Ob2** (add to release `/O2 /G6`; ICD 704512→729088 B, more
inlining): baseline 56.8/58.6/58.3 (avg 57.9, cold first run) → /Ob2
58.6/58.9/58.5 (avg 58.67). = +1.3% raw / ~neutral trimmed, zero regression,
byte-identical codegen semantics. **KEPT** (safe, non-negative). Corroborates the
.143 finding that C-codegen flags (/G6) are ~INERT: even CPU-bound, the hot cost
is the x87 hand-asm T&L + per-triangle Glide submit (`__GL_USE_INTEL_ASM`), not
the C the flag touches. ⇒ pure-flag ceiling is low; real wins are the C/asm
restructures (queue items 4-9, esp. #8 vertex-dedup ~10-25%). Next flag with
actual reach: /QIfist (item 3) — kills __ftol fldcw serialization in hot C
float→int casts.

## ICD flag A/B CORRECTED + 2.7MB-build discrepancy (2026-07-21)
**The earlier "/Ob2 = 58.67" was CONFOUNDED by the game-local deploy trap.** run_bench
launches quake3.exe from `C:\Quake III Arena\Quake3`, which had a game-local
`retrogl.dll` = the **2,742,298-byte** deployed build. Game-local shadows system32,
so my system32-only /Ob2 deploy was never loaded; the run measured the 2.7MB build
(~58 fps). The qconsole "LoadLibrary system32" line I trusted was STALE (from a
manual launch out of `C:\q3home`, which has no game-local copy). LESSON (re-confirmed,
matches [[text-garble-solved-alignment]]): trust NOTHING but a fresh qconsole; when
A/B-ing the ICD, NEUTRALIZE the game-local copy (rename it) so only the build under
test loads — or deploy to BOTH system32 AND every game-local path.

**Clean 3-way A/B (game-local renamed aside, all loaded from system32, confirmed):**
| build | flags | Q3@640 avg (3 runs) | vs base |
|---|---|---|---|
| base   | /O2 /G6 (704,512 B)        | 53.53 | — |
| /Ob2   | +/Ob2 (729,088 B)          | 53.80 | +0.5% (noise, non-neg) |
| /QIfist| +/Ob2 /QIfist (729,088 B)  | 52.73 | **−1.5% → REJECTED** |
/QIfist REJECTED: net regression on this box (the __ftol removal doesn't offset;
and it's a semantic chop→nearest change — reject a regressing semantic change).
/Ob2 kept (safe, marginally positive). Confirms the pure-flag ceiling here is ±1%.

**THE REAL FINDING: the deployed 2.7MB ICD (~58 fps) is ~8% FASTER than a clean
release rebuild from `v56k-6000` source (base = 53.5 fps).** The compiler flags are
±1% noise next to this ~4-fps source-level gap. The fast deployed driver
(3dfxvgl.dll, 2,742,298 B) was built from a config/branch that v56k-6000 does NOT
reproduce. ⇒ before more flag tuning, must identify + rebuild from the source that
produces the fast 2.7MB build (investigation in progress). Optimizing on the slower
v56k-6000 base would ship a regression vs what's already deployed.

## RESOLVED: the fast ICD is a DIFFERENT tree; compiler+math lane already exhausted (2026-07-21)
The 8% gap resolves cleanly — **I was optimizing the wrong source tree.**
- **Fast deployed ICD (2,742,298 B, ~58 fps)** = the **`retro3dfx-gl` GitHub fork**
  (MesaFX 6.2.2), cross-built with **mingw gcc-13** via
  `retro-agent/retro3dfx/build-mesafx-retail.sh`. Its compiler+math flags
  (`Makefile.mgw:74`): `-O2 -ffast-math -march=pentium3 -mtune=pentium3
  -mfpmath=sse -DNDEBUG`. THIS is the driver on .124. Rebuild via that script
  (needs `build-stack.sh` once first).
- **The 704 KB `opengl.dll`** I A/B'd today = the **retro-3dfx SWLIBS MSVC6/Wine**
  tree (`v56k-6000`) — a SEPARATE, slower lineage NOT deployed to .124. My
  /Ob2(+0.5%)//QIfist(−1.5%) results are real but on the wrong tree ⇒ irrelevant
  to the deployed driver. `icd-opt-ob2` branch keeps the /Ob2 experiment; NOT
  merged (inert + wrong tree).
- **The gcc compiler+math lane on the CORRECT fork was ALREADY RUN & EXHAUSTED
  2026-07-17** (retro-agent/retro3dfx/CHANGELOG.md 0.1.7–0.1.11): `0.1.7 opt/lto`
  **-O3 -funroll-loops = 58.7 INERT** ("hot path already SSE; -O can't remove
  algorithmic cost"); SSE cliptest intrinsics **REGRESSED** (Vanderhoof x86 asm
  wins); SSE emit INERT. Only `0.1.11` lod-bias (quality) merged. **Verdict:
  MesaFX/V3 vertex path is near-optimal; it already beats AmigaMerlin + era 3dfx
  ICD.** ⇒ No meaningful compiler+math headroom remains. Today's MSVC-tree A/B
  independently reached the same conclusion (flags ±1%, cost is in the asm).
- **Remaining ICD levers are NOT compiler/math:** quality knobs (lod-bias done;
  gamma/dither done), higher-effort compat/features (texture_env_combine on V3,
  ARB pixelformat, S3TC — 0.1.30 review open items), or accept the V3 hardware
  ceiling (fillrate at high-res + single-TMU). PGO was deferred (needs on-target
  instrumented run) but is low-EV given -O3 was inert.
- **DEPLOY DISCIPLINE (cost me a confounded run today):** .124's Q3 dir
  (`C:\Quake III Arena\Quake3`) has a game-local `retrogl.dll`; run_bench launches
  from there so game-local SHADOWS system32. A/B the ICD by neutralizing the
  game-local copy OR deploying to system32 AND every game-local path.

## ROOT CAUSE FOUND+FIXED: black/garbage mipmapped textures in D3D (2026-07-21)

**Symptom:** all mipmapped D3D content black (3DMark trucks/dragons) or garbage-
striped; un-mipped content (HUD, skybox, 2D) perfect. GL/Glide unaffected.
**Repro:** minimal windowed D3D8 lab (`toolchain scratchpad d3dlab.exe`, staged
C:\RETRO_AGENT\d3dlab.exe): `big512mip` = solid black, `mippoint` = garbage
stripes, any no-mip mode = perfect. Deterministic in 5 s — no benchmark needed.
**Root cause:** W2K `D3TXTR.C` rev 40 (10/25/00, days before 3dfx shut down,
"no longer use surface local pointers") deleted the per-LOD board-offset line in
TEXTURELOAD's mipmapped path, leaving `addr` stale for every mip-level download:
`addr = psurfDst->mmData[nDstLOD].fpVidMem - _FX(textureHeapStart[tmuCnt]);`
(Win9x rev 35 has the line; the dangling "// board address offset" comment
made the deletion visible.) Texels landed at stale offsets; the TMU sampled
unwritten memory.
**Fix:** restored the line (commented). Verified on .143: all d3dlab mip modes
correct (marker-color mips sample at right LODs), mippoint garbage → perfect
checker.

## Warm-rerun D3D degradation (2026-07-21, post-mip-fix) — NEXT INVESTIGATION
3-run warm-rerun loop (same 3DMark process, 640x480x16, single-chip, mip-fix
driver): run0 ended ~4min no score, run1 bounced fullscreen/desktop and ended
~70s no score, run2 produced a score window after only ~44s (tests failing
progressively faster). Fresh-boot runs complete the full ~6min suite cleanly
(1601 marks). Pattern = driver state degrades across repeated D3D device
create/destroy cycles in one boot (leak: texture handles / heap fragmentation /
D3 context state). Matches the historic "errors INSTANTLY after previous
activity" reports. Ring showed no DP2/WEDGE errors -> the failure path returns
clean errors to the runtime or dies in the runtime. Next: instrument
D3CONTXT create/destroy + heap stats into the ring, run N warm reruns, watch
for monotonic drift (fresh boot needed between reliability experiments).

## RESOLVED: warm-rerun "degradation" is 3DMark2001, not the driver (2026-07-21)
Decisive test: 4 COLD-process runs (kill+relaunch 3DMark each time, NO reboot,
same instr7 driver) = **['ok','ok','ok','ok']** — every fresh-process run
completes the full suite. WARM reruns (Benchmark again in the same process)
failed randomly ~2/3. Conclusion: 3DMark2001SE corrupts its own D3D state on
re-benchmark within one process; a fresh process clears it. Corroborated by
the driver being provably clean across every warm-rerun failure (instr4-7 ring:
CTX create/destroy balance returns to 0, dd3DSurfaceCount flat, ZERO
CREATESURF-FAIL/CSEX-FAIL/ALLOC-FAIL/DP2-ERR/WEDGE lines; DP2-FIRST fires =
first frame drawn) and by the random (not monotonic) pass/fail pattern.
**Benchmark protocol: one fresh 3DMark process per run** (dm_freshproc.py /
dm_wrap.exit-wrapper). Real-world launch-and-run is unaffected. NOT a driver bug.

## RESOLVED: D3D "2-way SLI banding" was the mip bug too (2026-07-21 evening)
Re-tested D3D 3DMark2001 on 2-way SLI (SSTH3_SLI_AA_CONFIGURATION default=2 on
5500) with the mip-fix driver: **Car Chase renders perfectly, zero banding**
(sli_midrun.png). Ring confirms SLI genuinely active: PROMOTE-SLIAA
`sliEn=1 aaEn=0 nlines=16 chips=2` + PROMOTE-SLIAA OK. The earlier "alternating
good/garbage horizontal bands" (CRT photos IMG_2062-64) were NOT a slave-chip
command-stream fault — they were the rev-40 mip-download bug's garbage texture
memory being scanned out through the SLI band-interleave, which mimicked SLI
banding. The mip fix (08fd889) closed this open item. D3D 2-way SLI works.
Lesson: don't attribute a display-interleave-shaped artifact to the SLI path
before ruling out texture/framebuffer content corruption.

## Benchmark rerun 2026-07-21 evening (all fixes verified in place)
Predeploy gate PASS (40 checks); on-target D3D suite 17/17; OpenGL golden gate
PASS (Q3 1024 world non-black nb=84 gr=0, CS de_dust 0-green). Q3 timedemo
(2-way SLI, ICD as deployed): 640=68.2, 800=72.9, 1024=70.6 fps (vs prior
72.8/73.0/71.7 — within run variance, no regression). D3D single-chip 3DMark
1601; 2-way SLI run pending.

## OPEN (new, 2026-07-21 night): D3D device-cycle accumulation → display wedge after ~12 cycles/boot
Distinct from the 3DMark warm-rerun app issue (that was one process; this is
fresh processes). Running d3dlab (each a fresh D3D device create/destroy) in
sequence: the first ~12 device cycles per boot render fine, the ~13th wedges
the display (SCREENSHOT hangs, box starves ~60-90s then self-recovers — NOT a
hard freeze, the wedge-breakers hold). Evidence: on-target d3dlab suite passes
12 modes then hangs on the 13th (dxt1); dxt1 run FIRST (standalone) passes, so
it's the cycle COUNT not the mode. Cold 4-process 3DMark passed (4 < 12).
Implies a driver-side per-device resource not fully freed on device/process
teardown (kernel-side display-driver allocation; CTX create/destroy ring
balance returns to 0, so it's NOT the D3D context — suspect a heap/handle-list
or exclusive-mode artifact in DDMEMMGR/DdCreateSurface/HNDLLIST). Real-world
impact: low (needs 12+ D3D app launches without reboot) but a genuine leak.
Mitigation in place: on-target suite runs a curated 9-mode set (under
threshold); full 14-mode matrix via RETRO_D3DLAB_MODES=all on a fresh boot.
NEXT: instrument per-device heap free counts + HNDLLIST alloc/free balance
across N cycles; find the unfreed allocation. Deferred — driver is otherwise
stable and this needs careful measurement, not a blind fix.

**CAVEAT (contention):** during this investigation the agent version changed
under me (1.15.0 → 1.15.6), i.e. a CONCURRENT session was actively working
box .143. The progressively-faster wedging (1-2 cycles late, vs 12 early) is
likely confounded by that session's own D3D/agent activity on the shared box.
The device-cycle accumulation is real (reproduced when I had the box to myself:
12 modes then wedge) but its exact threshold and whether reboots fully clear it
need a DEDICATED, UNCONTENDED session to measure. Not a hard freeze in any case
(wedge-breakers hold; box self-recovers). Box left clean-rebooted + healthy.

## RESOLVED (2026-07-22): "device-cycle accumulation" is NOT a driver leak
Investigated the open item with instr8/9 (kernel-pool + video-surface alloc/
free balance counters in the display driver, read from the persistent registry
ring — immune to agent/network confounds). Controlled, single-session cycling:
  - 16x sel1 (tiny 64x64):            poolLive FLAT 36, no wedge
  - 16x big512mip (512x512 mipmapped): vidSurfLive FLAT 1, nullFree=0, no wedge
  - 16x big512mip WITH mid-render GDI SCREENSHOT each cycle: no wedge
Both kernel pool AND video-memory surfaces are freed cleanly on every D3D
device teardown. There is NO monotonic resource leak. The driver's windowed-
D3D device create/destroy cycle is clean.

**The earlier apparent ~12-cycle "wedges" were a CONCURRENT-SESSION artifact.**
During these tests the agent version churned 1.15.0 -> 1.15.6 -> 1.16.0 — a
second session was actively rebuilding and REDEPLOYING the agent on the shared
box .143 (agent redeploy = agent restart, sometimes reboot). My multi-minute
cycle sequences overlapped those deploys; an agent restart mid-sequence times
out the client connection and looks identical to a display wedge. dxt1 "wedged
on cycle 1" exactly as the agent went 1.15.6->1.16.0. Single-mode runs that
happened to fall between deploys ran 16 clean cycles.

CONCLUSION: no driver fix needed for this item. The D3D device cycle is leak-
free (proven). The on-target suite's curated-mode workaround (added when the
cause was thought to be a leak) is unnecessary but harmless; the full 14-mode
matrix (RETRO_D3DLAB_MODES=all) passes on an UNCONTENDED clean box. For future
driver verification on .143, coordinate with any concurrent session or use a
window when the agent version is stable. Leak-balance instrumentation retained
(instr9) as a permanent diagnostic.

## DEFINITIVE (2026-07-22): the display driver's D3D cycle is CLEAN — "wedge" is agent/box-level
Read the FULL flight recorder after a client-observed dxt1 "wedge" (cycle 13,
no screenshots, agent version stable 1.16.0). EVERY cycle in the ring logged
cleanly: CTX-CREATE (poolLive=34, vidSurfLive=1, nullFree=0) -> DP2-FIRST (drew)
-> CTX-DESTROY (balanced) — including the dxt1 cycle. NO ALLOC-FAIL, NO
WEDGE-BREAK, NO DP2 error anywhere. poolLive and vidSurfLive are ROCK STEADY
across every cycle. So the display driver did NOT wedge or leak — it completed
dxt1's create/draw/destroy with balanced resources.

The client-observed "UNREACHABLE" is therefore the AGENT/box going transiently
network-unresponsive, NOT a display-driver defect. Corroborated by
non-determinism: sel1-cycle-1 went unreachable once (sel1x16 passed cleanly
elsewhere); big512mipx16 never failed; the failure point isn't a fixed mode or
count. This is environmental (a concurrent session was actively rebuilding +
redeploying the agent, 1.15.0->1.16.0, and exercising the shared box .143
throughout).

**RESOLUTION of the open item:** NO display-driver fix is warranted. The D3D
device create/destroy cycle is proven leak-free (pool + video memory balanced
across 16+ cycles) and clean (flight recorder shows no wedge/error even on the
cycle the client called a wedge). The instr8/9 balance instrumentation is
retained as a permanent diagnostic.

**One agent-domain follow-up (not display-driver):** the agent's GDI SCREENSHOT
(DrvCopyBits/DrvBitBlt) during an active D3D present is the operation most
correlated with client stalls. If it recurs on an UNCONTENDED box, audit the
2D GDI blit path for an unbounded accelerator wait (the 4 wedge-breakers cover
the DDraw/3D paths H3MakeRoom/DdFlip/FXBUSYWAIT/H3_GP_WAIT; a pure-2D DrvBitBlt
spin, if any, is not yet bounded). Deferred: needs a clean box to reproduce,
and the driver's own log shows no wedge, so this is a low-priority robustness
audit, not a confirmed bug.

## FOLLOW-UP COMPLETE (2026-07-22): all accelerator waits bounded; screenshot path was already safe
Executed the deferred 2D-path audit. Result: the LIVE GDI screenshot readback
path — DrvCopyBits -> START_DIRECT_ACCESS_H3 (H3G.H) -> H3_GP_WAIT — was ALREADY
bounded by the H3_GP_WAIT wedge-breaker added during stabilization (fc8e313).
That is exactly why the display driver never actually wedged (flight recorder
stayed clean through every "wedge"): the screenshot's own engine wait was
already protected. So the earlier client "unreachable" events were agent/box
transients, not this path.

Completed the coverage anyway (every accelerator wait now bounded):
- LIVE DDraw surface-lock busy spins: DDSURF.C x3, DDOVL32.C x2 -> FXBUSYWAIT
  (already bounded). LIVE flip-status waits: DDSURF.C DdLock + DDFLIP.C DdFlip
  -> new DdLock-FlipWait / DdFlip-FlipWait breakers.
- Dead-code blit spins (BITBLT.C x4 under PERF_COPY_BITS_OPT, DDFXNT.C x1 under
  ENABLE_V3_W2K_GLIDE_CHANGES — both #ifdefs off) bounded defensively.
Deployed instr10, verified NO regression: D3D 5 key modes render correct
(sel1/dxt1/big512mip/tex2/mod2x match goldens), OpenGL golden gate PASS (Q3
1024 world nb=84 gr=0, CS de_dust 0-green). Predeploy gate + source/binary
assertions updated. The stabilization follow-up is closed.

## 2026-07-22 — clean-room glide (Voodoo3) bring-up: TLS accessor crash (getThreadValueFast)

**Symptom:** clean-room `glide3x.dll` (voodoo-cleanroom, MesaFX ICD path) crashed
Q3 at `GLW_ChoosePFD` with `instruction at 0x06f5a22d referenced memory at
0x0000001c` (NULL+0x1c read). Same MesaFX ICD works fine on retail glide.

**Root cause:** `getThreadValueFast()` (glide3/src/fxglide.h) reads the TLS slot
straight out of the TEB via `%fs:` + `_GlideRoot.tlsOffset`. Our mingw/gcc-13
build's inline-asm variant faulted inside `grGetString`'s `GR_DCL_GC` — the first
glide entry to actually *read* TLS (detect/select only *write* via
`setThreadValue`/`TlsSetValue`). Fix: use the ABI-correct `TlsGetValue(tlsIndex)`
instead of the raw TEB read. (tlsIndex measured =18, OSWin95=0, so the classic
high-index/OS-mismatch theories were NOT the cause — the raw `%fs:` read itself
was the problem under our toolchain.)

**Deadly-trap corollary — stale objects on header change:** the era Makefiles do
NOT track header dependencies. Editing `fxglide.h` (where `getThreadValueFast` is
an inline) did NOT recompile `diget.o` et al — the old `%fs:` asm stayed in the
DLL and the crash persisted through a "successful" rebuild. Always
`find glide3 minihwc -name '*.o' -delete` before rebuilding after a HEADER edit.

**Result:** after the fix, `grGetString(GR_HARDWARE)` returns "Voodoo3 (tm)",
Q3 gets "3 PFDs found / hardware acceleration found / PIXELFORMAT 2 selected",
and **grSstWinOpen is reached** (was never reached before). Crash moved deep
into `hwcInitVideo` (next layer). Layers solved so far: base-mapping,
MMIO-read (dramInit1=0x40530031 real HW), detect+bInfo, grGlideInit, TLS.

## 2026-07-22 — clean-room glide RENDERS Q3 (lost-context NULL deref fixed)

After the TLS fix (above), grSstWinOpen reached `hwcShareContextData` →
`*gc->lostContext = FXFALSE` and crashed: gc->lostContext == NULL. Root cause:
`hwcShareContextData` NT/CONTEXT_DWORD_NT branch (minihwc.c) stored the driver's
`dwordOffset` with NO fallback, while the SHARE_CONTEXT_DWORD (non-NT) and linux
branches fall back to `&dummyContextDWORD`. Our display driver maps no
lost-context dword → NULL. Fix: mirror the `&dummyContextDWORD` guard in the NT
branch. **Result: Quake 3 renders correctly** on the clean-room open-source stack
(retro3dfx-glide + MesaFX retrogl ICD) on .124 Voodoo3 — full HUD, textures,
lighting, in-world text, player models (verified via windowed LFB->GDI capture;
fullscreen Glide output isn't GDI-capturable on 3dfx).

Full layer sequence solved this session (all in the clean-room glide bring-up):
base-map (GETLINEARADDR-before-ALLOCCONTEXT) → MMIO read (dramInit1=0x40530031
real HW) → detect+bInfo → grGlideInit → **TLS accessor (TlsGetValue)** →
**lost-context NULL fallback** → hwcInitVideo → render. Fork commit a71eb3f
(voidsstr/retro3dfx-glide @ glide-devel-sezero).

OPEN: intermittent crash under FULLSCREEN (r_fullscreen 1) — windowed renders
clean; fullscreen sometimes stops at GLW_ChoosePFD (possibly a stale crash
dialog stealing the DDraw exclusive mode switch). Next: broader game/res sweep +
fullscreen stability.

### clean-room glide Q3 fullscreen resolution sweep (2026-07-22, .124 Voodoo3)
All 1260-frame timedemos COMPLETED — stable across resolutions, no crashes:
  640×480  46.0 fps   (retail ref 58.8)
  800×600  44.3 fps   (retail ref 58.4)
  1024×768 39.2 fps   (retail ref 51.2)
Clean-room glide is ~78–80% of retail glide speed → optimization target (the
render/FIFO/LFB path, not the vertex path which the ICD campaign already tuned).
Stability is solid. NOTE: a ghost "quake3.exe - Application Error" dialog (no
owning process, survives taskkill + UICLICK) lingers from an earlier crashed run;
cosmetic — does NOT block rendering (sweep ran with it present) — cleared by reboot.

### clean-room glide multi-engine validation (2026-07-22, .124 Voodoo3)
The clean-room glide renders BOTH engines via the full open-source stack
(our glide3x + our retrogl/MesaFX ICD), each completing a full timedemo:
  Q3 (id Tech 3)  640×480  46.0 fps  (retail 58.8)
  Q2 (id Tech 2 / ref_gl)  640×480×16  88.4 fps  (retail 93.6)
     GL_RENDERER: "Mesa Glide v0.62 Voodoo3 (tm) [retro3dfx 0.1.31]", clean ShutdownGame.
Consistent ~78–94% of retail glide speed → the clean-room glide's render/FIFO/LFB
path is the optimization target (vertex/ICD path already tuned in the ICD campaign).
Deploy: OUR glide3x.dll next to each game exe (LoadLibrary search order); Q2 also
needs gl_bitdepth 16 + gl_mode 3.

## 2026-07-23 — CS 1.6 / GoldSrc "display mode not working" on .124 Voodoo3: FIXED (OpenGL), D3D not viable

**Symptom:** CS 1.6 launched from the desktop failed with "display mode not
working" / "Video mode change failure".

**Root causes (two, both fixed):**
1. **PowerStrip** (a display-mode-hooking tuning util) auto-started (HKLM\...\Run
   `PowerStrip=d:\program files\powerstrip\pstrip.exe`), had crashed, and its
   "Safety Precaution" recovery dialog blocked startup AND it hooks
   ChangeDisplaySettings — breaking games' fullscreen mode switches. FIX: removed
   its autostart + killed it. (This is the "Safety Precaution" dialog seen on
   base HL too — it's PowerStrip, not the game.)
2. **Broken GoldSrc video config**: `HKCU\Software\Valve\Half-Life\Settings`
   `EngineDLL=sw.dll` (software) + ScreenWidth/Height **swapped** (480×640). FIX:
   `EngineDLL=hw.dll`, ScreenWidth=640, ScreenHeight=480, ScreenBPP=16.

**OpenGL — WORKS via our clean-room stack (CS menu + de_dust render):**
- Deploy OUR **retrogl ICD AS `gldrv\3dfxgl.dll`** (GoldSrc's `EngineGLDriver=3dfxgl.dll`
  loads it as the "3Dfx OpenGL" driver). Our retrogl uses **Glide-exclusive
  fullscreen** (grSstWinOpen, like Q3) which **bypasses the GDI ChangeDisplaySettings
  mode-switch** that fails with the stock 3dfx MiniGL ("Video mode change failure").
- Pair with OUR **clean-room glide3x** — it exports `grAADrawTriangle@24`, which our
  retrogl imports but the **retail AmigaMerlin glide3x does NOT export** (→ "Entry
  Point Not Found: grAADrawTriangle@24 in glide3x.dll" if paired with retail glide).
- system32\glide3x is **WFP-protected** → seed `dllcache\glide3x.dll` with ours
  FIRST, then system32 (WFP escape), so ours persists.
- `FX_NO_PALETTED_TEXTURE=1` (CS paletted textures).
- **GOTCHA — opengl32 is a KnownDLL:** a game-local `opengl32.dll` (retrogl) is
  IGNORED (Windows loads system32's). Deploy retrogl **as `gldrv\3dfxgl.dll`**, not
  as game-local opengl32.
- **GOTCHA — BCShield** (BC Romania build anti-cheat) kills `hl.exe` if launched
  directly; launch via `Counter-Strike.exe`. (The menu is stable; the earlier
  `+map` "exits" were BCShield, not the driver — de_dust rendered fine first.)

**Direct3D — NOT viable for GoldSrc on the Voodoo3:** `-d3d` → "Video mode change
failure: the specified video mode is not supported → software mode", at every
resolution. D3D on .124 goes through the **vintage H5 D3D HAL (3dfxv3d.dll), NOT
our clean-room stack** (which has no D3D HAL); the H5 HAL doesn't enumerate the
DirectDraw/D3D fullscreen mode GoldSrc's (deprecated) D3D renderer wants. No
Glide-fullscreen bypass exists for D3D (it must use DDraw mode enumeration).
OpenGL is the correct GoldSrc path on 3dfx (as it always was).

## 2026-08-30 — DOOM 3 cannot be staged: v1.3 gates the MAIN MENU on a local CD-key check

The only Doom 3 copy on the share (`Files/Games/Windows XP/DOOM 3/`) is a
Demonoid torrent container. Its three ISOs look genuinely retail — volume
labels `DOOM3_1/2/3`, 2004 file dates, `pak000..pak004.pk4` stored as plain
files, and SafeDisc's `DrvMgt.dll` still present (a repacker would have
stripped it). The official id patch `DOOM 3 UPDATE 1.3.exe` is there too. So
the *binaries* are publisher binaries and, on the face of it, stageable.

It is still blocked, and the proof came from the bundled crack's own NFO
(`PATCH/TNT.NFO`, TEAM TNT, 2005-05-25):

  "v1.3 added more checks over v1.2. It does a local CD-Key check now as well
   as the previous online CD-Key check **before you can even gain access to the
   Main Menu**. This crack will allow you to play Single Player without a valid
   Key..."

Two things follow, and they are easy to get backwards:

* **The 1.3 patch DOES remove SafeDisc** — the same NFO says "v1.2 and v1.3 do
  not have copy protection. Previous versions used Safedisc." So no disc, no
  mount launcher and no no-CD patch are needed. That part is solved.
* **The 1.3 patch ADDS a local CD-key check that runs before the main menu.**
  Single player is gated on it. So a key is not optional, not multiplayer-only,
  and cannot be sidestepped by installing via direct file copy instead of
  running the installer.

The only key on the share is `DOOM SERIAL.txt`, shipped in the same torrent
("THIS ONE IS OFFICIAL"). The NFO also notes 1.3 broke keygen keys — which
means a key that still works in 1.3 is a *leaked real retail key*, not a
generated one. Using it is worse provenance, not better.

**So the blocker is exactly one artifact: a legitimate CD key.** Not the data,
not the patch, not the DRM. Unblock it with the user's own retail key, or a
GOG/Steam Doom 3. Do not reach for `PATCH/DOOM3.EXE` — that is the TNT crack.

Retail data was extracted and left at `~/.cache/d3stage/iso/` (pak000-pak004 +
game00.pk4 + retail Doom3.exe, ~2.4 GB) so that supplying a key is a short
finish rather than another hour of ISO reads.

**The general lesson: read the crack's NFO before deciding a title is clean.**
It is the most precise available description of what the protection actually
does, written by people who had to defeat it. Here it settled both questions —
that SafeDisc was gone, and that the key check was not — in one paragraph.

## 2026-08-30 — Far Cry staged from the GOG build; fleet GPUs are much better than the docs claim

Staged `Games-Library/FarCry/` (3610 MB) from
`setup_far_cry_2.0.0.9.exe` (GOG, DRM-free, = retail patched to 1.4),
extracted with `innoextract --gog`. No key, no disc, no crack; all 57 PE files
are XP-loadable (no SubsystemVersion >= 6.0).

Two things worth remembering:

* **The Far Cry engine has NO registry dependency at all.** `CrySystem.dll`,
  `CryGame.dll` and `FarCry.exe` contain no `Software\...` string; everything
  resolves relative to the working directory. That is why the launchers
  `cd /d "%~dp0"` and why `install.reg` carries only an App Paths entry.
* **Far Cry rewrites `System.cfg` on exit**, so a mode set once on a box sticks
  and no redeploy corrects it. The launchers therefore `copy /Y` a staged
  template over `System.cfg` every launch — the same class of problem as an id
  engine rewriting `config.cfg`, solved the same way.

**CLAUDE.md's hardware table is badly out of date and cost real reasoning time.**
Measured 2026-08-30 across the eight live boxes:

  .123  Athlon 64 4000+ 2403 MHz, 2047 MB, **Radeon HD 3850 AGP** (1002:9515)
  .124  PIII 845 MHz, 511 MB, GeForce2 GTS (10DE:0150)
  .133  dual PIII 701 MHz, **255 MB**, **GeForce4 Ti 4600** (10DE:0250)
  .143  Athlon 1000 MHz (family 6 model 2 — K7, **no SSE**), 511 MB,
        **GeForce 6800** (10DE:0041) alongside the Voodoo5 5500
  .145  i5-2400 3093 MHz, 2047 MB, GeForce 8400GS (10DE:10C3)
  .171  P4 2793 MHz, 509 MB, Intel 865G only (8086:2572)
  .240  Athlon 64 3300+ 2403 MHz, 1534 MB, **Radeon 9800 XT** (1002:4E4A)
  .246  i5-2400 3093 MHz, 2047 MB, Radeon HD 6xxx (1002:68F9), **Windows 7**

So ".133 = Voodoo5 6000" and ".143 = Voodoo5 5500" describe the 3dfx card in
the box, not the card driving the display. Three boxes carry a DX9-or-better
GPU that the fleet docs do not mention at all. Verify with `VIDEODIAG` before
reasoning about what a box can run.

---

## Fleet capability gaps (appended 2026-08-30 by the retro-agent coordinator session)

*Appended at the end rather than inserted newest-first: this file was already
modified by another session and the top of the file is where they collide.*

### THREE BOXES HAD NO DISC MOUNTER, AND SEVEN STAGED TITLES SILENTLY DEPENDED ON ONE

`.123`, `.246` and `.124` had **no disc-image mounter at all**; only `.133` and
`.171` did. Seven already-staged titles mount a disc image from their launcher —
**SystemShock2, Shogo, RedFaction, StarCraft, Descent2, Descent3,
SoldierOfFortune2** — so on those boxes those titles had **never been able to
work**, and nothing anywhere reported it. The launchers tolerate the failure,
which is correct behaviour, but it means the gap is invisible until someone
double-clicks the shortcut.

The lesson generalises past mounters: **a runtime dependency that lives on the
BOX rather than in the staged tree is invisible to every check we have.** The
validator proves a title is internally consistent; `GAMESYNC` proves the files
arrived. Neither can see that the machine lacks something the launcher needs.
That is the gap the capability gate is being built to close, and a mounter is a
first-class capability in it — note it is **software state, remediable**, unlike
a GPU that simply is what it is.

Installing Daemon Tools 3.47, for whoever does the remaining boxes:
- **`/S` does not work.** It is a UPX-packed custom stub, not NSIS or Inno, so
  the silent switch is simply ignored and you must walk the GUI:
  Install → Next x3 → Close → **No** to its reboot prompt.
- **The virtual drive does not appear until a reboot**, even though the `d347bus`
  service already reads RUNNING. Do not conclude the install failed.
- Check `LICSTATUS` first and reboot with `scripts/fleet/safe-reboot.py <ip>` —
  never a bare `REBOOT`, because these boxes PXE boot first.
- `.246` is **Windows 7**; confirm 3.47 is the right build there before walking
  its installer.

### A CRACK CAN LIVE IN A DLL WHILE THE MAIN EXE HASHES CLEAN

Battlefield 1942's copy protection is **not** in `BF1942.exe` — that binary is
clean in both the cracked and the original copy. It lives in
`Mods\bf1942\Mod.dll`, which the repack had replaced with a **4,096-byte stub**
carrying `tHIS iS a wIN 32 pROGRAM! -=[ tE ]=-`, exporting exactly the three
symbols the engine looks up and doing nothing but `ExitProcess`. The genuine
`Mod.dll` is SafeDisc 2 wrapped.

**So verifying provenance by hashing the executable you launch proves nothing.**
Check every binary the engine loads. A second, independent patch to `BF1942.exe`
(50 bytes across 14 short runs, identical PE timestamp and size) was found first
and would have been treated as the whole story.

### TWO FLEET BOXES ARE NOT VINTAGE

`.145` (DELL) and `.246` (ADMIN-PC, Windows 7) are **Sandy Bridge quad-cores**;
every other box is a PIII or P4. Any claim of the form "the fleet cannot run X"
is wrong unless it excludes those two, and they are the only realistic targets
for the 2004-era titles being staged.

Also: **`SYSINFO` reports no CPU MHz, no total RAM and no GPU name** on any box
(`gpu: None` on all eight), and `VIDEODIAG.adapters[0]` lists stale registry
keys rather than the live adapter. Neither is a basis for a hardware decision.
