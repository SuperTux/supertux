env = Environment()
env.Append(CPPDEFINES = [('DATA_PREFIX', '\'"data/"\''),
                         ('NOOPENGL', '1')])
env.ParseConfig("sdl-config --libs --cflags")
env.Append(LIBS=['z', 'SDL_mixer', 'SDL_image'])
env.Program("supertux", Glob("src/*.cpp"))

# EOF #
