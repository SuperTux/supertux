/*
  sound.h
  
  Super Tux - Audio Functions
  
  by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/supertux/
  
  April 22, 2000 - July 15, 2002
*/


#ifndef NOSOUND

Mix_Chunk * load_sound(char * file);
void playsound(Mix_Chunk * snd);
Mix_Music * load_song(char * file);

#endif
