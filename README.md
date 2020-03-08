# nexustk-logger

Dynamic hooking functions to log outgoing, unencrypted Nexus: The Kingdom of Wind packets

## How to use

1. Use OllyDbg/x32dbg attached to `NexusTK.exe` to find correct address of send function.
2. Update `PATCH_SRC` variable in `process.h` with correct addresses for the given client version.
3. `sh compile.sh`
4. Run `nexustk-logger.exe`

## Notes

This relies on Docker to compile and I've only tested compiling from Mac OS X to Windows.
