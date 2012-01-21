# -*- python -*-

env = Environment(CXXFLAGS=['-O0',
                            '-g3',
                            '-Wall',
                            '-Wcast-qual',
                            '-Wconversion',
                            '-Weffc++',
                            '-Werror',
                            '-Wextra',
                            '-Winit-self',
                            '-Wno-unused-parameter',
                            '-Wnon-virtual-dtor',
                            '-Wshadow',
                            '-ansi',
                            '-pedantic',
                            ],
                  CPPPATH=['tinygettext', '.'])

# env.ParseConfig("sdl-config --cflags --libs")
# env['CPPDEFINES'] += HAVE_SDL

libtinygettext = env.StaticLibrary('tinygettext/tinygettext', 
                                   ['tinygettext/tinygettext.cpp',
                                    'tinygettext/language.cpp',
                                    'tinygettext/plural_forms.cpp',
                                    'tinygettext/dictionary.cpp',
                                    'tinygettext/dictionary_manager.cpp',
                                    'tinygettext/unix_file_system.cpp',
                                    'tinygettext/po_parser.cpp',
                                    'tinygettext/iconv.cpp',
                                    'tinygettext/log.cpp'])

env.Program('test/tinygettext_test', ['test/tinygettext_test.cpp', libtinygettext])
env.Program('test/po_parser_test', ['test/po_parser_test.cpp', libtinygettext])

# EOF #
