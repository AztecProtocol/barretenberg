use std::{env, path::PathBuf, process::Command};
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
const INTEL_APPLE: &str = "x86_64-darwin";
const INTEL_LINUX: &str = "x86_64-linux-clang";
const ARM_APPLE: &str = "aarch64-darwin";
const ARM_LINUX: &str = "aarch64-linux";

const CC_ENV_KEY: &&str = &"CC";
const CXX_ENV_KEY: &&str = &"CXX";

const BINGDEN_ENV_KEY: &str = "BINDGEN_EXTRA_CLANG_ARGS";

fn select_toolchain() -> &'static str {
    let arch = select_arch();
    let os = select_os();
    match (os, arch) {
        (OS::Linux, Arch::X86_64) => INTEL_LINUX,
        (OS::Linux, Arch::Arm) => ARM_LINUX,
        (OS::Apple, Arch::X86_64) => INTEL_APPLE,
        (OS::Apple, Arch::Arm) => ARM_APPLE,
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

fn which_clang(clang_command: &'static str) -> Option<String> {
    let which_clang_command = Command::new("which")
        .arg(clang_command)
        .output()
        .expect("Failed to execute which clang command");

    if which_clang_command.status.success() {
        let path =
            String::from_utf8(which_clang_command.stdout).expect("Invalid UTF-8 output from which");
        Some(path.trim().to_owned())
    } else {
        None
    }
}

fn set_compiler(toolchain: &'static str) {
    match toolchain {
        INTEL_APPLE | ARM_APPLE => {
            env::set_var(
                CC_ENV_KEY,
                format!("{}/opt/llvm/bin/clang", find_brew_prefix()),
            );
            env::set_var(
                CXX_ENV_KEY,
                format!("{}/opt/llvm/bin/clang++", find_brew_prefix()),
            );
        }
        INTEL_LINUX | ARM_LINUX => {
            if let Ok(cc) = env::var(CC_ENV_KEY) {
                println!("Using environment defined compiler $CC={cc}")
            } else {
                env::set_var(
                    CC_ENV_KEY,
                    which_clang("clang").expect("No clang found in $CC or $PATH, set $CC or $PATH to contain clang compier v.10..14"),
                );
                env::set_var(
                    CXX_ENV_KEY,
                    which_clang("clang++").expect("No clang found in $CXX or $PATH, set $CXX or $PATH to contain clang compier v.10..14"),
                );
            }
        }
        &_ => unimplemented!("Finding compiler for toolchain {} failed, ensure clanng v10..14 installed, and $CC, $CXX are set accordingly", toolchain),
    }
}

fn main() {
    // TODO: Passing value like that is consistent with cargo but feels hacky from nix perspective

    let bindgen_flags = env::var(BINGDEN_ENV_KEY).unwrap_or_default();

    // Link C++ std lib
    println!("cargo:rustc-link-lib={}", select_cpp_stdlib());

    let bindings = if bindgen_flags.is_empty() {
        println!(
            "cargo:info={BINGDEN_ENV_KEY} environment variable not set. Using fixed Barretenberg path `../cpp`"
        );
        // Builds the project in ../barretenberg into dst
        println!("cargo:rerun-if-changed=../cpp");

        // Select toolchain
        let toolchain = select_toolchain();

        // Set brew environment variable if needed
        // TODO: We could check move this to a bash script along with
        // TODO: checks that check that all the necessary dependencies are
        // TODO installed via llvm
        set_compiler(toolchain);

        let dst = cmake::Config::new("../cpp")
            .very_verbose(true)
            .cxxflag("-fPIC")
            .cxxflag("-fPIE")
            .env("NUM_JOBS", num_cpus::get().to_string())
            .define(
                "CMAKE_TOOLCHAIN_FILE",
                format!("./cmake/toolchains/{toolchain}.cmake"),
            )
            .define("TESTING", "OFF")
            .always_configure(false)
            .build();

        // Manually link all of the libraries

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
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/crypto/blake3s",
            dst.display()
        );

        println!(
            "cargo:rustc-link-search={}/build/src/aztec/env",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/crypto/pedersen",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/ecc",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/crypto/keccak",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/crypto/schnorr",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/dsl",
            dst.display()
        );

        println!(
            "cargo:rustc-link-search={}/build/src/aztec/plonk/",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/polynomials/",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/srs/",
            dst.display()
        );
        println!(
            "cargo:rustc-link-search={}/build/src/aztec/numeric/",
            dst.display()
        );

        println!(
            "cargo:rustc-link-search={}/build/src/aztec/honk",
            dst.display()
        );

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

        println!(
            "cargo:rustc-link-search={}/build/src/aztec/rollup/proofs/standard_example",
            dst.display()
        );

        // Generate bindings from a header file and place them in a bindings.rs file
        bindgen::Builder::default()
            // Clang args so that we can use relative include paths
            .clang_args(&["-I../barretenberg/src/aztec", "-I../..", "-I../", "-xc++"])
            .header("../barretenberg/src/aztec/bb/bb.hpp")
            .generate()
            .expect("Unable to generate bindings")
    } else {
        bindgen::Builder::default()
            // Clang args so that we can use relative include paths
            .clang_args(&["-xc++"])
            .header_contents("wrapper.h", "#include <aztec/bb/bb.hpp>")
            .generate()
            .expect("Unable to generate bindings")
    };

    println!("cargo:rustc-link-lib=static=crypto_blake2s");
    println!("cargo:rustc-link-lib=static=crypto_blake3s");
    println!("cargo:rustc-link-lib=static=env");
    println!("cargo:rustc-link-lib=static=crypto_pedersen");
    println!("cargo:rustc-link-lib=static=ecc");
    println!("cargo:rustc-link-lib=static=crypto_keccak");
    println!("cargo:rustc-link-lib=static=crypto_schnorr");
    println!("cargo:rustc-link-lib=static=dsl");
    println!("cargo:rustc-link-lib=static=plonk");
    println!("cargo:rustc-link-lib=static=polynomials");
    println!("cargo:rustc-link-lib=static=srs");
    println!("cargo:rustc-link-lib=static=numeric");
    println!("cargo:rustc-link-lib=static=honk");

    println!("cargo:rustc-link-lib=static=stdlib_primitives");
    println!("cargo:rustc-link-lib=static=stdlib_sha256");
    println!("cargo:rustc-link-lib=static=stdlib_blake2s");
    println!("cargo:rustc-link-lib=static=stdlib_schnorr");
    println!("cargo:rustc-link-lib=static=stdlib_pedersen");

    println!("cargo:rustc-link-lib=static=rollup_proofs_standard_example");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings");
}

