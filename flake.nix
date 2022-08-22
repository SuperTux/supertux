# SuperTux - Milestone 1
# Copyright (C) 2022 Ingo Ruhnke <grumbel@gmail.com>
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

  inputs = rec {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";

    SDL-win32.url = "github:grumnix/SDL-win32";
    SDL-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL-win32.inputs.tinycmmc.follows = "tinycmmc";

    SDL_mixer-win32.url = "github:grumnix/SDL_mixer-win32";
    SDL_mixer-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL_mixer-win32.inputs.tinycmmc.follows = "tinycmmc";
    SDL_mixer-win32.inputs.SDL-win32.follows = "SDL-win32";

    SDL_image-win32.url = "github:grumnix/SDL_image-win32";
    SDL_image-win32.inputs.nixpkgs.follows = "nixpkgs";
    SDL_image-win32.inputs.tinycmmc.follows = "tinycmmc";
    SDL_image-win32.inputs.SDL-win32.follows = "SDL-win32";
  };

  outputs = { self, nixpkgs, tinycmmc, SDL-win32, SDL_mixer-win32, SDL_image-win32 }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      {
        packages = rec {
          default = supertux-milestone1;

          supertux-milestone1 = pkgs.stdenv.mkDerivation rec {
            pname = "supertux-milestone1";
            version = "0.1.4";

            src = nixpkgs.lib.cleanSource ./.;

            enableParallelBuilding = true;

            nativeBuildInputs = [
              pkgs.buildPackages.autoconf
              pkgs.buildPackages.automake
              pkgs.buildPackages.autoreconfHook
              pkgs.buildPackages.pkgconfig
            ]
            ++ (nixpkgs.lib.optional pkgs.targetPlatform.isLinux pkgs.makeWrapper);

            postFixup = ""
              + (nixpkgs.lib.optionalString pkgs.targetPlatform.isLinux ''
                   wrapProgram $out/bin/supertux-milestone1 \
                     --prefix LIBGL_DRIVERS_PATH ":" "${pkgs.mesa.drivers}/lib/dri" \
                     --prefix LD_LIBRARY_PATH ":" "${pkgs.mesa.drivers}/lib"
                 '')
              + (nixpkgs.lib.optionalString pkgs.stdenv.targetPlatform.isWindows ''
                   mkdir -p $out/bin/
                   find ${pkgs.windows.mcfgthreads} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                   find ${pkgs.stdenv.cc.cc} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
                   ln -sfv ${SDL-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                   ln -sfv ${SDL_image-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                   ln -sfv ${SDL_mixer-win32.packages.${pkgs.system}.default}/bin/*.dll $out/bin/
                 '');

            buildInputs =
              (if pkgs.stdenv.targetPlatform.isWindows
               then [
                 SDL-win32.packages.${pkgs.system}.default
                 SDL_image-win32.packages.${pkgs.system}.default
                 SDL_mixer-win32.packages.${pkgs.system}.default
               ]
               else [
                 pkgs.SDL
                 pkgs.SDL_image
                 pkgs.SDL_mixer
                 pkgs.libGL
                 pkgs.libGLU
               ]) ++
              [
                pkgs.zlib
              ];
          };

          supertux-milestone1-win32 = pkgs.runCommand "supertux-milestone1-win32" {} ''
            mkdir -p $out
            mkdir -p $out/data/

            cp -vr ${supertux-milestone1}/bin/supertux-milestone1.exe $out/
            cp -vLr ${supertux-milestone1}/bin/*.dll $out/
            cp -vr ${supertux-milestone1}/share/supertux-milestone1/. $out/data/
          '';

          supertux-milestone1-win32-zip = pkgs.runCommand "supertux-milestone1-win32-zip" {} ''
            mkdir -p $out
            WORKDIR=$(mktemp -d)

            cp --no-preserve mode,ownership --verbose --recursive \
              ${supertux-milestone1-win32}/. "$WORKDIR"

            cd "$WORKDIR"
            ${nixpkgs.legacyPackages.x86_64-linux.zip}/bin/zip \
              -r \
              $out/supertux-milestone1-${supertux-milestone1.version}-${pkgs.system}.zip \
              .
          '';
        };
      }
    );
}
