# https://github.com/ereslibre/nixities/blob/2c60af777fc863f90e6e4eeffcf3465def93a1f3/packages/wasi-sdk/default.nix
{ lib, pkgs, stdenv }:
let
  pname = "wasi-sdk";
  version = "12";
in
pkgs.stdenv.mkDerivation {
  inherit pname version;

  sourceRoot = "${pname}-${version}.0";
  dontBuild = true;
  dontConfigure = true;
  dontStrip = true;

  nativeBuildInputs =
    lib.optional stdenv.isLinux (with pkgs; [ autoPatchelfHook ]);

  installPhase = ''
    mkdir -p $out/{bin,lib,share}
    mv bin/* $out/bin/
    mv lib/* $out/lib/
    mv share/* $out/share/
  '';

  src =
    let
      mapSystem = system:
        if system == "x86_64-linux" then {
          tarballSuffix = "linux";
          hash = "sha256-+kdpTXW/b86Y++eScZMpiyXuA9reJ/ykU9fdUwN4lzo=";
        } else {
          tarballSuffix = "macos";
          hash = "sha256-juJfnD/eYY/upcV62tOFFSYmeEtra1L7Vj5e2DK/U+8=";
        };
    in
    (if builtins.elem stdenv.hostPlatform.system [
      "x86_64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ] then
      let system = mapSystem stdenv.hostPlatform.system;
      in pkgs.fetchurl {
        url =
          "https://github.com/WebAssembly/${pname}/releases/download/${pname}-${version}/${pname}-${version}.0-${system.tarballSuffix}.tar.gz";
        hash = system.hash;
      }
    else
      throw "unsupported system");

  meta = { platforms = [ "x86_64-linux" "x86_64-darwin" "aarch64-darwin" ]; };
}
