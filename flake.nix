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

          supertux2 = pkgs.callPackage ./supertux2.nix {
            inherit self;

            SDL2_ttf = if pkgs.targetPlatform.isWindows
                       then SDL2_ttf-win32.packages.${pkgs.system}.default
                       else SDL2_ttf.packages.${pkgs.system}.default;

            sexpcpp = sexpcpp.packages.${pkgs.system}.default;
            squirrel = squirrel.packages.${pkgs.system}.default;
            tinycmmc = tinycmmc.packages.${pkgs.system}.default;
            tinygettext = tinygettext.packages.${pkgs.system}.default;
            wstsound = wstsound.packages.${pkgs.system}.default;

            physfs = if pkgs.targetPlatform.isWindows
                     then physfs-win32.packages.${pkgs.system}.default
                     else pkgs.physfs;

            curl = if pkgs.targetPlatform.isWindows
                   then curl-win32.packages.${pkgs.system}.default
                   else pkgs.curl;

            glew = if pkgs.targetPlatform.isWindows
                   then glew-win32.packages.${pkgs.system}.default
                   else pkgs.glew;

            glm = (pkgs.glm.overrideAttrs (oldAttrs: { meta = {}; }));

            SDL2 = if pkgs.targetPlatform.isWindows
                   then SDL2-win32.packages.${pkgs.system}.default
                   else pkgs.SDL2;

            SDL2_image = if pkgs.targetPlatform.isWindows
                         then SDL2_image-win32.packages.${pkgs.system}.default
                         else pkgs.SDL2_image;

            mcfgthreads = pkgs.windows.mcfgthreads;
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
