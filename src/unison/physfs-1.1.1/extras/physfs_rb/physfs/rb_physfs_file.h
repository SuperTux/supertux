/*
 * PhysicsFS File abstraction - ruby interface
 * 
 * Author::  Ed Sinjiashvili (slimb@vlinkmail.com)
 * License:: LGPL
 */

#ifndef __RB__PHYSFS__FILE__H__
#define __RB__PHYSFS__FILE__H__

extern VALUE classPhysfsFile;

VALUE physfs_file_new    (PHYSFS_file *file);
VALUE physfs_file_close  (VALUE self);
VALUE physfs_file_read   (VALUE self, VALUE objSize, VALUE objCount);
VALUE physfs_file_write  (VALUE self, VALUE buf, VALUE objSize, VALUE objCount);
VALUE physfs_file_eof    (VALUE self);
VALUE physfs_file_tell   (VALUE self);
VALUE physfs_file_seek   (VALUE self, VALUE pos);
VALUE physfs_file_length (VALUE self);

void init_physfs_file (void);

#endif
