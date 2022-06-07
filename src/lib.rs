use winapi::um::winnt::{DLL_PROCESS_ATTACH, LPCWSTR};

detour::static_detour! {
  static BuildPacketHook: fn(*u8, u16);
}

fn our_BuildPacket(input: u8, input_size: u16) {
  let input_slice = unsafe { std::slice::from_raw_parts(input, input_size as usize) };
  for i in 0..input_size as usize {
    print!("{:02x}", input_slice[i]);
  }
  println!("");
  BuildPacketHook.call(input, input_size);
}

// entry point
#[no_mangle]
#[allow(non_snake_case, unused_variables)]
pub extern "system" fn DllMain(dll_module: winapi::HINSTANCE, call_reason: winapi::DWORD, reserved: winapi::LPVOID) -> winapi::BOOL {
  if (call_reason == DLL_PROCESS_ATTACH) {
    unsafe { kernel32::AllocConsole() };
    println!("Initializing...");
    let client_buildPacket: usize = 0x00576660;
    unsafe { BuildPacketHook.initialize(client_buildPacket, our_BuildPacket).unwrap() };
    unsafe { BuildPacketHook.enable().unwrap() };
  }
  return winapi::TRUE;
}
