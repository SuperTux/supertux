#ifndef SUPERTUX_RESOURCES_H
#define SUPERTUX_RESOURCES_H

extern Surface* img_waves[3]; 
extern Surface* img_water;
extern Surface* img_pole;
extern Surface* img_poletop;
extern Surface* img_flag[2];
extern Surface* img_cloud[2][4];

extern Surface* img_box_full;
extern Surface* img_box_empty;
extern Surface* img_super_bkgd;
extern Surface* img_red_glow;

void loadshared();
void unloadshared();

#endif

/* EOF */

