[![Build Status](https://travis-ci.org/SuperTux/supertux.svg?branch=master)](https://travis-ci.org/SuperTuxTeam/supertux)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/4025/badge.svg)](https://scan.coverity.com/projects/4025)
SuperTux - <http://supertux.lethargik.org/>
===========================================
Last update: February 26, 2010

Description
-----------

SuperTux is a jump'n'run game with strong inspiration from the
Super Mario Bros. games for the various Nintendo platforms.

Run and jump through multiple worlds, fighting off enemies by jumping
on them, bumping them from below or tossing objects at them, grabbing
power-ups and other stuff on the way.


Story: Penny gets captured!
---------------------------

Tux and Penny were out having a nice picnic on the ice fields of
Antarctica. Suddenly, a creature jumped from behind an ice bush, there
was a flash, and Tux fell asleep!

When Tux wakes up, he finds that Penny is missing. Where she lay
before now lies a letter. "Tux, my arch enemy!" says the letter. "I
have captured your beautiful Penny and have taken her to my fortress.
The path to my fortress is littered with my minions. Give up on the
thought of trying to reclaim her, you haven't got a chance! -Nolok"

Tux looks and sees Nolok's fortress in the distance. Determined to
save his beloved Penny, he begins his journey.


Documentation
-------------
Important documentation for SuperTux is contained in multiple files.
Please see them:

`INSTALL.md`
: Requirements, compiling and installing.

`README.md`
: This file

`WHATSNEW.txt`
: Changes since the previous versions of SuperTux.

`LICENSE`
: The GNU General Public License, under whose terms SuperTux is
licensed. (Most of the data subdirectory is also licensed under
CC-by-SA)

`docs/levelguidelines.txt`
: Very useful information for those that want to design levels for
SuperTux.

`data/credits.txt`
: Credits for people that contributed to the creationof SuperTux. (You
can view these in the game menuas well.)

`data/README`
: Licensing information about the data.


Running the game
----------------

SuperTux makes use of proc to see where it is. In other words, it does
not have any need to be installed and can be run from anywhere. This
makes it possible to click in the executable in your filemanager (i.e.
Konqueror or Nautilus) as opposed to many other Linux games.

Options can be reached from the menu, so you don't need to specify
arguments, but if you want, type `supertux2 --help` to check the ones
that are available. Also, notice that SuperTux saves the options, so
it's often enough to specify them once. For example, fullscreen mode
causes problems on some setups, so just run `supertux2 --window` and
you should be set.

The game uses OpenGL to render the graphics. You will either need a
CPU with about 10 GHz or an accelerated video card with the vendor's
drivers. (On Linux, the team recommends using cards from NVidia with
the proprietary drivers, but ATI or another vendor should do.)


Playing the game
----------------

Both keyboards and joysticks/gamepads are supported. You can change
the controls via the Options menu. Basically, the only keys you will
need to use in-game are to do the following actions: jump, duck,
right, left, power and 'P' to pause/unpause the game. There isn't much
to tell about the first few, but the "action" key allows you to pick
up objects and use any powerup you got. For instance, with the fire
flower, you can shoot bullets (note that this is the only power
currently implemented).

Other useful keys include the Esc key, which is used to go to the menu
or to go up a level in the menu. The menu can be navigated using the
arrow keys or the mouse.

In the worldmap, the arrow keys are used to navigate and Enter to
enter the current level.


Development status
------------------

This version of SuperTux is still under development, even though the
0.3 line of versions is somewhat aged. This means that badguys,
features, levels, graphics or anything else may be removed in the
future.

Especially the Forest world included in this package may be subject to
drastic changes. These changes may go as far as removing the world
altogether. Currently (February 2010) there is no consensus within the
SuperTux development community regarding the fate of the Forest. Don't
get too attached to it ;)


The End
-------

Thanks for trying out SuperTux.

The SuperTux development team
