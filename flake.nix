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

{
  description = "A 2D platform game featuring Tux the penguin";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    sexpcpp.url = "github:lispparser/sexp-cpp";
    sexpcpp.inputs.nixpkgs.follows = "nixpkgs";
    sexpcpp.inputs.flake-utils.follows = "flake-utils";
    sexpcpp.inputs.tinycmmc.follows = "tinycmmc";

    tinygettext.url = "github:tinygettext/tinygettext";
    tinygettext.inputs.nixpkgs.follows = "nixpkgs";
    tinygettext.inputs.flake-utils.follows = "flake-utils";
    tinygettext.inputs.tinycmmc.follows = "tinycmmc";

    curl-win32.url = "github:grumnix/curl-win32";
    curl-win32.inputs.nixpkgs.follows = "nixpkgs";
    curl-win32.inputs.tinycmmc.follows = "tinycmmc";

    physfs-win32.url = "github:grumnix/physfs-win32";
    physfs-win32.inputs.nixpkgs.follows = "nixpkgs";
    physfs-win32.inputs.tinycmmc.follows = "tinycmmc";

    SDL2-win32.url = "github:grumnix/SDL2-win32";
    SDL2-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL2-win32.inputs.tinycmmc.follows = "tinycmmc";

    SDL2_image-win32.url = "github:grumnix/SDL2_image-win32";
    SDL2_image-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL2_image-win32.inputs.tinycmmc.follows = "tinycmmc";

    freetype-win32.url = "github:grumnix/freetype-win32";
    freetype-win32.inputs.nixpkgs.follows = "nixpkgs";
    freetype-win32.inputs.flake-utils.follows = "flake-utils";

    SDL2_ttf.url = "github:SuperTux/SDL_ttf";
    SDL2_ttf.inputs.nixpkgs.follows = "nixpkgs";
    SDL2_ttf.inputs.flake-utils.follows = "flake-utils";

    SDL2_ttf-win32.url = "github:grumnix/SDL2_ttf-win32";
    SDL2_ttf-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL2_ttf-win32.inputs.flake-utils.follows = "flake-utils";
    SDL2_ttf-win32.inputs.tinycmmc.follows = "tinycmmc";

    wstsound.url = "github:WindstilleTeam/wstsound";
    wstsound.inputs.nixpkgs.follows = "nixpkgs";
    wstsound.inputs.flake-utils.follows = "flake-utils";
    wstsound.inputs.tinycmmc.follows = "tinycmmc";

    squirrel.url = "github:grumnix/squirrel";
    squirrel.inputs.nixpkgs.follows = "nixpkgs";
    squirrel.inputs.tinycmmc.follows = "tinycmmc";

    glew-win32.url = "github:grumnix/glew-win32";
    glew-win32.inputs.nixpkgs.follows = "nixpkgs";
    glew-win32.inputs.tinycmmc.follows = "tinycmmc";
  };

  outputs = { self, nixpkgs, flake-utils,
              tinycmmc, sexpcpp, tinygettext,
              curl-win32,
              SDL2-win32, SDL2_image-win32, freetype-win32, physfs-win32, SDL2_ttf, SDL2_ttf-win32,
              wstsound, squirrel, glew-win32 }:

    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = supertux2;

          supertux2 = pkgs.stdenv.mkDerivation rec {
            pname = "supertux2";
            # FIXME: Should use `git describe` to get the version
            # number or leave it to cmake, but the .git/ directory
            # isn't included in the Nix store.
            version = "0.6.3-${nixpkgs.lib.substring 0 8 self.lastModifiedDate}-${self.shortRev or "dirty"}";

            src = nixpkgs.lib.cleanSource ./.;

            patchPhase = let
              ver = builtins.splitVersion version;
            in ''
              substituteInPlace config.h.cmake \
                 --replace "#define _SQ64" ""

               cat > version.cmake <<EOF
SET(SUPERTUX_VERSION_MAJOR ${builtins.elemAt ver 0})
SET(SUPERTUX_VERSION_MINOR ${builtins.elemAt ver 1})
SET(SUPERTUX_VERSION_PATCH ${builtins.elemAt ver 2})
SET(SUPERTUX_VERSION_TWEAK ${builtins.elemAt ver 3})
SET(SUPERTUX_VERSION_STRING "v${version}")
SET(SUPERTUX_VERSION_BUILD "${builtins.elemAt ver 4}")
EOF
            '';

            cmakeFlags = [
              "-DINSTALL_SUBDIR_BIN=bin"
              "-DUSE_SYSTEM_SDL2_TTF=ON"
            ];

            postFixup =
              (nixpkgs.lib.optionalString pkgs.targetPlatform.isLinux ''
                wrapProgram $out/bin/supertux2 \
                  --prefix LIBGL_DRIVERS_PATH ":" "${pkgs.mesa.drivers}/lib/dri" \
                  --prefix LD_LIBRARY_PATH ":" "${pkgs.mesa.drivers}/lib"
              '')
            + (nixpkgs.lib.optionalString pkgs.stdenv.targetPlatform.isWindows ''
                 mkdir -p $out/bin/
                 find ${pkgs.windows.mcfgthreads} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                 find ${pkgs.stdenv.cc.cc} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                 ln -sfv ${SDL2-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${SDL2_image-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${SDL2_ttf-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${pkgs.fmt_8}/bin/*.dll $out/bin/

                 ln -sfv ${wstsound.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${squirrel.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${physfs-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${curl-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${tinygettext.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 ln -sfv ${glew-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
              '');

            nativeBuildInputs = [
              pkgs.buildPackages.cmake
              pkgs.buildPackages.pkgconfig
            ]
            ++ (nixpkgs.lib.optional pkgs.targetPlatform.isLinux
                 pkgs.buildPackages.makeWrapper);

            buildInputs = [
              (if pkgs.targetPlatform.isWindows
               then SDL2_ttf-win32.packages.${pkgs.system}.default
               else SDL2_ttf.packages.${pkgs.system}.default)

              sexpcpp.packages.${pkgs.system}.default
              squirrel.packages.${pkgs.system}.default
              tinycmmc.packages.${pkgs.system}.default
              tinygettext.packages.${pkgs.system}.default
              wstsound.packages.${pkgs.system}.default

              (if pkgs.targetPlatform.isWindows
               then physfs-win32.packages.${pkgs.system}.default
               else pkgs.physfs)

              pkgs.libpng

              (if pkgs.targetPlatform.isWindows
               then curl-win32.packages.${pkgs.system}.default
               else pkgs.curl)

              pkgs.fmt_8

              (if pkgs.targetPlatform.isWindows
               then glew-win32.packages.${pkgs.system}.default
               else pkgs.glew)

              (pkgs.glm.overrideAttrs (oldAttrs: { meta = {}; }))

              (if pkgs.targetPlatform.isWindows
               then SDL2-win32.packages.${pkgs.system}.default
               else pkgs.SDL2)

              (if pkgs.targetPlatform.isWindows
               then SDL2_image-win32.packages.${pkgs.system}.default
               else pkgs.SDL2_image)

              # pkgs.gtest
            ]
            ++ (nixpkgs.lib.optional pkgs.targetPlatform.isWindows
              freetype-win32.packages.${pkgs.system}.default);
          };

          supertux2-win32 = pkgs.runCommand "supertux2-win32" {} ''
            mkdir -p $out
            mkdir -p $out/data/

            cp --verbose --recursive ${supertux2}/bin/supertux2.exe $out/
            cp --verbose --recursive --dereference --no-preserve=all ${supertux2}/bin/*.dll $out/
            cp --verbose --recursive ${supertux2}/data/. $out/data/
          '';

          supertux2-win32-zip = pkgs.runCommand "supertux2-win32-zip" {} ''
            mkdir -p $out
            WORKDIR=$(mktemp -d)

            cp --no-preserve mode,ownership --verbose --recursive \
              ${supertux2-win32}/. "$WORKDIR"

            cd "$WORKDIR"
            ${nixpkgs.legacyPackages.x86_64-linux.zip}/bin/zip \
              -r \
              $out/SuperTux-${supertux2.version}-${pkgs.system}.zip \
              .
          '';
        };
      }
    );
}
