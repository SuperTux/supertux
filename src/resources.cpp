#include "globals.h"
#include "scene.h"
#include "player.h"
#include "badguy.h"
#include "resources.h"

texture_type img_waves[3]; 
texture_type img_water;
texture_type img_pole;
texture_type img_poletop;
texture_type img_flag[2];
texture_type img_cloud[2][4];

/* Load graphics/sounds shared between all levels: */
void loadshared()
{
  int i;

  /* Tuxes: */
  texture_load(&smalltux_stand_left, datadir + "/images/shared/smalltux-left-6.png", USE_ALPHA);
  texture_load(&smalltux_stand_right, datadir + "/images/shared/smalltux-right-6.png", USE_ALPHA);

  texture_load(&smalltux_jump_left, datadir + "/images/shared/smalltux-jump-left.png", USE_ALPHA);
  texture_load(&smalltux_jump_right, datadir + "/images/shared/smalltux-jump-right.png", USE_ALPHA);

  tux_right.resize(8);
  texture_load(&tux_right[0], datadir + "/images/shared/smalltux-right-1.png", USE_ALPHA);
  texture_load(&tux_right[1], datadir + "/images/shared/smalltux-right-2.png", USE_ALPHA);
  texture_load(&tux_right[2], datadir + "/images/shared/smalltux-right-3.png", USE_ALPHA);
  texture_load(&tux_right[3], datadir + "/images/shared/smalltux-right-4.png", USE_ALPHA);
  texture_load(&tux_right[4], datadir + "/images/shared/smalltux-right-5.png", USE_ALPHA);
  texture_load(&tux_right[5], datadir + "/images/shared/smalltux-right-6.png", USE_ALPHA);
  texture_load(&tux_right[6], datadir + "/images/shared/smalltux-right-7.png", USE_ALPHA);
  texture_load(&tux_right[7], datadir + "/images/shared/smalltux-right-8.png", USE_ALPHA);

  tux_left.resize(8);
  texture_load(&tux_left[0], datadir + "/images/shared/smalltux-left-1.png", USE_ALPHA);
  texture_load(&tux_left[1], datadir + "/images/shared/smalltux-left-2.png", USE_ALPHA);
  texture_load(&tux_left[2], datadir + "/images/shared/smalltux-left-3.png", USE_ALPHA);
  texture_load(&tux_left[3], datadir + "/images/shared/smalltux-left-4.png", USE_ALPHA);
  texture_load(&tux_left[4], datadir + "/images/shared/smalltux-left-5.png", USE_ALPHA);
  texture_load(&tux_left[5], datadir + "/images/shared/smalltux-left-6.png", USE_ALPHA);
  texture_load(&tux_left[6], datadir + "/images/shared/smalltux-left-7.png", USE_ALPHA);
  texture_load(&tux_left[7], datadir + "/images/shared/smalltux-left-8.png", USE_ALPHA);

  texture_load(&firetux_right[0], datadir + "/images/shared/firetux-right-0.png", USE_ALPHA);
  texture_load(&firetux_right[1], datadir + "/images/shared/firetux-right-1.png", USE_ALPHA);
  texture_load(&firetux_right[2], datadir + "/images/shared/firetux-right-2.png", USE_ALPHA);

  texture_load(&firetux_left[0], datadir + "/images/shared/firetux-left-0.png", USE_ALPHA);
  texture_load(&firetux_left[1], datadir + "/images/shared/firetux-left-1.png", USE_ALPHA);
  texture_load(&firetux_left[2], datadir + "/images/shared/firetux-left-2.png", USE_ALPHA);


  texture_load(&cape_right[0], datadir + "/images/shared/cape-right-0.png",
               USE_ALPHA);

  texture_load(&cape_right[1], datadir + "/images/shared/cape-right-1.png",
               USE_ALPHA);

  texture_load(&cape_left[0], datadir + "/images/shared/cape-left-0.png",
               USE_ALPHA);

  texture_load(&cape_left[1], datadir + "/images/shared/cape-left-1.png",
               USE_ALPHA);

  texture_load(&bigtux_right[0], datadir + "/images/shared/bigtux-right-0.png",
               USE_ALPHA);

  texture_load(&bigtux_right[1], datadir + "/images/shared/bigtux-right-1.png",
               USE_ALPHA);

  texture_load(&bigtux_right[2], datadir + "/images/shared/bigtux-right-2.png",
               USE_ALPHA);

  texture_load(&bigtux_right_jump, datadir + "/images/shared/bigtux-right-jump.png", USE_ALPHA);

  texture_load(&bigtux_left[0], datadir + "/images/shared/bigtux-left-0.png",
               USE_ALPHA);

  texture_load(&bigtux_left[1], datadir + "/images/shared/bigtux-left-1.png",
               USE_ALPHA);

  texture_load(&bigtux_left[2], datadir + "/images/shared/bigtux-left-2.png",
               USE_ALPHA);

  texture_load(&bigtux_left_jump, datadir + "/images/shared/bigtux-left-jump.png", USE_ALPHA);

  texture_load(&bigcape_right[0], datadir + "/images/shared/bigcape-right-0.png",
               USE_ALPHA);

  texture_load(&bigcape_right[1], datadir + "/images/shared/bigcape-right-1.png",
               USE_ALPHA);

  texture_load(&bigcape_left[0], datadir + "/images/shared/bigcape-left-0.png",
               USE_ALPHA);

  texture_load(&bigcape_left[1], datadir + "/images/shared/bigcape-left-1.png",
               USE_ALPHA);

  texture_load(&bigfiretux_right[0], datadir + "/images/shared/bigfiretux-right-0.png",
               USE_ALPHA);

  texture_load(&bigfiretux_right[1], datadir + "/images/shared/bigfiretux-right-1.png",
               USE_ALPHA);

  texture_load(&bigfiretux_right[2], datadir + "/images/shared/bigfiretux-right-2.png",
               USE_ALPHA);

  texture_load(&bigfiretux_right_jump, datadir + "/images/shared/bigfiretux-right-jump.png", USE_ALPHA);

  texture_load(&bigfiretux_left[0], datadir + "/images/shared/bigfiretux-left-0.png",
               USE_ALPHA);

  texture_load(&bigfiretux_left[1], datadir + "/images/shared/bigfiretux-left-1.png",
               USE_ALPHA);

  texture_load(&bigfiretux_left[2], datadir + "/images/shared/bigfiretux-left-2.png",
               USE_ALPHA);

  texture_load(&bigfiretux_left_jump, datadir + "/images/shared/bigfiretux-left-jump.png", USE_ALPHA);

  texture_load(&bigcape_right[0], datadir + "/images/shared/bigcape-right-0.png",
               USE_ALPHA);

  texture_load(&bigcape_right[1], datadir + "/images/shared/bigcape-right-1.png",
               USE_ALPHA);

  texture_load(&bigcape_left[0], datadir + "/images/shared/bigcape-left-0.png",
               USE_ALPHA);

  texture_load(&bigcape_left[1], datadir + "/images/shared/bigcape-left-1.png",
               USE_ALPHA);


  texture_load(&ducktux_right, datadir +
               "/images/shared/ducktux-right.png",
               USE_ALPHA);

  texture_load(&ducktux_left, datadir +
               "/images/shared/ducktux-left.png",
               USE_ALPHA);

  texture_load(&skidtux_right, datadir +
               "/images/shared/skidtux-right.png",
               USE_ALPHA);

  texture_load(&skidtux_left, datadir +
               "/images/shared/skidtux-left.png",
               USE_ALPHA);

  texture_load(&duckfiretux_right, datadir +
               "/images/shared/duckfiretux-right.png",
               USE_ALPHA);

  texture_load(&duckfiretux_left, datadir +
               "/images/shared/duckfiretux-left.png",
               USE_ALPHA);

  texture_load(&skidfiretux_right, datadir +
               "/images/shared/skidfiretux-right.png",
               USE_ALPHA);

  texture_load(&skidfiretux_left, datadir +
               "/images/shared/skidfiretux-left.png",
               USE_ALPHA);


  /* Boxes: */

  texture_load(&img_box_full, datadir + "/images/shared/box-full.png",
               IGNORE_ALPHA);
  texture_load(&img_box_empty, datadir + "/images/shared/box-empty.png",
               IGNORE_ALPHA);


  /* Water: */


  texture_load(&img_water, datadir + "/images/shared/water.png", IGNORE_ALPHA);

  texture_load(&img_waves[0], datadir + "/images/shared/waves-0.png",
               USE_ALPHA);

  texture_load(&img_waves[1], datadir + "/images/shared/waves-1.png",
               USE_ALPHA);

  texture_load(&img_waves[2], datadir + "/images/shared/waves-2.png",
               USE_ALPHA);


  /* Pole: */

  texture_load(&img_pole, datadir + "/images/shared/pole.png", USE_ALPHA);
  texture_load(&img_poletop, datadir + "/images/shared/poletop.png",
               USE_ALPHA);


  /* Flag: */

  texture_load(&img_flag[0], datadir + "/images/shared/flag-0.png",
               USE_ALPHA);
  texture_load(&img_flag[1], datadir + "/images/shared/flag-1.png",
               USE_ALPHA);


  /* Cloud: */

  texture_load(&img_cloud[0][0], datadir + "/images/shared/cloud-00.png",
               USE_ALPHA);

  texture_load(&img_cloud[0][1], datadir + "/images/shared/cloud-01.png",
               USE_ALPHA);

  texture_load(&img_cloud[0][2], datadir + "/images/shared/cloud-02.png",
               USE_ALPHA);

  texture_load(&img_cloud[0][3], datadir + "/images/shared/cloud-03.png",
               USE_ALPHA);


  texture_load(&img_cloud[1][0], datadir + "/images/shared/cloud-10.png",
               USE_ALPHA);

  texture_load(&img_cloud[1][1], datadir + "/images/shared/cloud-11.png",
               USE_ALPHA);

  texture_load(&img_cloud[1][2], datadir + "/images/shared/cloud-12.png",
               USE_ALPHA);

  texture_load(&img_cloud[1][3], datadir + "/images/shared/cloud-13.png",
               USE_ALPHA);


  /* Bad guys: */
  load_badguy_gfx();

  /* Upgrades: */

  texture_load(&img_mints, datadir + "/images/shared/mints.png", USE_ALPHA);
  texture_load(&img_coffee, datadir + "/images/shared/coffee.png", USE_ALPHA);


  /* Weapons: */

  texture_load(&img_bullet, datadir + "/images/shared/bullet.png", USE_ALPHA);

  texture_load(&img_red_glow, datadir + "/images/shared/red-glow.png",
               USE_ALPHA);



  /* Distros: */

  texture_load(&img_distro[0], datadir + "/images/shared/distro-0.png",
               USE_ALPHA);

  texture_load(&img_distro[1], datadir + "/images/shared/distro-1.png",
               USE_ALPHA);

  texture_load(&img_distro[2], datadir + "/images/shared/distro-2.png",
               USE_ALPHA);

  texture_load(&img_distro[3], datadir + "/images/shared/distro-3.png",
               USE_ALPHA);


  /* Tux life: */

  texture_load(&tux_life, datadir + "/images/shared/tux-life.png",
               USE_ALPHA);

  /* Herring: */

  texture_load(&img_golden_herring, datadir + "/images/shared/golden-herring.png",
               USE_ALPHA);


  /* Super background: */

  texture_load(&img_super_bkgd, datadir + "/images/shared/super-bkgd.png",
               IGNORE_ALPHA);


  /* Sound effects: */

  /* if (use_sound) // this will introduce SERIOUS bugs here ! because "load_sound"
                    // initialize sounds[i] with the correct pointer's value:
                    // NULL or something else. And it will be dangerous to
                    // play with not-initialized pointers.
                    // This is also true with if (use_music)
                    Send a mail to me: neoneurone@users.sf.net, if you have another opinion. :)
  */
  for (i = 0; i < NUM_SOUNDS; i++)
    sounds[i] = load_sound(datadir + soundfilenames[i]);

  /* Herring song */
  herring_song = load_song(datadir + "/music/SALCON.MOD");
}


