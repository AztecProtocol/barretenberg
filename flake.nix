{
  description =
    "Barretenberg: C++ cryptographic library, BN254 elliptic curve library, and PLONK SNARK prover";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.11";

    flake-utils.url = "github:numtide/flake-utils";

  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs {
          inherit system;
        };

        barretenberg.nix_path = ./cpp/barretenberg.nix;

        optional = pkgs.lib.lists.optional;

        crossTargets = builtins.listToAttrs
          (
            [ ] ++ optional (pkgs.hostPlatform.isx86_64 && pkgs.hostPlatform.isLinux) {
              name = "cross-${pkgs.pkgsCross.aarch64-multiplatform-musl.system}";
              value = with pkgs.pkgsCross.aarch64-multiplatform-musl.pkgsLLVM; callPackage barretenberg.nix_path {
                llvmPackages = llvmPackages_11;
              };
            } ++ optional (pkgs.hostPlatform.isx86_64 && pkgs.hostPlatform.isDarwin) {
              name = "cross-${pkgs.pkgsCross.aarch64-darwin.system}";
              value = pkgs.pkgsCross.aarch64-darwin.callPackage barretenberg.nix_path {
                llvmPackages = pkgs.pkgsCross.aarch64-darwin.llvmPackages_11;
              };
            }
          );

        shellDefaults = {
          nativeBuildInputs = [
            pkgs.starship
            # TODO: This installs the wrong version of llvm tools in the wasm32 shell
            pkgs.llvmPackages_11.llvm
          ];

          shellHook = ''
            eval "$(starship init bash)"
            
          '';
        };
      in
      rec {
        packages = {
          llvm11 = pkgs.callPackage barretenberg.nix_path {
            llvmPackages = pkgs.llvmPackages_11;
          };
          llvm12 = pkgs.callPackage barretenberg.nix_path {
            llvmPackages = pkgs.llvmPackages_12;
          };
          llvm13 = pkgs.callPackage barretenberg.nix_path {
            llvmPackages = pkgs.llvmPackages_13;
          };
          llvm14 = pkgs.callPackage barretenberg.nix_path {
            llvmPackages = pkgs.llvmPackages_14;
          };
          llvm15 = pkgs.callPackage barretenberg.nix_path {
            llvmPackages = pkgs.llvmPackages_15;
          };
          wasm32 = pkgs.pkgsCross.wasi32.callPackage barretenberg.nix_path {
            llvmPackages = pkgs.pkgsCross.wasi32.llvmPackages_12;
          };

          default = packages.llvm15;
        } // crossTargets;

        devShells = {
          default = pkgs.mkShell.override { stdenv = packages.default.stdenv; }
            ({
              inputsFrom =
                [ packages.default ];
            } // shellDefaults);

          wasm32 = pkgs.mkShell.override
            {
              stdenv = packages.wasm32.stdenv;
            }
            ({
              inputsFrom = [ packages.wasm32 ];
            } // shellDefaults);
        };
      });
}
