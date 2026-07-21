#!/bin/bash
# PRE-DEPLOY GATE for the 3dfx driver stack. Run this before copying any
# rebuilt driver binary to a box. It refuses (non-zero exit) if a verified
# fix is missing from the sources, the build tree is out of sync with the
# repo tree, or the built DLL is stale / missing fix markers.
#
#   tests/predeploy.sh                 # gate the display driver artifact
#   tests/predeploy.sh <path-to-dll>   # gate a specific artifact
#
# After deploy + reboot, run the on-target suite:
#   python3 tests/run_target_tests.py          # D3D matrix + ring checks
#   python3 /tmp/post_instr_verify.py          # OpenGL golden gate (Q3/CS)
set -o pipefail
cd "$(dirname "$0")" || exit 2
overall=0
echo "===== predeploy gate ====="
bash test_source_invariants.sh || overall=1
echo
bash test_built_artifact.sh "$@" || overall=1
echo
if [ $overall -eq 0 ]; then
  echo "PREDEPLOY GATE: PASS — ok to deploy"
else
  echo "PREDEPLOY GATE: FAIL — do NOT deploy"
fi
exit $overall
