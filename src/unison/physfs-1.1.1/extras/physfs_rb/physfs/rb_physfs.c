/*
 * PhysicsFS - ruby interface
 * 
 * Author::  Ed Sinjiashvili (slimb@vlinkmail.com)
 * License:: LGPL
 */

#include "physfs.h"
#include "ruby.h"

#include "rb_physfs.h" 
#include "rb_physfs_file.h"

VALUE modulePhysfs;

/*
 * PhysicsFS::init str
 *
 * initialize PhysicsFS
 */
VALUE physfs_init (VALUE self, VALUE str)
{
    int result = PHYSFS_init (STR2CSTR(str));

    if (result)
	return Qtrue;

    return Qfalse;
}

/*
 * PhysicsFS::deinit
 */
VALUE physfs_deinit (VALUE self)
{
    if (PHYSFS_deinit ())
	return Qtrue;

    return Qfalse;
}

/*
 * PhysicsFS::version
 * 
 * return PhysicsFS::Version object
 */
VALUE physfs_version (VALUE self)
{
    char evalStr[200];
    PHYSFS_Version ver;

    PHYSFS_getLinkedVersion (&ver);

    sprintf (evalStr, "PhysicsFS::Version.new %d, %d, %d", 
	     ver.major, ver.minor, ver.patch);
    return rb_eval_string (evalStr);
}

/*
 * PhysicsFS::supported_archives
 *
 * return Array of PhysicsFS::ArchiveInfo objects
 */
VALUE physfs_supported_archives (VALUE self)
{
    const PHYSFS_ArchiveInfo **info = PHYSFS_supportedArchiveTypes();
    VALUE klass = rb_const_get (modulePhysfs, rb_intern ("ArchiveInfo"));
    VALUE ary = rb_ary_new ();
    VALUE params[4];

    while ( *info != 0 ) 
    {
        params[0] = rb_str_new2 ((*info)->extension);
        params[1] = rb_str_new2 ((*info)->description);
        params[2] = rb_str_new2 ((*info)->author);
        params[3] = rb_str_new2 ((*info)->url);

        rb_ary_push (ary, rb_class_new_instance (4, params, klass));
        info++;
    }

    return ary;
}

/*
 * PhysicsFS::last_error
 *
 * return string representation of last PhysicsFS error
 */
VALUE physfs_last_error (VALUE self)
{
    const char *last_error = PHYSFS_getLastError ();

    if (last_error == 0)
	last_error = "";

    return rb_str_new2 (last_error);
}

/*
 * PhysicsFS::dir_separator
 *
 * return platform directory separator
 */
VALUE physfs_dir_separator (VALUE self)
{
    return rb_str_new2 (PHYSFS_getDirSeparator ());
}

/*
 * PhysicsFS::permit_symlinks boolValue
 *
 * turn symlinks support on/off
 */
VALUE physfs_permit_symlinks (VALUE self, VALUE allow)
{
    int p = 1;
    
    if (allow == Qfalse || allow == Qnil)
        p = 0;

    PHYSFS_permitSymbolicLinks (p);
    return Qtrue;
}

/*
 * PhysicsFS::cdrom_dirs
 *
 * return Array of strings containing available CDs 
 */
VALUE physfs_cdrom_dirs (VALUE self)
{
    char **cds = PHYSFS_getCdRomDirs();
    char **i;
    VALUE ary = rb_ary_new ();

    for (i = cds; *i != 0; i++)
        rb_ary_push (ary, rb_str_new2 (*i));

    PHYSFS_freeList (cds);
    return ary;
}

/*
 * PhysicsFS::base_dir
 *
 * return base directory
 */
VALUE physfs_base_dir (VALUE self)
{
    const char *base_dir = PHYSFS_getBaseDir ();
    if (base_dir == 0)
        base_dir = "";

    return rb_str_new2 (base_dir);
}

/*
 * PhysicsFS::user_dir
 *
 * return user directory
 */
VALUE physfs_user_dir (VALUE self)
{
    const char *user_dir = PHYSFS_getBaseDir ();
    if (user_dir == 0)
        user_dir = "";

    return rb_str_new2 (user_dir);
}
   
