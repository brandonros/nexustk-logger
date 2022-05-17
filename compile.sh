#!/bin/sh
set -e
# cleanup
rm -f nexustk-logger.dll nexustk-logger.exe
# init compiler
if [ ! -f ~/dockcross-windows-shared-x86 ]
then
  docker run --rm dockcross/windows-shared-x86 > ~/dockcross-windows-shared-x86
  chmod +x ~/dockcross-windows-shared-x86
fi
# compile
~/dockcross-windows-shared-x86 bash -c '$CC process.c dll.c -shared -o nexustk-logger.dll -lpsapi'
~/dockcross-windows-shared-x86 bash -c '$CC process.c main.c -o nexustk-logger.exe -lpsapi'
