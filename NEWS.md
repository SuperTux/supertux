SuperTux Release and Development Notes
======================================

SuperTux 0.4.0 (TBD)
----------------------------

Tracking changes of next release (0.4.0):
* support for horizontal and vertical gradients
that span the length / height of a sector. Add
(direction "horizontal\_sector / vertical\_sector") to your gradient


SuperTux 0.3.6 (2015-05-01)
----------------------------

A bugfix release is sneaking in. 3 weeks after 0.3.5 was released it's time to
fix some bugs. This affects mostly the windows installer where some libraries
were missing and add-ons couldn't be downloaded. Luckily we managed to get all
of these nasty bugs away.
One more patch affects linux packagers: The AppData file from our repository is
now contained in the source package.
Thanks to the activity of some translators there are also new translations in
this release.

If you want to help us and improve SuperTux, join our IRC channel #supertux on
chat.freenode.net or write to the mailing list!


SuperTux 0.3.5 (2015-04-11)
---------------------------

In celebration of SuperTux's 15 year anniversary, we are releasing SuperTux 0.3.5.

This release includes the switch to SDL2 for graphics rendering, a few performance
improvements, tons of bugfixes, new tiles and badguys amongst other things.

NOTE: Due to the planned shutdown of Google Code, the SuperTux source code has
moved to GitHub. You can find all the repositories here:
https://github.com/SuperTux/supertux

A more detailed changelog for this release is available at
http://supertux.lethargik.org/wiki/Changelog_0.3.5

Major changes in this release:

* move to SDL2 for graphics rendering
* glow effects
* new badguys: iceflame, ghostflame, livefire, goldbomb, smartblock
* new bonuses: coinrain, coinexplode
* statistics improved
* icy island levels tweaked
* new sounds
* massive improvements to localization
* efficiency tweaks
* menus reworked
* addon manager improved
* new tilemap: halloween
* new powerups: air- and earth-flower
* support for horizontal gradients in levels (add
(direction "vertical|horizontal") to your level)


SuperTux 0.3.4 (2013-07)
------------------------

It's been more than three years since the last development snapshot for
Milestone 2 of SuperTux, making this a bit overdue. Once again it is hard to
say what the most notable changes are, but one thing players should notice is
a greater wealth of levels to play through an expanded avalibility of add-ons.

Additionally, SuperTux development has moved to GitHub. Check it out at:
https://github.com/SuperTux/supertux/

If you are intersted in contribuiting to SuperTux, please do so. The more
people working on the project, the faster development can continue. The hope
is that this release will generate more interest which will lead to more
frequent releases. You can refer to the GitHub wiki page on "Contributing" to get
started.

And of course, enjoy the game and have fun!


SuperTux 0.3.3 (2010-02-26)
---------------------------

After more than three years of development the SuperTux development team has
agreed to package a development snapshot of Milestone 2 of our jump and run
game.

Unfortunatly, after such a long time it's hard to say what the most notable
changes are. We hope that the game engine is more stable (or at least more
advanced ;) and that the game is more fun to play overall.


We hope you enjoy SuperTux. Now off you go, rescue Penny! ;)


SuperTux 0.3.2-SVN (2007-12-20)
-------------------------------

Just in time for Christmas, we bring you SuperTux 0.3.2-SVN, a preview
of SuperTux Milestone 2.

This build doesn't contain all levels planned for Milestone 2 and
its story is by no means complete, but the game is already fun to
play.

SuperTux 0.3.2-SVN features:

* new, OpenGL- and OpenAL-based engine
* final boss for Icy Island
* brand new Forest World with new badguys and new game objects
* new and improved soundtrack, immersive sound effects
* much more...

The changes in more detail: The SuperTux 0.1 engine was nearly
completely rewritten. The game is translatable now, a new camera
algorithm allows scrolling in all four directions. Collision
detection supports slopes and moving objects now. We have scripting
support for dynamic level events and animations. New game objects
include trampolines, switches, portable stones, upside-down
levels, wind, moving platforms and particle effects. The sound
system is now OpenAL-based with an improved soundtrack now played
from Ogg/Vorbis-files, which were pre-rendered on a high-class
synthesizer.

This is also a call for artists and coders: If you like the game in
its current state, join us and help us finish Milestone 2!
