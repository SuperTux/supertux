#
# SConstruct build file. See http://www.scons.org for details.

# based on a script from chenlee@ustc.edu
def Glob(dirs, pattern = '*' ):
    import os, fnmatch 
    files = []
    for dir in dirs:
        for file in os.listdir( Dir(dir).srcnode().abspath ): 
            if fnmatch.fnmatch(file, pattern) : 
                files.append( os.path.join( dir, file ) ) 
    return files 

opts = Options('custom.py')
opts.Add('CXX', 'The C++ compiler', 'g++')
opts.Add('CXXFLAGS', 'Additional C++ compiler flags', '')
opts.Add('CPPPATH', 'Additional preprocessor paths', '')
opts.Add('CPPFLAGS', 'Additional preprocessor flags', '')
opts.Add('LIBPATH', 'Additional library paths', '')
opts.Add('LIBS', 'Additional libraries', '')
opts.Add('DESTDIR', \
        'destination directory for installation. It is prepended to PREFIX', '')
opts.Add('PREFIX', 'Installation prefix', '/usr/local')

env = Environment(options = opts)
conf = Configure(env)

# TODO check -config apps in the Configure context
    
if not conf.CheckLib('SDL_mixer'):
    print "Couldn't find SDL_mixer library!"
    Exit(1)
if not conf.CheckLib('SDL_image'):
    print "Couldn't find SDL_image library!"
    Exit(1)
if not conf.CheckLib('GL'):
    print "Couldn't find OpenGL library!"
    Exit(1)

env = conf.Finish()

env.ParseConfig('sdl-config --cflags --libs')
env.Append(CPPPATH = ["#", "#/src", "#/lib"])
env.Append(CPPDEFINES = \
        {'DATA_PREFIX':"'\"" + env['PREFIX'] + "/share/supertux\"'" ,
         'LOCALEDIR'  :"'\"" + env['PREFIX'] + "/locales\"'"})

env.Append(LIBS = ["supertux"])
env.Append(LIBPATH=["#"])

build_dir="build/linux"

env.Export(["env", "Glob"])
env.SConscript("lib/SConscript", build_dir=build_dir + "/lib", duplicate=0)
env.SConscript("src/SConscript", build_dir=build_dir + "/src", duplicate=0)
