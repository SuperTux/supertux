/*
 * SDL_RWops - ruby interface
 *
 * Author::	Ed Sinjiashvili (slimb@vlinkmail.com)
 * License::	LGPL
 */

#include "SDL_rwops.h"
#include "ruby.h"

#include "rb_physfs.h"
#include "rb_sdl_rwops.h"

VALUE classRWops;

/*
 * RWops constructor
 */
VALUE sdl_rwops_new (SDL_RWops *ops)
{
    VALUE result; 

    if (ops == 0)
	return Qnil;

    result = Data_Wrap_Struct (classRWops, 0, SDL_FreeRW, ops);
    return result;
}

/*
 * PhysicsFS::RWops::from_file name, mode
 *
 * create RWops object from file
 */
VALUE sdl_rwops_from_file (VALUE self, VALUE name, VALUE mode)
{
    SDL_RWops *ops = SDL_RWFromFile(STR2CSTR(name), STR2CSTR(mode));
    return sdl_rwops_new (ops);
}

/*
 * PhysicsFS::RWops::from_memory string
 *
 * create RWops object from memory
 */
VALUE sdl_rwops_from_mem (VALUE self, VALUE str)
{
    int	  len	   = RSTRING(str)->len;
    void *mem	   = STR2CSTR(str);
    SDL_RWops *ops = SDL_RWFromMem(mem, len);

    return sdl_rwops_new (ops);
}

/*
 * PhysicsFS::RWops#seek offset, whence
 *
 * position RWops object 
 */
VALUE sdl_rwops_seek (VALUE self, VALUE offset, VALUE whence)
{
    int result;
    SDL_RWops *ops;
    
    Data_Get_Struct (self, SDL_RWops, ops);
    if (ops == 0)
	return Qnil;

    result = SDL_RWseek(ops, FIX2INT(offset), FIX2INT(whence));
    return INT2FIX(result);
}

/*
 * PhysicsFS::RWops#close
 *
 * close RWops. No use of the object is possible after that.
 */
VALUE sdl_rwops_close (VALUE self)
{
    int result;
    SDL_RWops *ops;
    
    Data_Get_Struct (self, SDL_RWops, ops);
    if (ops == 0)
	return Qnil;
    
    result = SDL_RWclose (ops);
    DATA_PTR(self) = 0;

    return INT2FIX(result);
}

/*
 * PhysicsFS::RWops#read
 *
 * read from RWops object objCount objSize'd entities.
 * return string containing raw data or nil
 */
VALUE sdl_rwops_read (VALUE self, VALUE objSize, VALUE objCount)
{
    int objRead;
    void *buffer;
    VALUE result;
    SDL_RWops *ops;

    Data_Get_Struct (self, SDL_RWops, ops);
    if (ops == 0)
	return Qnil;

    buffer = malloc (FIX2UINT(objSize) * FIX2UINT(objCount));
    if (buffer == 0)
	return Qnil;

    objRead = SDL_RWread (ops, buffer, FIX2UINT(objSize), FIX2UINT(objCount));
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
 * PhysicsFS::RWops#write buffer, size, n
 *
 * write raw string containing n objects size length each.
 * return number of objects written or nil
 */
VALUE sdl_rwops_write (VALUE self, VALUE buffer, VALUE size, VALUE n)
{
    int result;
    SDL_RWops *ops;

    Data_Get_Struct (self, SDL_RWops, ops);
    if (ops == 0)
	return Qnil;

    result = SDL_RWwrite (ops, STR2CSTR(buffer), FIX2INT(size), FIX2INT(n));
	
    if (result == -1)
	return Qnil;

    return INT2FIX(result);
}

void init_sdl_rwops (void)
{
    classRWops = rb_define_class_under (modulePhysfs, "RWops", rb_cObject);
    
    rb_define_method (classRWops, "seek",  sdl_rwops_seek,  2);
    rb_define_method (classRWops, "read",  sdl_rwops_read,  2);
    rb_define_method (classRWops, "write", sdl_rwops_write, 3);
    rb_define_method (classRWops, "close", sdl_rwops_close, 0);
    
    rb_define_singleton_method (classRWops, "from_file", 
				sdl_rwops_from_file, 2);
    rb_define_singleton_method (classRWops, "from_memory", 
				sdl_rwops_from_mem, 1);
}
