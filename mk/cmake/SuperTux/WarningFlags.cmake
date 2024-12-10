option(WERROR "Stop on first compiler warning" OFF)
option(WARNINGS "Enable long list of warnings for compiler to check" OFF)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE STRING "Debug flags")
  set(CMAKE_CXX_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  set(CMAKE_C_FLAGS_PROFILE "-pg" CACHE STRING "Profile flags")
  set(CMAKE_LD_FLAGS_PROFILE "-lgmon" CACHE STRING "Profile flags")

  if(WERROR)
    string(CONCAT SUPERTUX2_EXTRA_WARNING_FLAGS "-Werror;")
  endif()

  if(WARNINGS)
    # temporarily disabled:
    #   -Wsign-conversion -Wfloat-equal -Wundef -Wswitch-default
    #   -Wswitch-enum -Wsign-promo -Wcast-qual -Woverloaded-virtual -Wmissing-format-attribute
    #   -Wpadded -Wabi -Winline -Wunsafe-loop-optimizations -Wstrict-overflow=5
    # fails on MinGW:
    #   -ansi

    set(ALL_WARNING_FLAGS
      -Wall -Weverything -Wextra
      -fdiagnostics-show-option
      -pedantic
      -Wno-long-long
      -Wcast-align
      -Wdisabled-optimization
      -Winit-self -Winvalid-pch
      -Wmissing-include-dirs
      -Wmissing-noreturn
      -Wpacked -Wredundant-decls
      -Wformat=2
      -Weffc++
      -Wctor-dtor-privacy
      -Wshadow
      -Wnon-virtual-dtor
      -Wcast-qual
      -Wold-style-cast
      -Wzero-as-null-pointer-constant
      -Wconversion
      -Wlogical-op
      -Wstrict-null-sentinel
      -Wsuggest-override
      -Wint-in-bool-context

      # flags that we deliberately ignore
      -Wno-unused-parameter
      -Wno-unused-template
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-float-equal
      -Wno-padded
      -Wno-weak-vtables
      -Wno-disabled-macro-expansion
      -Wno-documentation
      -Wno-reserved-id-macro
      -Wno-sign-conversion
      -Wno-reserved-identifier
      -Wno-unknown-warning-option

      # warnings that should probably be fixed in code
      -Wno-documentation-unknown-command
      -Wno-inconsistent-missing-destructor-override
      -Wno-deprecated-dynamic-exception-spec
      -Wno-deprecated
      -Wno-switch-enum
      -Wno-covered-switch-default
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-duplicate-enum
      -Wno-unreachable-code)
    
    foreach(flag IN LISTS ALL_WARNING_FLAGS)
      check_cxx_compiler_flag(${flag} HAVE_FLAG_${flag})
      if(HAVE_FLAG_${flag})
        string(APPEND SUPERTUX2_EXTRA_WARNING_FLAGS "${flag};")
      endif()
      unset(HAVE_FLAG CACHE)
    endforeach(flag IN LISTS ALL_WARNING_FLAGS)

    # This snipped adapted from AOMediaCodec cmakefile
    # The detection of cross compilation by -Wpoison-system-directories has false positives on macOS because
    # --sysroot is implicitly added. Turn the warning off.
    if(NOT DEFINED HAVE_POISON_SYSTEM_DIRECTORIES_WARNING)
        check_cxx_compiler_flag(-Wpoison-system-directories HAVE_POISON_SYSTEM_DIRECTORIES_WARNING)
    endif()
    if(HAVE_POISON_SYSTEM_DIRECTORIES_WARNING)
      string(APPEND SUPERTUX2_EXTRA_WARNING_FLAGS "-Wno-poison-system-directories;")
    endif()
   endif()
endif()

# EOF #
