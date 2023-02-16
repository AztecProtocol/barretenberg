use std::{env, path::PathBuf};
// These are the operating systems that are supported
pub enum OS {
    Linux,
    Apple,
}
// These are the supported architectures
pub enum Arch {
    X86_64,
    Arm,
}
// These constants correspond to the filenames in cmake/toolchains
//
// There are currently no toolchain for windows, please use WSL
const INTEL_APPLE: &str = "x86_64-apple-clang";
const INTEL_LINUX: &str = "x86_64-linux-clang";
const ARM_APPLE: &str = "arm-apple-clang";
const ARM_LINUX: &str = "aarch64-linux-clang";

fn select_toolchain() -> &'static str {
    let arch = select_arch();
    let os = select_os();
    match (os, arch) {
        (OS::Linux, Arch::X86_64) => INTEL_LINUX,
        (OS::Linux, Arch::Arm) => INTEL_LINUX,
        (OS::Apple, Arch::X86_64) => INTEL_LINUX,
        (OS::Apple, Arch::Arm) => INTEL_LINUX,
    }
}
fn select_arch() -> Arch {
    let arch = std::env::consts::ARCH;
    match arch {
        "arm" => Arch::Arm,
        "aarch64" => Arch::Arm,
        "x86_64" => Arch::X86_64,
        _ => {
            // For other arches, we default to x86_64
            Arch::X86_64
        }
    }
}
fn select_os() -> OS {
    let os = std::env::consts::OS;
    match os {
        "linux" => OS::Linux,
        "macos" => OS::Apple,
        "windows" => unimplemented!("windows is not supported"),
        _ => {
            // For other OS's we default to linux
            OS::Linux
        }
    }
}
fn select_cpp_stdlib() -> &'static str {
    // The name of the c++ stdlib depends on the OS
    match select_os() {
        OS::Linux => "stdc++",
        OS::Apple => "c++",
    }
}
fn set_brew_env_var(toolchain: &'static str) {
    // The cmake file for macos uses an environment variable
    // to figure out where to find certain programs installed via brew
    if toolchain == INTEL_APPLE || toolchain == ARM_APPLE {
        env::set_var("BREW_PREFIX", find_brew_prefix());
    }
}

