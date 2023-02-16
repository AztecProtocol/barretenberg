// Suppress the flurry of warnings caused by using "C" naming conventions
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

// This matches bindgen::Builder output
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

pub mod blake2s;
pub mod composer;
pub mod pedersen;
pub mod pippenger;
pub mod schnorr;
