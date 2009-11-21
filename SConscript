#  SuperTux
#  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http:#www.gnu.org/licenses/>.

class Project:
    def __init__(self):
        self.build_squirrel()
        self.build_tinygettext()
        self.build_binreloc()
        self.build_findlocale()
        self.build_miniswig()
        self.build_supertux()
        self.build_tests()
        
    def build_tinygettext(self):
        env = Environment(CPPPATH=[".", "src"])
        env.ParseConfig("sdl-config --libs --cflags")
        self.libtinygettext = env.StaticLibrary("tinygettext", Glob("external/tinygettext/tinygettext/*.cpp"))

    def build_binreloc(self):
        env = Environment(CPPPATH=["external/binreloc/", "."])
        self.libbinreloc = env.StaticLibrary("binreloc", "external/binreloc/binreloc.c")

    def build_findlocale(self):
        env = Environment(CPPPATH=["external/findlocale/", "."])
        self.libfindlocale = env.StaticLibrary("findlocale", "external/findlocale/findlocale.c")

    def build_squirrel(self):
        env = Environment(CPPPATH=["external/squirrel/include/"])
        self.libsquirrel = env.StaticLibrary("squirrel",
                                             Glob("external/squirrel/squirrel/*.cpp") +
                                             Glob("external/squirrel/sqstdlib/*.cpp") +
                                             Glob("external/squirrel/sqstdlib/*.c"))

    def build_miniswig(self):
        env = Environment(CPPPATH=[".", "tools/miniswig/"])
        miniswig_bin = env.Program('miniswig',
                                   ['tools/miniswig/parser.yy',
                                    'tools/miniswig/lexer.ll',
                                    'tools/miniswig/create_docu.cpp',
                                    'tools/miniswig/xmlwriter.cpp',
                                    'tools/miniswig/create_wrapper.cpp',
                                    'tools/miniswig/main.cpp',
                                    'tools/miniswig/tree.cpp'])

        # env = self.env.Clone()
        # env.Append(MINISWIG=miniswig_bin)

        # env.Depends(env.Command('src/scripting/miniswig.tmp', 'src/scripting/wrapper.interface.hpp',
        #                         ["$CXX -E -Isrc/ -x c -CC $SOURCE -o $TARGET -DSCRIPTING_API"]),
        #             ['src/scripting/interface.hpp',
        #              'src/scripting/game_objects.hpp'])

        # env.Depends(env.Command(['src/scripting/wrapper.cpp', 'src/scripting/wrapper.hpp'], 'src/scripting/miniswig.tmp',
        #                         ["$MINISWIG --input $SOURCE --output-cpp ${TARGETS[0]} --output-hpp ${TARGETS[1]} "+
        #                          "--module windstille --select-namespace Scripting"]),
        #             miniswig_bin)
        
        # g++ -x "c++" -E -CC -DSCRIPTING_API src/scripting/wrapper.interface.hpp -o miniswig.tmp -Isrc
        # tools/miniswig/miniswig --input miniswig.tmp \
        #                         --output-cpp src/scripting/wrapper.cpp \
        #                         --output-hpp src/scripting/wrapper.hpp \
        #                         --module supertux \
        #                         --select-namespace scripting
        
    def build_supertux(self):
        self.env = Environment(CPPPATH=["external/squirrel/include/",
                                        "external/findlocale/",
                                        "external/binreloc/",
                                        "external/obstack/",
                                        "external/tinygettext/",
                                        "src/",
                                        "/usr/include/AL/", # yuck
                                        "."],
                               CXXFLAGS=["-O2", "-g3",
                                         "-ansi",
                                         "-pedantic",
                                         "-Wall",
                                         "-Wextra",
                                         "-Wnon-virtual-dtor",
                                         "-Weffc++",
                                         # "-Wconversion",
                                         "-Werror",
                                         # "-Wshadow",
                                         "-Wcast-qual",
                                         "-Winit-self", # only works with >= -O1
                                         "-Wno-unused-parameter"])

        # Add libraries
        self.env.ParseConfig("sdl-config --libs --cflags")
        self.env.ParseConfig("pkg-config --libs --cflags openal")
        self.env.ParseConfig("pkg-config --libs --cflags vorbis vorbisfile ogg")
        self.env.Append(LIBS=[self.libsquirrel, self.libbinreloc, self.libtinygettext, self.libfindlocale])
        self.env.Append(LIBS=["SDL_image", "curl", "GL", "GLEW", "physfs"])

        # Create config.h
        self.iconv_const = 0
        config_h = open('config.h', 'w')
        config_h.write('#define PACKAGE_NAME "Supertux"\n')
        config_h.write('#define PACKAGE_VERSION "Milestone 2"\n')
        config_h.write('#define ENABLE_BINRELOC 1\n')
        config_h.write('#define APPDATADIR "data"\n')
        config_h.write('#define HAVE_LIBCURL 1\n')
        config_h.write('#define HAVE_OPENGL 1\n')
        config_h.write('#define ICONV_CONST %s\n' % self.iconv_const)
        config_h.close()

        version_h = open('version.h', 'w')
        version_h.close()

        # base source
        supertux_sources = Glob("src/*.cpp") + Glob("src/*/*.cpp") + Glob("src/supertux/menu/*.cpp")

        # optional video drivers
        supertux_sources += Glob("src/video/gl/*.cpp")
        supertux_sources += Glob("src/video/sdl/*.cpp")

        self.libsupertux = self.env.StaticLibrary("supertux", supertux_sources)
        self.env.Program("supertux", ["src/main.cpp", self.libsupertux])

    def build_tests(self):
        for filename in Glob("test/*_test.cpp", strings=True):
            self.env.Program(filename[:-4], [filename, self.libsupertux])

project = Project()

# EOF #
