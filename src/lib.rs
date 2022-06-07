// cargo +nightly-i686-pc-windows-msvc build

#![feature(abi_thiscall)]

use std::ffi::c_void;
use detour::static_detour;
use winapi::um::winnt::{DLL_PROCESS_ATTACH};
use winapi::shared::minwindef::{HINSTANCE, DWORD, BOOL, TRUE, LPVOID};

static_detour! {
  static CryptIncomingPacketHook: unsafe extern "stdcall" fn(*const u8, usize, *const u8, usize) -> usize;
  static CryptOutgoingPacketHook: unsafe extern "thiscall" fn(*const c_void, *const u8, u16);
}

type FnCryptIncomingPacket = unsafe extern "stdcall" fn(*const u8, usize, *const u8, usize) -> usize;
type FnCryptOutgoingPacket = unsafe extern "thiscall" fn(*const c_void, *const u8, u16);

fn our_cryptOutgoingPacket(this: *const c_void, input: *const u8, input_size: u16) {
  print!("our_cryptOutgoingPacket: ");
  let input_slice = unsafe { std::slice::from_raw_parts(input, input_size as usize) };
  for i in 0..input_size as usize {
    print!("{:02x}", input_slice[i]);
  }
  println!("");
  unsafe { CryptOutgoingPacketHook.disable().unwrap() };
  unsafe { CryptOutgoingPacketHook.call(this, input, input_size) };
  unsafe { CryptOutgoingPacketHook.enable().unwrap() };
}

fn our_cryptIncomingPacket(input: *const u8, packet_size: usize, output: *const u8, crypt_type: usize) -> usize {
  print!("our_cryptIncomingPacket: input = {:p} packet_size = {} output = {:p} crypt_type = {} ", input, packet_size, output, crypt_type);
  let input_slice = unsafe { std::slice::from_raw_parts(input, packet_size as usize) };
  let output_slice = unsafe { std::slice::from_raw_parts(output, packet_size as usize) };
  unsafe { CryptIncomingPacketHook.disable().unwrap() };
  let ret_val = unsafe { CryptIncomingPacketHook.call(input, packet_size, output, crypt_type) };
  unsafe { CryptIncomingPacketHook.enable().unwrap() };
  for i in 0..packet_size as usize {
    print!("{:02x}", output_slice[i]);
  }
  println!("");
  return ret_val;
}

// entry point
#[no_mangle]
#[allow(non_snake_case, unused_variables)]
pub extern "system" fn DllMain(dll_module: HINSTANCE, call_reason: DWORD, reserved: LPVOID) -> BOOL {
  if call_reason == DLL_PROCESS_ATTACH {
    unsafe { winapi::um::consoleapi::AllocConsole() };
    println!("Initializing...");
    // outgoing
    let client_cryptOutgoingPacket: FnCryptOutgoingPacket = unsafe { std::mem::transmute(0x00576660) };
    unsafe { CryptOutgoingPacketHook.initialize(client_cryptOutgoingPacket, our_cryptOutgoingPacket).unwrap() };
    unsafe { CryptOutgoingPacketHook.enable().unwrap() };
    // incoming
    let client_cryptIncomingPacket: FnCryptIncomingPacket = unsafe { std::mem::transmute(0x00578b20) };
    unsafe { CryptIncomingPacketHook.initialize(client_cryptIncomingPacket, our_cryptIncomingPacket).unwrap() };
    unsafe { CryptIncomingPacketHook.enable().unwrap() };
  }
  return TRUE;
}
