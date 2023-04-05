{ stdenv, cmake, ninja, binaryen, fetchzip }:
let
  hostPlatform = stdenv.hostPlatform;
  toolchain_file = ./cpp/cmake/toolchains/wasm32-wasi.cmake;
  WASI_VERSION = "12";
  OS = if hostPlatform.isDarwin then "macos" else if hostPlatform.isLinux then "linux" else "";
  HASH = if hostPlatform.isDarwin then "sha256-zDlPPmH1mtpA66LZ8iS5AVZUWL5DY3v9YLs3qNN7y1U=" else if hostPlatform.isLinux then "sha256-ctDMbtbjFGwRbudp+eONU912trHtI1gyQVtBCmFeIFw=" else "";
  wasisdk = fetchzip {
    url = "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-${WASI_VERSION}/wasi-sdk-${WASI_VERSION}.0-${OS}.tar.gz";
    sha256 = HASH;
  };
in
stdenv.mkDerivation
{
  pname = "barretenberg.wasm";
  version = "0.1.0";

  src = ./cpp;

  nativeBuildInputs = [ cmake ninja binaryen ];

  buildInputs = [ ];

  cmakeFlags = [
    "-DTESTING=OFF"
    "-DBENCHMARKS=OFF"
    "-DCMAKE_TOOLCHAIN_FILE=${toolchain_file}"
    "-DCMAKE_C_COMPILER=${wasisdk}/bin/clang"
    "-DCMAKE_CXX_COMPILER=${wasisdk}/bin/clang++"
    "-DCMAKE_AR=${wasisdk}/bin/llvm-ar"
    "-DCMAKE_RANLIB=${wasisdk}/bin/llvm-ranlib"
    "-DCMAKE_SYSROOT=${wasisdk}/share/wasi-sysroot"
    "-DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER"
    "-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY"
    "-DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY"
    "-DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY"
    "-DCMAKE_C_COMPILER_WORKS=ON"
    "-DCMAKE_CXX_COMPILER_WORKS=ON"
  ];

  enableParallelBuilding = true;
}
