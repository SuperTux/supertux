# -*- python -*-

import os

env = Environment(ENV=os.environ,
                  CXX="g++",
                  CXXFLAGS=['-O0',
                            '-g3',
                            '-std=c++0x',
                            '-Wall',
                            '-Wextra',
                            '-Wcast-qual',
                            '-Wconversion',
                            '-Weffc++',
                            '-Werror',
                            '-Wextra',
                            '-Winit-self',
                            '-Wno-unused-parameter',
                            '-Wnon-virtual-dtor',
                            '-Wshadow',
                            # '-ansi', # conflicts with -std=c++0x
                            '-pedantic',
                            ],
                  CPPPATH=['include', 'src'])

# env.ParseConfig("sdl-config --cflags --libs")
# env['CPPDEFINES'] += HAVE_SDL

libtinygettext = env.StaticLibrary('src/tinygettext',
                                   ['src/tinygettext.cpp',
                                    'src/language.cpp',
                                    'src/plural_forms.cpp',
                                    'src/dictionary.cpp',
                                    'src/dictionary_manager.cpp',
                                    'src/unix_file_system.cpp',
                                    'src/po_parser.cpp',
                                    'src/iconv.cpp',
                                    'src/log.cpp'])

env.Program('test/tinygettext_test', ['test/tinygettext_test.cpp', libtinygettext])
env.Program('test/po_parser_test', ['test/po_parser_test.cpp', libtinygettext])

# EOF #
