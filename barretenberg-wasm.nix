{ lib, stdenv, cmake, ninja, binaryen, callPackage, debug ? false }:
let
  optionals = lib.lists.optionals;

  toolchain_file = ./cpp/cmake/toolchains/wasm32-wasi.cmake;
  wasi-sdk = callPackage ./wasi-sdk.nix { };
in
stdenv.mkDerivation
{
  pname = if debug then "barretenberg-debug.wasm" else "barretenberg.wasm";
  version = "0.1.0";

  src = ./cpp;

  nativeBuildInputs = [ cmake ninja binaryen wasi-sdk ];

  buildInputs = [ ];

  cmakeFlags = [
    "-DTESTING=OFF"
    "-DBENCHMARKS=OFF"
    "-DCMAKE_TOOLCHAIN_FILE=${toolchain_file}"
    "-DCMAKE_C_COMPILER=${wasi-sdk}/bin/clang"
    "-DCMAKE_CXX_COMPILER=${wasi-sdk}/bin/clang++"
    "-DCMAKE_AR=${wasi-sdk}/bin/llvm-ar"
    "-DCMAKE_RANLIB=${wasi-sdk}/bin/llvm-ranlib"
    "-DCMAKE_SYSROOT=${wasi-sdk}/share/wasi-sysroot"
    "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER"
    "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY"
    "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY"
    "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY"
    "-DCMAKE_C_COMPILER_WORKS=ON"
    "-DCMAKE_CXX_COMPILER_WORKS=ON"
    
  ] ++ (if debug then [ "-DCMAKE_BUILD_TYPE=Debug" ] else [ "-DCMAKE_BUILD_TYPE=Release" ]);
  

  CXXFLAGS = optionals (debug) [ "-ggdb -O1" ];

  enableParallelBuilding = true;
}
