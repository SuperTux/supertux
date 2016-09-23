SuperTux Release and Development Notes
======================================

SuperTux 0.5.0-rc.5 (2016-09-23)
--------------------------------

The SuperTux team is excited to announce the availability of the fourth release candidate for the upcoming stable release 0.5.0 after less than a year of development. The most prominent change for this release is a new in-game level editor which allows you to create levels and worldmaps on-the-fly from within SuperTux itself.

Changes:

* In-game level editor
* Improved levels in Antarctica and Forest Island
* Language packs are fixed
* Engine performance improvements
* Extended the scripting API: gradients are now scriptable
* Added a few more tiles and music
* New console commands and command line options (related to the editor)
* Various other bugfixes of issues reported since the v0.4.0 release

Changes from 0.5.0-rc.1 (2016-08-05):

* Fixed build system in order to correctly support SemVer pre-release tags
* Fixed miniswig compilation
* Editor: correctly join paths (using FileSystem::join())
* Editor: LevelDot: add a NULL check to prevent a crash when editing worldmaps
* Editor: correctly save BadGuy and Dispenser direction
* Editor: reorder layer objects when their object menu is closed
* Editor: allow setting z-pos of decals
* Editor: display a notile when the tile ID is invalid
* Editor: make it possible to switch tilesets

Changes from 0.5.0-rc.2 (2016-08-10):

* Editor: Torch: make it possible to change the sprite (#541)
* Don't turn Tux into stone above ground when using earth bonus (#537)
* WillOWisp: get name property in order to fix a scripting issue (#529)
* Editor: don't play WillOWisp sound
* Editor: move moving objects to the center of the mouse pointer when adding
* Editor: Switch: make sprite option visible again
* Make use of auto pointers and modern iterators where appropriate
* README: various updates related to download count
* Save and load visibility of SpriteChange tiles in squirrel table (#342)
* Editor: Fix an issue where the name of a scripted object wouldn't be saved (#557)
* Editor: add an alignment option for background (#547)
* Editor: Better approach for layer removal (#510)
* Add a launcher script for Windows builds (#443)

Changes from 0.5.0-rc.3 (2016-08-26):

* Fixes for the build system and updates with regard to the automatic deployment
  of releases
* More code style fixes
* Editor: verify some level properties have been set
* Add the possibility to add sawblade/hurting platforms using the editor
* Jump with Up: mark as deprecated in KeyboardMenu and JoystickMenu (addresses
  #499)
* Fix an issue with odd behavior of music playback (#583)
* Update some parts of the (user) documentation
* Editor: multiple fixes to levelset and worldmap creation, including UX
  improvements
* Editor: Angle option for spotlight
* Unify string spelling for consistency
* String freeze for translators is now in effect
* Editor: sort layer in ascending order
* Scripting: deduplicate code using the ExposedObject template class
* Update the translations to reflect the translation progress as of 2016-09-12

Changes from 0.5.0-rc.4 (2016-09-12):

* Use PhysFS file system abstraction for is\_directory checks
* Prevent statistics text from overlapping
* Fix a bug with Level.edit() in squirrel (#207)

This section will be updated with more RCs or a stable version being released.

SuperTux 0.4.0 (2015-12-20)
----------------------------

Compared to 0.1.3, this release features:

* a nearly completely rewritten game engine based on OpenGL, OpenAL, SDL2, ...
* support for translations
* in-game manager for downloadable add-ons and translations
* Bonus Island III, a for now unfinished Forest Island and the development levels in Incubator Island
* a final boss in Icy Island
* new and improved soundtracks and sound effects
* new badguys, bonuses and power-ups (air-, earth- and ice-flower)
* a halloween tilemap
* new graphic effects (glowing objects, particles, ...)
* levels and worldmaps are scriptable using [squirrel](http://squirrel-lang.org/)
* much more game objects: trampolines, switches, portable stones, wind, moving platforms, ... - most of them have scripting APIs
* improved statistics
* many invisible changes, like unit tests, efficiency improvements and more bugfixes
* much more...

Compared to the latest beta, 0.3.5a, this release features:

* updated translations
* reworked add-on manager to support downloadable and updatable localization packs
* many bugfixes
* added iced-graphics to some more badguys
* melting animation for some badguys
* updated more levels (fix playability, ...)
* updated build system, featuring unit tests, libraries as git submodules and improved Travis-CI builds to support nightly builds
* more levels in Incubator Island
* support for horizontal and vertical gradients that span the length / height of a sector. Add `(direction "horizontal_sector / vertical_sector")` to your gradient.
* much more...

SuperTux 0.3.5a (2015-05-01)
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
