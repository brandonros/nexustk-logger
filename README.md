# nexustk-logger

Dynamic hooking functions to log outgoing, unencrypted Nexus: The Kingdom of Wind packets

## How to use

1. Use OllyDbg/x32dbg attached to `NexusTK.exe` to find correct address of send function.
2. Update `PATCH_SRC` + `PATCH_DST` variables in `process.h` with correct addresses.
3. `cc dll.c -o nexustk.dll`
4. `cc main.c -o nexustk-logger.exe`
5. Run `nexustk-logger.exe`
