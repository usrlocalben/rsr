#!/bin/bash -eu
RC_FILE=$1
HEADER_FILE=$2
OUTPUT=$3
VCVARS_SETTER=$4


export TMP="$(mktemp -d)"
OWD=$(pwd)
# trap "cd \"$OWD\" && rm -fr \"$TMP\"" EXIT


#mkdir -p "$TMP"/$(dirname "$RC_FILE")"
cp $RC_FILE $TMP/bazel.rc

# use this if .rc file includes e.g. "src/foo/bar/resource.h"
#mkdir -p "$TMP/$(dirname "$HEADER_FILE")"
#cp $HEADER_FILE $TMP/$HEADER_FILE

# use this if .rc and resource.h are in same dir (makes vs happy)
cp $HEADER_FILE $TMP/$(basename "$HEADER_FILE")


# Create the batch file that sets up the VC environment and runs the Resource
# Compiler.
RUNNER=$TMP/vs.bat
"$VCVARS_SETTER" $RUNNER
echo "@rc /nologo bazel.rc" >> $RUNNER
chmod +x $RUNNER


# Run the script and move the output to its final location.
cd $TMP
#cat $RC_FILE > bazel.rc


./vs.bat
cd $OWD
mv $TMP/bazel.res $OUTPUT
