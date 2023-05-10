if(ANDROID)
  #FIXME: ogg vorbis package not working
  find_library(OggVorbis vorbis)
else()
  find_package(OggVorbis REQUIRED)
endif()

add_library(LibOggVorbis INTERFACE IMPORTED)
set_target_properties(LibOggVorbis PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${VORBIS_INCLUDE_DIR}"
  INTERFACE_LINK_LIBRARIES "${OGGVORBIS_LIBRARIES}"
  )

mark_as_advanced(
  OGG_LIBRARY
  VORBISENC_LIBRARY
  VORBISFILE_LIBRARY
  VORBIS_INCLUDE_DIR
  VORBIS_LIBRARY
  )

# EOF #
