{ overrideCC, stdenv, llvmPackages, cmake, ninja, lib, callPackage, binaryen, gcc11 }:
let
  targetPlatform = stdenv.targetPlatform;
  # As per https://discourse.nixos.org/t/gcc11stdenv-and-clang/17734/7 but it seems to break the wasm build
  # so we check to see if the targetPlatform is wasm and just the llvmPackages.stdenv
  buildEnv =
    if (stdenv.targetPlatform.isWasm) then
      llvmPackages.stdenv
    else
      overrideCC llvmPackages.stdenv (llvmPackages.clang.override { gccForLibs = gcc11.cc; });
  optionals = lib.lists.optionals;
  toolchain_file = ./cpp/cmake/toolchains/${targetPlatform.system}.cmake;
in
buildEnv.mkDerivation
{
  pname = "libbarretenberg";
  version = "0.1.0";

  src = ./cpp;

  nativeBuildInputs = [ cmake ninja ]
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
