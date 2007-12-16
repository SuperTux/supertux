/*
 * SDL_RWops - ruby interface
 *
 * Author::	Ed Sinjiashvili (slimb@vlinkmail.com)
 * License::	LGPL
 */

#ifndef __RB__SDL__RWOPS__H__
#define __RB__SDL__RWOPS__H__

extern VALUE classRWops;

VALUE sdl_rwops_new (SDL_RWops *ops);
void init_sdl_rwops (void);

#endif
