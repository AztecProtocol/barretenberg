{
  description = "Build Barretenberg wrapper";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

    crane = {
      url = "github:ipetkov/crane";
      inputs.nixpkgs.follows = "nixpkgs";
    };

    flake-utils.url = "github:numtide/flake-utils";

    rust-overlay = {
      url = "github:oxalica/rust-overlay";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        flake-utils.follows = "flake-utils";
      };
    };

    libbarretenberg_pkgs = {
      url = "path:./../";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    { self, nixpkgs, crane, flake-utils, rust-overlay, libbarretenberg_pkgs, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
          overlays = [ (import rust-overlay) ];
        };


        craneLib = (crane.mkLib pkgs).overrideScope' (final: prev: {
          stdenv = pkgs.llvmPackages.stdenv;
        });

        # TODO: line below looks terrible we can do better naming here in referenced flake
        libbarretenberg = libbarretenberg_pkgs.packages.${system}.default;

        barretenberg_wrapper_crate = craneLib.buildPackage {
          src = craneLib.cleanCargoSource ./.;

          doCheck = false;

          # Bindegn needs these
          LIBCLANG_PATH = "${pkgs.llvmPackages.libclang.lib}/lib";
          BINDGEN_EXTRA_CLANG_ARGS = "-I${libbarretenberg}/include/aztec -L${libbarretenberg}";
          RUSTFLAGS = "-L${libbarretenberg}/lib -lomp";

          buildInputs = [
            pkgs.llvmPackages.openmp
            libbarretenberg
          ] ++ pkgs.lib.optionals pkgs.stdenv.isDarwin [
            pkgs.libiconv
          ];
        };
      in rec {
        checks = { inherit barretenberg_wrapper_crate; };

        packages.default = barretenberg_wrapper_crate;

        # apps.default = flake-utils.lib.mkApp {
        #   drv = pkgs.writeShellScriptBin "barretenberg_wrapper" ''
        #     ${my-crate}/bin/barretenberg_wrapper
        #   '';
        # };

        apps.default = flake-utils.lib.mkApp { drv = barretenberg_wrapper_crate; };

        devShells.default = pkgs.mkShell {
          inputsFrom = builtins.attrValues self.checks;

          buildInputs = packages.default.buildInputs ;

          BINDGEN_EXTRA_CLANG_ARGS = "-I${libbarretenberg}/include/aztec -L${libbarretenberg}";

          nativeBuildInputs = with pkgs; [
            cargo
            rustc ];
        };
      });
}
