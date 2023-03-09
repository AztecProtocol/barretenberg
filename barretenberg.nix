{ overrideCC, llvmPackages, cmake, lib, callPackage, binaryen, gcc11 }:
let
  # As per https://discourse.nixos.org/t/gcc11stdenv-and-clang/17734/7
  # But it seems to break the wasm build
  # stdenv = overrideCC llvmPackages.stdenv (llvmPackages.clang.override { gccForLibs = gcc11.cc; });
  stdenv = llvmPackages.libcxxStdenv;
  optionals = lib.lists.optionals;
  targetPlatform = stdenv.targetPlatform;
  toolchain_file = ./cpp/cmake/toolchains/${targetPlatform.system}.cmake;
in
stdenv.mkDerivation
{
  pname = "libbarretenberg";
  version = "0.1.0";

  src = ./cpp;

  nativeBuildInputs = [ cmake ]
    ++ optionals targetPlatform.isWasm [ binaryen ];

  buildInputs = [ ]
    ++ optionals (targetPlatform.isDarwin || targetPlatform.isLinux) [
    llvmPackages.openmp
  ];

  cmakeFlags = [
    "-DTESTING=OFF"
    "-DBENCHMARKS=OFF"
    "-DCMAKE_TOOLCHAIN_FILE=${toolchain_file}"
  ]
  ++ optionals (targetPlatform.isDarwin || targetPlatform.isLinux)
    [ "-DCMAKE_BUILD_TYPE=RelWithAssert" ];

  NIX_CFLAGS_COMPILE =
    optionals targetPlatform.isDarwin [ " -fno-aligned-allocation" ];

  enableParallelBuilding = true;
}
