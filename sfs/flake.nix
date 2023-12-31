{
  inputs = {
    nixpkgs.url = "github:NickCao/nixpkgs/nixos-unstable-small";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let pkgs = import nixpkgs { inherit system; }; in
        {
          devShells.default = pkgs.mkShell {
            nativeBuildInputs = with pkgs;[ rustc cargo rustfmt rust-analyzer pkg-config rustPlatform.bindgenHook ];
            buildInputs = with pkgs;[ fuse3 ];
          };
        }
      );
}
