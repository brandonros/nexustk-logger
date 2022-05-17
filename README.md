# nexustk-logger

Dynamic hooking functions to log outgoing, unencrypted Nexus: The Kingdom of Wind packets

## How to use

1. Use OllyDbg/x32dbg attached to `NexusTK.exe` to find correct address of send function (set breakpoint on ws2_32.send and work backwards).
2. Update constants in `process.h` with correct addresses + instructions for the given client version.
3. `./compile.sh`
4. Move `nexustk-logger.dll` to `C:\Program Files (x86)\KRU\NexusTK`
5. Start client.
6. Run `nexustk-logger.exe`

## Notes

This relies on Docker to compile and I've only tested compiling from Mac OS X to Windows.
