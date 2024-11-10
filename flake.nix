{
  description = "Knobbox";

  # Declare inputs to fetch from
  inputs = {
    nixpkgs = {
      url = "github:NixOS/nixpkgs/nixpkgs-unstable";
      flake = true;
    };
  };

  # Declare your Nix environment
  outputs = { self, nixpkgs, flake-utils }: 
    let
     forAllSystems = function:
      nixpkgs.lib.genAttrs [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ] (system: function nixpkgs.legacyPackages.${system});
    in {
      devShell = forAllSystems (pkgs: 
          with pkgs; mkShell {
            buildInputs = [
              arduino-ide
              arduino-cli
              rustup
              cargo
              rust-analyzer
              pkg-config
              libudev-zero
              python3
              python312Packages.protobuf
              python312Packages.grpcio-tools
            ]; 

            shellHook = ''
              export ENV_NAME=buttonbox
              exec $SHELL
            '';
          });
    };
}

