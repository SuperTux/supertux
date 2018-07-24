# SuperTux

[![Build Status](https://travis-ci.org/SuperTux/supertux.svg?branch=master)](https://travis-ci.org/SuperTux/supertux)
[![AppVeyor Build Satus](https://ci.appveyor.com/api/projects/status/github/SuperTux/supertux?svg=true&branch=master)](https://ci.appveyor.com/project/supertux/supertux-9ml4d/branch/master)
[![Github All Releases](https://img.shields.io/github/downloads/supertux/supertux/total.svg?maxAge=2592000)](https://github.com/SuperTux/supertux)

SuperTux is a jump'n'run game with strong inspiration from the
Super Mario Bros. games for the various Nintendo platforms.

Run and jump through multiple worlds, fighting off enemies by jumping
on them, bumping them from below or tossing objects at them, grabbing
power-ups and other stuff on the way.

![Screenshot](https://www.supertux.org/images/0_5_1/0_5_1_3.png)


## Story: Penny gets captured!

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

## Installation

For major platforms, stable releases are built and available for download from
[supertux.org](https://www.supertux.org/download.html) or alternatively directly
from [GitHub](https://github.com/SuperTux/supertux/releases). You should be able
to install these using default tools provided by your platform. On macOS, when
Gatekeeper is enabled (default) it will refuse to open SuperTux. This is due to
the lack of a signature on the application. If you wish to open SuperTux anyway
without disabling the Gatekeeper feature entirely, you can open the application
from the context menu (control click on the icon). macOS will then remember your
choice the next time.

## Documentation

Important documentation for SuperTux is contained in multiple files.
Please see them:

* `INSTALL.md` - Requirements, compiling and installing.
* `README.md` - This file
* `NEWS.md` - Changes since the previous versions of SuperTux.
* `LICENSE.txt` - The GNU General Public License, under whose terms SuperTux is
licensed. (Most of the data subdirectory is also licensed under
CC-by-SA)
* `docs/levelguidelines.txt` - Very useful information for those that want to
design levels for SuperTux.
* `data/credits.stxt` - Credits for people that contributed to the creation of
SuperTux. (You can view these in the game menu as well.)


## Playing the game

Both keyboards and joysticks/gamepads are supported. You can change
the controls via the Options menu. Basically, the only keys you will
need to use in-game are to do the following actions: jump, duck,
right, left, action and 'P' to pause/unpause the game. There isn't much
to tell about the first few, but the "action" key allows you to pick
up objects and use any powerup you got. For instance, with the fire
flower, you can shoot fireballs, or with the ice flower fire ice pellets.

Other useful keys include the Esc key, which is used to go to the menu
or to go up a level in the menu. The menu can be navigated using the
arrow keys or the mouse.

In the worldmap, the arrow keys are used to navigate and Enter to
enter the current level.

## Community

In case you need help, feel free to reach out using the following means:

* **IRC:** [#supertux](ircs://chat.freenode.net/#supertux) on
  [freenode](https://freenode.net) hosts most of the discussions between
  developers. Also, real-time support can be provided here. If you don't know
  how to use an IRC client, you access the channel using a web-based
  [client](https://kiwiirc.com/client/chat.freenode.net:+6697/?nick=Guest?#supertux).
  Please stay around after asking questions, otherwise you will be disconnected
  and might miss potential answers.
* **Matrix:** [#supertux:matrix.org](https://matrix.to/#/#supertux:matrix.org)
  is bridged to our IRC room.
* **[Forum](https://forum.freegamedev.net/viewforum.php?f=66):** The SuperTux
  community is very active on the forum, the discussion ranges from feature
  proposals to support questions. In particular, most community-contributed
  add-ons are published there first, so this is worth checking.
* **Mailing Lists:** The
  [supertux-devel](http://lists.lethargik.org/listinfo.cgi/supertux-devel-lethargik.org)
  mailing list is intended for development purposes. However, it is not very
  active at the moment.
* **Social Media:** Mostly on [Twitter](https://twitter.com/supertux_team) at
  the moment.

## Development status

With the release of SuperTux 0.4.0 (December 2015), we wanted to provide a
fairly stable release. The release of SuperTux 0.5.0 should have fixed some
stability issues from 0.4.0 and introduced a new level editor. Development of
the game will continue (of course), now working towards more releases.
Most notably, development focus will be shifted on providing a better gameplay
experience.

The Forest World (you can check it out: Start Game > Contrib Levels > Forest
World) is relatively unstable, which is why it resides only in the contrib
levels menu. We (SuperTux Team) intend to continue with the development of the
Forest World, but decided that in order to allow more access to the most recent
version (in repositories etc.) we would have to release Milestone 2 without the
Forest World included in the Story Mode. Constructive feedback with regards to
the Forest World is welcome.
