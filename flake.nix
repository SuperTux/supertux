# SuperTux
# Copyright (C) 2021 Ingo Ruhnke <grumbel@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Due to the use of submodules this has to be build with:
#
#     nix build "git+file://$(pwd)?submodules=1"
#
# See: https://github.com/NixOS/nix/pull/5434
{
  description = "A 2D platform game featuring Tux the penguin";

  inputs = rec {
    nixpkgs.url = "github:nixos/nixpkgs";
    nix.inputs.nixpkgs.follows = "nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nix, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree rec {
          raqm = pkgs.stdenv.mkDerivation rec {
            pname = "libraqm";
            version = "0.7.2";
            src = fetchTarball {
              url = "https://github.com/HOST-Oman/libraqm/releases/download/v${version}/raqm-${version}.tar.xz";
              sha256 = "1shcs5l27l7380dvacvhl8wrdq3lix0wnhzvfdh7vx2pkzjs3zk6";
            };
            nativeBuildInputs = [
              pkgs.meson
              pkgs.cmake
              pkgs.ninja
              pkgs.gcc
              pkgs.pkgconfig
              pkgs.python3
            ];
            buildInputs = [
              pkgs.freetype
              pkgs.harfbuzz
              pkgs.fribidi
            ];
            propagatedBuildInputs = [
              pkgs.glib
              pkgs.pcre
            ];
          };

          supertux2 = pkgs.stdenv.mkDerivation rec {
            pname = "supertux2";
            # FIXME: Should use `git describe` to get the version
            # number or leave it to cmake, but the .git/ directory
            # isn't included in the Nix store.
            version = "0.6.3-unknown-" + (if (self ? shortRev) then self.shortRev else "dirty");
            src = nixpkgs.lib.cleanSource ./.;
            postPatch = let
              ver = builtins.splitVersion version;
            in ''cat > version.cmake <<EOF
SET(SUPERTUX_VERSION_MAJOR ${builtins.elemAt ver 0})
SET(SUPERTUX_VERSION_MINOR ${builtins.elemAt ver 1})
SET(SUPERTUX_VERSION_PATCH ${builtins.elemAt ver 2})
SET(SUPERTUX_VERSION_TWEAK )
SET(SUPERTUX_VERSION_STRING "v${version}")
SET(SUPERTUX_VERSION_BUILD "${builtins.elemAt ver 4}")
EOF
'';
            cmakeFlags = [ "-DINSTALL_SUBDIR_BIN=bin" ];
            enableParallelBuilding = true;
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.gcc
              pkgs.pkgconfig
              pkgs.makeWrapper
              pkgs.git
            ];
            postFixup = ''
                wrapProgram $out/bin/supertux2 \
                  --prefix LIBGL_DRIVERS_PATH ":" "${pkgs.mesa.drivers}/lib/dri" \
                  --prefix LD_LIBRARY_PATH ":" "${pkgs.mesa.drivers}/lib"
            '';
            buildInputs = [
              raqm

              pkgs.boost
              pkgs.curl
              pkgs.fribidi
              pkgs.harfbuzz
              pkgs.freetype
              pkgs.libGL
              pkgs.libGLU
              pkgs.glew
              pkgs.gtest

              pkgs.glm
              pkgs.SDL2
              pkgs.SDL2_image

              pkgs.openal
              pkgs.libvorbis
              pkgs.libogg
              pkgs.gtest
            ];
           };
        };
        defaultPackage = packages.supertux2;
      }
    );
}
