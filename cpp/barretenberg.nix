{ lib, callPackage, llvmPackages, cmake, leveldb }:
let
  stdenv = llvmPackages.stdenv;
  optionals = lib.lists.optionals;
  targetPlatform = stdenv.targetPlatform;
  toolchain_file = ./cmake/toolchains/${targetPlatform.system}.cmake;
in
llvmPackages.stdenv.mkDerivation {
  pname = "libbarretenberg";
  version = "0.1.0";

  src = ./.;

  nativeBuildInputs = [ cmake ]
    ++ optionals targetPlatform.isWasm [ (callPackage ./wasilibc.nix { }) ];

  buildInputs = [ ]
    ++ optionals (targetPlatform.isDarwin || targetPlatform.isLinux) [
    llvmPackages.openmp
    leveldb
  ];

  cmakeFlags = [
    "-DTESTING=OFF"
    "-DBENCHMARKS=OFF"
    # "-DCMAKE_TOOLCHAIN_FILE=${toolchain_file}"
  ]
  ++ optionals (targetPlatform.isDarwin || targetPlatform.isLinux)
    [ "-DCMAKE_BUILD_TYPE=RelWithAssert" ];

  NIX_CFLAGS_COMPILE =
    optionals targetPlatform.isDarwin [ " -fno-aligned-allocation" ]
    ++ optionals targetPlatform.isWasm [ "-D_WASI_EMULATED_PROCESS_CLOCKS" ];

  NIX_LDFLAGS =
    optionals targetPlatform.isWasm [ "-lwasi-emulated-process-clocks" ];

  enableParallelBuilding = true;

  installPhase =
    if (targetPlatform.isWasm) then ''
      mkdir -p $out/bin
      cp -a bin/. $out/bin
    '' else ''
      mkdir -p $out/lib
      mkdir -p $out/include
      find src -name \*.a -exec cp {} $out/lib \;
      cd $src/src
      find aztec \( -name "*.hpp" -o -name "*.h" \) -exec cp --parents --no-preserve=mode,ownership {} $out/include \;
    '';
}
