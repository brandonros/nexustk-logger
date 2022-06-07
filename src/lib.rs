#![feature(abi_thiscall)]

use std::ffi::c_void;
use detour::static_detour;
use winapi::um::winnt::{DLL_PROCESS_ATTACH};
use winapi::shared::minwindef::{HINSTANCE, DWORD, BOOL, TRUE, LPVOID};

static_detour! {
  static CryptOutgoingPacketHook: unsafe extern "thiscall" fn(*const c_void, *const u8, u16);
}

type FnCryptOutgoingPacket = unsafe extern "thiscall" fn(*const c_void, *const u8, u16);

fn our_cryptOutgoingPacket(this: *const c_void, input: *const u8, input_size: u16) {
  let input_slice = unsafe { std::slice::from_raw_parts(input, input_size as usize) };
  for i in 0..input_size as usize {
    print!("{:02x}", input_slice[i]);
  }
  println!("");
  unsafe { CryptOutgoingPacketHook.disable().unwrap() };
  unsafe { CryptOutgoingPacketHook.call(this, input, input_size) };
  unsafe { CryptOutgoingPacketHook.enable().unwrap() };
}

// entry point
#[no_mangle]
#[allow(non_snake_case, unused_variables)]
pub extern "system" fn DllMain(dll_module: HINSTANCE, call_reason: DWORD, reserved: LPVOID) -> BOOL {
  if call_reason == DLL_PROCESS_ATTACH {
    unsafe { winapi::um::consoleapi::AllocConsole() };
    println!("Initializing...");
    let client_cryptOutgoingPacket: FnCryptOutgoingPacket = unsafe { std::mem::transmute(0x00576660) };
    unsafe { CryptOutgoingPacketHook.initialize(client_cryptOutgoingPacket, our_cryptOutgoingPacket).unwrap() };
    unsafe { CryptOutgoingPacketHook.enable().unwrap() };
  }
  return TRUE;
}
