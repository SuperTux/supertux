#ifndef SUPERTUX_RESOURCES_H
#define SUPERTUX_RESOURCES_H

extern texture_type img_waves[3]; 
extern texture_type img_water;
extern texture_type img_pole;
extern texture_type img_poletop;
extern texture_type img_flag[2];
extern texture_type img_cloud[2][4];

extern texture_type img_box_full;
extern texture_type img_box_empty;
extern texture_type img_super_bkgd;
extern texture_type img_red_glow;

void loadshared();
void unloadshared();

#endif

/* EOF */