fn link_lib_omp(toolchain: &'static str) {
    // We are using clang, so we need to tell the linker where to search for lomp
    match toolchain {
        INTEL_LINUX | ARM_LINUX => {
            let llvm_dir = match env::var("LIBCLANG_PATH") {
                Ok(lib_clang_var) => {
                    println!("cargo:warning=Using LIBCLANG_PATH={lib_clang_var} from environment");
                    lib_clang_var
                }
                Err(_) => find_llvm_lib_path(),
            };
            // .ok()
            // .map(String::from)
            // .unwrap_or_else(|| find_llvm_lib_path());
            println!("cargo:rustc-link-search={llvm_dir}")
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

fn find_llvm_lib_path() -> String {
    // // Most linux systems will have the `find` application
    // //
    // // This assumes that there is a single llvm-X folder in /usr/lib
    // let output = std::process::Command::new("sh")
    //     .arg("-c")
    //     .arg("find /usr/lib -type d -name \"*llvm-*\" -print -quit")
    //     .stdout(std::process::Stdio::piped())
    //     .output()
    //     .expect("Failed to execute command to run `find`");
    // // This should be the path to llvm
    // let path_to_llvm = String::from_utf8(output.stdout).unwrap();
    // path_to_llvm.trim().to_owned()
    let default_clang_path = "/usr/bin/clang";
    let clang_path = env::var("CMAKE_CXX_COMPILER")
        .ok()
        .map(PathBuf::from)
        .unwrap_or_else(|| {
            let output = Command::new("which").arg("clang++").output().ok();

            if let Some(output) = output {
                if output.status.success() {
                    let path = String::from_utf8_lossy(&output.stdout).trim().to_string();
                    println!("cargo:warning=Will use {path} found on system Path!");
                    PathBuf::from(path)
                } else {
                    println!(
                        "cargo:warning=Falling back to default clang path `{default_clang_path}`"
                    );
                    PathBuf::from(default_clang_path)
                }
            } else {
                println!("cargo:warning=Falling back to default clang path `{default_clang_path}`");
                PathBuf::from(default_clang_path)
            }
        });

    let clang_dir = clang_path.parent().unwrap();

    println!(
        "cargo:warning=Will use {} found on system Path!",
        clang_dir.as_os_str().to_str().unwrap()
    );

    let lib_dir = clang_dir.join("../lib").canonicalize().unwrap();

    if lib_dir.exists() {
        let use_lib = String::from(lib_dir.as_os_str().to_str().unwrap());
        println!("cargo:warning=Will use {use_lib}");
        use_lib
    } else {
        panic!("Could not identify Clang path, use export CMAKE_CXX_COMPILER=<Path>");
    }
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
