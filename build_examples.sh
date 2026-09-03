#!/bin/bash

if [ ! -d build ]; then
    mkdir -p build;
fi

CompileFlags=" \
    -g \
    -O0 \
    -ffreestanding \
    -fno-stack-protector \
    -fpie \
    -std=c11 \
    -nostdlib \
    -Wall -Wextra -Wpedantic -Werror \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function"

LinkFlags=" \
    -fuse-ld=lld \
    -Wl,-nostdlib \
    -Wl,-entry,EntryPoint"

GreenColor='\033[0;32m'
RedColor='\033[0;31m'
DefaultAttributes='\033[0m'

for FilePath in examples/*.c; do
    FileNameWithExtension="${FilePath##*/}"
    FileNameAlone="${FileNameWithExtension%.*}"

    SourceFile="${FilePath}"
    OutputFile="build/${FileNameAlone}"

    clang $CompileFlags $LinkFlags $SourceFile -o $OutputFile

    if [ $? -eq 0 ]; then
        printf "${GreenColor}[SUCCESS]:${DefaultAttributes} ${FileNameWithExtension}\n"
    else
        printf "${RedColor}[FAILED ]:${DefaultAttributes} ${FileNameWithExtension}\n"
    fi
done

