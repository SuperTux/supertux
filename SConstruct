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

def CheckSDLConfig(context, minVersion):
    context.Message('Checking for sdl-config >= %s... ' % minVersion)
    from popen2 import Popen3
    p = Popen3(['sdl-config', '--version'])
    ret = p.wait()
    out = p.fromchild.readlines()
    if ret != 0:
        context.Result(False)
        return False
    if len(out) != 1:
        # unable to parse output!
        context.Result(False)
        return False
    # TODO validate output and catch exceptions
    version = map(int, out[0].strip().split('.'))
    minVersion = map(int, minVersion.split('.'))
    # TODO comparing versions that way only works for pure numeric version
    # numbers and fails for custom extensions. I don't care about this at
    # the moment as sdl-config never used such version numbers afaik.
    ret = (version >= minVersion)
    context.Result(ret)
    return ret

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
opts.Add(ListOption('VARIANT', 'Build variant', 'optimize',
            ['optimize', 'debug', 'profile']))

env = Environment(options = opts)
conf = Configure(env, custom_tests = {
    'CheckSDLConfig' : CheckSDLConfig
})

# TODO check -config apps in the Configure context
   
if not conf.CheckSDLConfig('1.2.4'):
    print "Couldn't find libSDL >= 1.2.4"
    Exit(1)
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

if str(env['VARIANT']) == "optimize":
    env.Append(CXXFLAGS = "-O2 -g")
elif str(env['VARIANT']) == "debug":
    env.Append(CXXFLAGS = "-O0 -g3")
    env.Append(CPPDEFINES = { "DEBUG":"1" })
elif str(env['VARIANT']) == "profile":
    env.Append(CXXFLAGS = "-pg -O2")

env.ParseConfig('sdl-config --cflags --libs')
env.Append(CPPPATH = ["#", "#/src", "#/lib"])
env.Append(CPPDEFINES = \
        {'DATA_PREFIX':"'\"" + env['PREFIX'] + "/share/supertux\"'" ,
         'LOCALEDIR'  :"'\"" + env['PREFIX'] + "/locales\"'"})

build_dir="build/" + env['PLATFORM'] + "/" + str(env['VARIANT'])

env.Append(LIBS = ["supertux"])
env.Append(LIBPATH=["#" + build_dir + "/lib"])

env.Export(["env", "Glob"])
env.SConscript("lib/SConscript", build_dir=build_dir + "/lib", duplicate=0)
env.SConscript("src/SConscript", build_dir=build_dir + "/src", duplicate=0)
