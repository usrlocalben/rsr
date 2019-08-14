#!/bin/bash -eu

# Copyright 2017 The Bazel Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -eu
OUTPUT=$1

function fail() {
  echo >&2 "ERROR: $@"
  exit 1
}

# Ensure the PATH is set up correctly.
if ! which which >&/dev/null ; then
  PATH="/bin:/usr/bin:$PATH"
  which which >&/dev/null \
      || fail "System PATH is not set up correctly, cannot run GNU bintools"
fi

# Check if Visual Studio is installed. Look for it at the default
# locations.
#VSVARS="C:/Program Files (x86)/Microsoft Visual Studio/2017/"
VSVARS="C:/Program Files (x86)/Microsoft Visual Studio/2019/"
VSEDITION="BuildTools"
if [ -d "${VSVARS}Enterprise" ]; then
  VSEDITION="Enterprise"
elif [ -d "${VSVARS}Professional" ]; then
  VSEDITION="Professional"
elif [ -d "${VSVARS}Community" ]; then
  VSEDITION="Community"
fi
VSVARS+="$VSEDITION/VC/Auxiliary/Build/VCVARSALL.BAT"

if [ ! -f "${VSVARS}" ]; then
  fail "VCVARSALL.bat not found, check your Visual Studio installation"
fi

cat > "$OUTPUT" <<EOF
@call "${VSVARS}" amd64
EOF

chmod +x "$OUTPUT"
