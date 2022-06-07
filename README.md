# nexustk-logger

Dynamic hooking functions to log outgoing, unencrypted Nexus: The Kingdom of Wind packets

## How to use

1. Use OllyDbg/x32dbg attached to `NexusTK.exe` to find correct address of send function (set breakpoint on ws2_32.send/wsock32.recv and work backwards).
2. `cargo +stable-i686-pc-windows-msvc build`
3. Inject DLL from `target/debug/nexustk_rs.dll`