fn main() {
    // Builds the project in ../cpp into dst
    println!("cargo:rerun-if-changed=../cpp");

    // env::set_var("LLVM_CONFIG_PATH", "/usr/bin/llvm-config-15");
    // env::set_var("LIBCLANG_PATH", "/usr/lib/llvm-15/lib");
    // env::set_var("LIBCLANG_STATIC_PATH", "/usr/lib/llvm-15/lib");
    // env::set_var("CLANG_PATH", "/usr/bin/clang-15");
    // env::set_var("CMAKE_C_COMPILER", "/usr/bin/clang-15");
    // env::set_var("CMAKE_CXX_COMPILER", "/usr/bin/clang++-15");

    // Select toolchain
    let toolchain = select_toolchain();

    println!("toolchain = {}", toolchain);
    // Set brew environment variable if needed
    // TODO: We could check move this to a bash script along with
    // TODO: checks that check that all the necessary dependencies are
    // TODO installed via llvm
    set_brew_env_var(toolchain);

    let dst = cmake::Config::new("../cpp")
        .very_verbose(true)
        .cxxflag("-fPIC")
        .cxxflag("-fPIE")
        .env("NUM_JOBS", num_cpus::get().to_string())
        .define("TOOLCHAIN", toolchain)
        .define("TESTING", "ON")
        .define("BENCHMARKS", "OFF")
        .define("DISABLE_ASM", "ON")
        .define("DISABLE_ADX", "ON")
        .define("CMAKE_C_COMPILER", "/usr/bin/clang-15")
        .define("CMAKE_CXX_COMPILER", "/usr/bin/clang++-15")
        .always_configure(true)
        .build();

    // Manually link all of the libraries

    // Link C++ std lib
    println!("cargo:rustc-link-lib={}", select_cpp_stdlib());
    // Link lib OpenMP
    link_lib_omp(toolchain);

    // println!(
    //     "cargo:rustc-link-search={}/build/src/aztec/bb",
    //     dst.display()
    // );
    // println!("cargo:rustc-link-lib=static=bb");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/blake2s",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=crypto_blake2s");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/env",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=env");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/pedersen",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=crypto_pedersen");
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/ecc",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=ecc");
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/keccak",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=crypto_keccak");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/crypto/schnorr",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=crypto_schnorr");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/dsl",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=dsl");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/plonk/",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=plonk");
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/polynomials/",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=polynomials");
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/srs/",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=srs");
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/numeric/",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=numeric");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/honk",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=honk");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/primitives",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/hash/sha256",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/hash/blake2s",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/encryption/schnorr",
        dst.display()
    );
    println!(
        "cargo:rustc-link-search={}/build/src/aztec/stdlib/hash/pedersen",
        dst.display()
    );

    println!("cargo:rustc-link-lib=static=stdlib_primitives");
    println!("cargo:rustc-link-lib=static=stdlib_sha256");
    println!("cargo:rustc-link-lib=static=stdlib_blake2s");
    println!("cargo:rustc-link-lib=static=stdlib_schnorr");
    println!("cargo:rustc-link-lib=static=stdlib_pedersen");

    println!(
        "cargo:rustc-link-search={}/build/src/aztec/rollup/proofs/standard_example",
        dst.display()
    );
    println!("cargo:rustc-link-lib=static=rollup_proofs_standard_example");

    // Generate bindings from a header file and place them in a bindings.rs file
    let bindings = bindgen::Builder::default()
        // Clang args so that we can use relative include paths
        .clang_args(&["-I../cpp/src/aztec", "-I../..", "-I../", "-xc++20"])
        .header("../cpp/src/aztec/bb/bb.hpp")
        .generate()
        .expect("Unable to generate bindings");
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings");
}

fn link_lib_omp(toolchain: &'static str) {
    // We are using clang, so we need to tell the linker where to search for lomp
    match toolchain {
        INTEL_LINUX | ARM_LINUX => {
            let llvm_dir = find_llvm_linux_path();
            println!("cargo:rustc-link-search={llvm_dir}/lib")
        }
        INTEL_APPLE => {
            let brew_prefix = find_brew_prefix();
            println!("cargo:rustc-link-search={brew_prefix}/opt/libomp/lib")
        }
        ARM_APPLE => {
            let brew_prefix = find_brew_prefix();
            println!("cargo:rustc-link-search={brew_prefix}/opt/libomp/lib")
        }
        &_ => unimplemented!("lomp linking of {toolchain} is not supported"),
    }
    match toolchain {
        ARM_LINUX | INTEL_APPLE | ARM_APPLE => {
            println!("cargo:rustc-link-lib=omp")
        }
        &_ => println!("cargo:rustc-link-lib=omp5"),
    }
}

fn find_llvm_linux_path() -> String {
    // Most linux systems will have the `find` application
    //
    // This assumes that there is a single llvm-X folder in /usr/lib
    let output = std::process::Command::new("sh")
        .arg("-c")
        .arg("find /usr/lib -type d -name \"*llvm-*\" -print -quit")
        .stdout(std::process::Stdio::piped())
        .output()
        .expect("Failed to execute command to run `find`");
    // This should be the path to llvm
    let path_to_llvm = String::from_utf8(output.stdout).unwrap();
    path_to_llvm.trim().to_owned()
}

fn find_brew_prefix() -> String {
    let output = std::process::Command::new("brew")
        .arg("--prefix")
        .stdout(std::process::Stdio::piped())
        .output()
        .expect("Failed to execute command to run `brew --prefix` is brew installed?");

    let stdout = String::from_utf8(output.stdout).unwrap();

    stdout.trim().to_string()
}