/*
 * PhysicsFS::write_dir 
 *
 * return write directory
 */
VALUE physfs_write_dir (VALUE self)
{
    const char *write_dir = PHYSFS_getWriteDir ();
    if (write_dir == 0)
        return Qnil;

    return rb_str_new2 (write_dir);
}

/*
 * PhysicsFS::write_dir= str
 *
 * set write directory to *str*
 */
VALUE physfs_set_write_dir (VALUE self, VALUE str)
{
    int result = PHYSFS_setWriteDir (STR2CSTR(str));

    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::add_to_search_path str, append
 *
 * if append > 0 - append str to search path, otherwise prepend it
 */
VALUE physfs_add_search_path (VALUE self, VALUE str, VALUE append)
{
    int result = PHYSFS_addToSearchPath (STR2CSTR(str), FIX2INT(append));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::remove_from_search_path str
 *
 * removes str from search path
 */
VALUE physfs_remove_search_path (VALUE self, VALUE str)
{
    int result = PHYSFS_removeFromSearchPath (STR2CSTR(str));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::search_path
 *
 * return current search_path - as array of strings
 */
VALUE physfs_search_path (VALUE self)
{
    char **path = PHYSFS_getSearchPath ();
    char **i;
    VALUE ary = rb_ary_new ();

    for (i = path ; *i != 0; i++)
        rb_ary_push (ary, rb_str_new2 (*i));

    PHYSFS_freeList (path);
    return ary;
}

// 
VALUE physfs_setSaneConfig(VALUE self, VALUE org, VALUE app, VALUE ext,
                           VALUE includeCdroms, VALUE archivesFirst)
{
    int res = PHYSFS_setSaneConfig (STR2CSTR(org), STR2CSTR(app), STR2CSTR(ext),
                                   RTEST(includeCdroms), RTEST(archivesFirst));
    if (res)
        return Qtrue;

    return Qfalse;
}

/*
 * PhysicsFS::mkdir newdir
 *
 * create new directory 
 */ 
VALUE physfs_mkdir (VALUE self, VALUE newdir)
{
    int result = PHYSFS_mkdir (STR2CSTR(newdir));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::delete name
 *
 * delete file with name
 */
VALUE physfs_delete (VALUE self, VALUE name)
{
    int result = PHYSFS_delete (STR2CSTR(name));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::real_dir name
 *
 * return real directory (in search path) of a name
 */
VALUE physfs_real_dir (VALUE self, VALUE name)
{
    const char *path = PHYSFS_getRealDir (STR2CSTR(name));
    if (path == 0)
        return Qnil;

    return rb_str_new2 (path);
}

/*
 * PhysicsFS::enumerate dir
 *
 * list a dir from a search path
 */
VALUE physfs_enumerate (VALUE self, VALUE dir)
{
    char **files = PHYSFS_enumerateFiles (STR2CSTR(dir));
    char **i;
    VALUE ary = rb_ary_new ();

    for (i = files; *i != 0; i++)
        rb_ary_push (ary, rb_str_new2 (*i));

    PHYSFS_freeList (files);
    return ary;
}

/*
 * PhysicsFS::exists? name
 *
 * does a file with name exist?
 */
VALUE physfs_exists (VALUE self, VALUE name)
{
    int result = PHYSFS_exists (STR2CSTR(name));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::is_directory? name
 *
 * return true if name is directory
 */
VALUE physfs_is_directory (VALUE self, VALUE name)
{
    int result = PHYSFS_isDirectory (STR2CSTR(name));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::is_symlink? name
 *
 * return true if name is symlink
 */
VALUE physfs_is_symlink (VALUE self, VALUE name)
{
    int result = PHYSFS_isSymbolicLink (STR2CSTR(name));
    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::last_mod_time name
 *
 * return last modification time of a file
 */
VALUE physfs_last_mod_time (VALUE self, VALUE name)
{
    int result = PHYSFS_getLastModTime (STR2CSTR(name));
    
    return INT2FIX(result);
}

/*
 * PhysicsFS::open_read name
 *
 * return +PhysicsFS::File+ ready for reading
 */
VALUE physfs_open_read (VALUE self, VALUE name)
{
    PHYSFS_File *file = PHYSFS_openRead (STR2CSTR(name));
    return physfs_file_new (file);
}

/*
 * PhysicsFS::open_write name
 *
 * return PhysicsFS::File ready for writing
 */
VALUE physfs_open_write (VALUE self, VALUE name)
{
    PHYSFS_File *file = PHYSFS_openWrite (STR2CSTR(name));
    return physfs_file_new (file);
}

/*
 * PhysicsFS::open_append name
 *
 * return PhysicsFS::File ready for appending
 */
VALUE physfs_open_append (VALUE self, VALUE name)
{
    PHYSFS_File *file = PHYSFS_openAppend (STR2CSTR(name));
    return physfs_file_new (file);
}

void Init_physfs_so (void)
{
    modulePhysfs = rb_define_module ("PhysicsFS");

    rb_define_singleton_method (modulePhysfs, "init_internal", physfs_init, 1);
    rb_define_singleton_method (modulePhysfs, "deinit", physfs_deinit, 0);
    rb_define_singleton_method (modulePhysfs, "version", physfs_version, 0);
    rb_define_singleton_method (modulePhysfs, "supported_archives",
				physfs_supported_archives, 0);
    rb_define_singleton_method (modulePhysfs, "last_error", 
				physfs_last_error, 0);
    rb_define_singleton_method (modulePhysfs, "dir_separator",
				physfs_dir_separator, 0);
    rb_define_singleton_method (modulePhysfs, "permit_symlinks",
                                physfs_permit_symlinks, 1);
    rb_define_singleton_method (modulePhysfs, "cdrom_dirs", 
                                physfs_cdrom_dirs, 0);
    rb_define_singleton_method (modulePhysfs, "base_dir", physfs_base_dir, 0);
    rb_define_singleton_method (modulePhysfs, "user_dir", physfs_user_dir, 0);

    rb_define_singleton_method (modulePhysfs, "write_dir", physfs_write_dir, 0);
    rb_define_singleton_method (modulePhysfs, "write_dir=", 
                                physfs_set_write_dir, 1);

    rb_define_singleton_method (modulePhysfs, "add_to_search_path",
                                physfs_add_search_path, 2);
    rb_define_singleton_method (modulePhysfs, "remove_from_search_path",
                                physfs_remove_search_path, 1);
    rb_define_singleton_method (modulePhysfs, "search_path",
                                physfs_search_path, 0);

    rb_define_singleton_method (modulePhysfs, "set_sane_config",
                                physfs_setSaneConfig, 5);

    rb_define_singleton_method (modulePhysfs, "mkdir", physfs_mkdir, 1);
    rb_define_singleton_method (modulePhysfs, "delete", physfs_delete, 1);
    rb_define_singleton_method (modulePhysfs, "real_dir",
                                physfs_real_dir, 1);
    rb_define_singleton_method (modulePhysfs, "enumerate", physfs_enumerate, 1);
    rb_define_singleton_method (modulePhysfs, "exists?", physfs_exists, 1);
    rb_define_singleton_method (modulePhysfs, "is_directory?", 
                                physfs_is_directory, 1);
    rb_define_singleton_method (modulePhysfs, "is_symlink?", 
                                physfs_is_symlink, 1);
    rb_define_singleton_method (modulePhysfs, "last_mod_time",
                                physfs_last_mod_time, 1);

    rb_define_singleton_method (modulePhysfs, "open_read", 
                                physfs_open_read, 1);
    rb_define_singleton_method (modulePhysfs, "open_write", 
                                physfs_open_write, 1);
    rb_define_singleton_method (modulePhysfs, "open_append", 
                                physfs_open_append, 1);

    init_physfs_file ();
    init_sdl_rwops ();
}

/*
// Local Variables:
// mode: C
// c-indentation-style: "stroustrup"
// indent-tabs-mode: nil
// End:
*/
