#!/usr/bin/env bash
set -e
cd build
make
cd ..

# ./build/TypherFrontend/TypherCompiler ./Tests/Sanity/sanity.ty

DEFAULT_EXE="Sanity/sanity.ty"

if [ -n "$1" ]; then
    EXE_TO_RUN="./Tests/$1"
else
    EXE_TO_RUN="./Tests/$DEFAULT_EXE"
fi

# Always double quote your variable when invoking!
./build/TypherFrontend/TypherCompiler "$EXE_TO_RUN"

./Sanity
echo $?