/* Free shared data: */
void unloadshared(void)
{
  int i;

  for (i = 0; i < 3; i++)
    {
      texture_free(&tux_right[i]);
      texture_free(&tux_left[i]);
      texture_free(&bigtux_right[i]);
      texture_free(&bigtux_left[i]);
    }

  texture_free(&bigtux_right_jump);
  texture_free(&bigtux_left_jump);

  for (i = 0; i < 2; i++)
    {
      texture_free(&cape_right[i]);
      texture_free(&cape_left[i]);
      texture_free(&bigcape_right[i]);
      texture_free(&bigcape_left[i]);
    }

  texture_free(&ducktux_left);
  texture_free(&ducktux_right);

  texture_free(&skidtux_left);
  texture_free(&skidtux_right);

  free_badguy_gfx();

  texture_free(&img_box_full);
  texture_free(&img_box_empty);

  texture_free(&img_water);
  for (i = 0; i < 3; i++)
    texture_free(&img_waves[i]);

  texture_free(&img_pole);
  texture_free(&img_poletop);

  for (i = 0; i < 2; i++)
    texture_free(&img_flag[i]);

  texture_free(&img_mints);
  texture_free(&img_coffee);

  for (i = 0; i < 4; i++)
    {
      texture_free(&img_distro[i]);
      texture_free(&img_cloud[0][i]);
      texture_free(&img_cloud[1][i]);
    }

  texture_free(&img_golden_herring);

  for (i = 0; i < NUM_SOUNDS; i++)
    free_chunk(sounds[i]);

  /* free the herring song */
  free_music( herring_song );
}

/* EOF */

