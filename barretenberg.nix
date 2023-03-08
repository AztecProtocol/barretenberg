{ overrideCC, llvmPackages, cmake, lib, callPackage, binaryen, gcc11 }:
let
  optionals = lib.lists.optionals;
  targetPlatform = llvmPackages.stdenv.targetPlatform;
  buildEnv =
    if (targetPlatform.isLinux) then
      overrideCC llvmPackages.stdenv (llvmPackages.clang.override { gccForLibs = gcc11.cc; })
    else
      llvmPackages.stdenv;
  toolchain_file = ./cpp/cmake/toolchains/${targetPlatform.system}.cmake;
in
buildEnv.mkDerivation
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
