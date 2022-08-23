{ self
, stdenv
, lib

, cmake
, pkgconfig
, makeWrapper

, SDL2
, SDL2_image
, SDL2_ttf
, curl
, fmt_8
, freetype
, glew
, glm
, libpng
, mcfgthreads
, mesa
, physfs
, sexpcpp
, squirrel
, tinycmmc
, tinygettext
, wstsound
}:

stdenv.mkDerivation rec {
  pname = "supertux2";
  # FIXME: Should use `git describe` to get the version
  # number or leave it to cmake, but the .git/ directory
  # isn't included in the Nix store.
  version = "0.6.3-${lib.substring 0 8 self.lastModifiedDate}-${self.shortRev or "dirty"}";

  src = lib.cleanSource ./.;

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
    (lib.optionalString stdenv.targetPlatform.isLinux ''
       wrapProgram $out/bin/supertux2 \
         --prefix LIBGL_DRIVERS_PATH ":" "${mesa.drivers}/lib/dri" \
         --prefix LD_LIBRARY_PATH ":" "${mesa.drivers}/lib"
    '')
  + (lib.optionalString stdenv.targetPlatform.isWindows ''
       mkdir -p $out/bin/
       find ${mcfgthreads} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
       find ${stdenv.cc.cc} -iname "*.dll" -exec ln -sfv {} $out/bin/ \;
       ln -sfv ${SDL2_image}/bin/*.dll $out/bin/
       ln -sfv ${SDL2_ttf}/bin/*.dll $out/bin/
       ln -sfv ${SDL2}/bin/*.dll $out/bin/
       ln -sfv ${curl}/bin/*.dll $out/bin/
       ln -sfv ${fmt_8}/bin/*.dll $out/bin/
       ln -sfv ${glew}/bin/*.dll $out/bin/
       ln -sfv ${physfs}/bin/*.dll $out/bin/
       ln -sfv ${squirrel}/bin/*.dll $out/bin/
       ln -sfv ${tinygettext}/bin/*.dll $out/bin/
       ln -sfv ${wstsound}/bin/*.dll $out/bin/
    '');

  nativeBuildInputs = [
    cmake
    pkgconfig
  ]
  ++ (lib.optional stdenv.targetPlatform.isLinux makeWrapper);

  buildInputs = [
    SDL2
    SDL2_image
    SDL2_ttf
    curl
    fmt_8
    glew
    glm
    libpng
    physfs
    sexpcpp
    squirrel
    tinycmmc
    tinygettext
    wstsound
  ];
}
