/*
 * PhysicsFS File abstraction - ruby interface
 * 
 * Author::  Ed Sinjiashvili (slimb@vlinkmail.com)
 * License:: LGPL
 */

#include "physfs.h"
#include "ruby.h"

#include "rb_physfs.h"
#include "rb_physfs_file.h"
#include "physfsrwops.h"

VALUE classPhysfsFile;

/*
 * construct new PhysicsFS::File object
 */
VALUE physfs_file_new (PHYSFS_File *file)
{
    if (file == 0)
        return Qnil;

    return Data_Wrap_Struct (classPhysfsFile, 0, 0, file);
}

/*
 * PhysicsFS::File#close 
 *
 * Close the file. It's illegal to use the object after its closure.
 */
VALUE physfs_file_close (VALUE self)
{
    int result;
    PHYSFS_File *file;
    Data_Get_Struct (self, PHYSFS_File, file);

    if (file == 0)
	return Qfalse;

    result = PHYSFS_close (file);
    DATA_PTR(self) = 0;

    if (result)
        return Qtrue;
    return Qfalse;
}

/*
 * PhysicsFS::File#read obj_size, num_objects
 *
 * Read *objCount* objects which are *objSize* each.
 * return String instance containing raw data or nil if failure.
 * #length of string will reflect real number of objects read.
 */
VALUE physfs_file_read (VALUE self, VALUE objSize, VALUE objCount)
{
    int objRead;
    void *buffer;
    VALUE result;
    PHYSFS_File *file;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil; //wasted file - no read possible

    buffer  = malloc (FIX2UINT(objSize) * FIX2UINT(objCount));
    if (buffer == 0)
	return Qnil;

    objRead = PHYSFS_read (file, buffer, FIX2UINT(objSize), FIX2UINT(objCount));
    if (objRead == -1)
    {
        free (buffer);
        return Qnil;
    }

    result = rb_str_new (buffer, objRead * FIX2UINT(objSize));
    free (buffer);
    return result;
}

/*
 * PhysicsFS::File#write buffer, obj_size, num_objects
 *
 * return nil on failure or number of objects written.
 */
VALUE physfs_file_write (VALUE self, VALUE buf, VALUE objSize, VALUE objCount)
{
    int result;
    PHYSFS_File *file;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil;

    result = PHYSFS_write (file, STR2CSTR(buf), 
                           FIX2UINT(objSize), FIX2UINT(objCount));
    if (result == -1)
        return Qnil;

    return INT2FIX(result);
}

/*
 * PhysicsFS::File#eof? 
 */
VALUE physfs_file_eof (VALUE self)
{
    int result;
    PHYSFS_File *file;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil;

    result = PHYSFS_eof (file);

    if (result)
        return Qtrue;

    return Qfalse;
}

/*
 * PhysicsFS::File#tell
 *
 * tells current position in file
 */
VALUE physfs_file_tell (VALUE self)
{
    int result;
    PHYSFS_File *file;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil;

    result = PHYSFS_tell (file);

    if (result == -1)
        return Qnil;

    return INT2FIX(result);
}    

/*
 * PhysicsFS::File#seek pos
 *
 * seek to pos in file
 */
VALUE physfs_file_seek (VALUE self, VALUE pos)
{
    int result;
    PHYSFS_File *file;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil;

    result = PHYSFS_seek (file, FIX2LONG(pos));

    if (result)
        return Qtrue;

    return Qfalse;    
}

/*
 * PhysicsFS::File#length 
 */
VALUE physfs_file_length (VALUE self)
{
    int result;
    PHYSFS_File *file;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil;

    result = PHYSFS_fileLength (file);

    if (result == -1)
        return Qnil;

    return INT2FIX(result);
}

/*
 * PhysicsFS::File#to_rwops
 *
 * File object is converted to RWops object. 
 * File object becomes unusable after that - every operation
 * should be done through new-born RWops object. 
 */
VALUE physfs_file_to_rwops (VALUE self)
{
    PHYSFS_File *file;
    SDL_RWops   *rwops;

    Data_Get_Struct (self, PHYSFS_File, file);
    if (file == 0)
	return Qnil;

    rwops = PHYSFSRWOPS_makeRWops (file);
    if (rwops == 0)
	return Qnil;

    DATA_PTR(self) = 0; // oh, gosh, we've sacrificed ourselves!
    return sdl_rwops_new (rwops);
}

void init_physfs_file (void)
{
    classPhysfsFile = rb_define_class_under (modulePhysfs, "File", rb_cObject);

    rb_define_method (classPhysfsFile, "close",    physfs_file_close,    0);
    rb_define_method (classPhysfsFile, "eof?",     physfs_file_eof,      0);
    rb_define_method (classPhysfsFile, "tell",     physfs_file_tell,     0);
    rb_define_method (classPhysfsFile, "seek",     physfs_file_seek,     1);
    rb_define_method (classPhysfsFile, "length",   physfs_file_length,   0);
    rb_define_method (classPhysfsFile, "read",     physfs_file_read,     2);
    rb_define_method (classPhysfsFile, "write",    physfs_file_write,    3);
    rb_define_method (classPhysfsFile, "to_rwops", physfs_file_to_rwops, 0);
}
