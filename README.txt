README.txt for Super Tux

by Bill Kendrick
bill@newbreedsoftware.com
http://www.newbreedsoftware.com/supertux/

Version 0.0.3

November 14, 2001


NOTICE!  THIS GAME IS UNDER CONSTRUCTION!
Things you'll notice:

  Bugs
  Documentation is quite incomplete
  Only one level


DESCRIPTION
-----------
  "Super Tux" is a game similar to Nintendo's "Super Mario Bros."(tm)
  Run and jump through multiple worlds, fighting off enemies by jumping
  on them or bumping them from below.  Grab power-ups and Linux distributions
  on the way.


STORY
-----
  Tux and Gown are having a picnic in Antarctica, when suddenly Gown is
  abducted!  Tux must follow the path of clues across the globe to find
  his girlfriend and save her!


DOCUMENTATION
-------------
  Important documentation for "Super Tux" is contained in multiple files.
  Please see them:

    AUTHORS.txt    - Credits for who made this game.
    CHANGES.txt    - Changes since the previous versions of "Super Tux"
    COPYING.txt    - The GNU Public License, which "Super Tux" is under.
    INSTALL.txt    - Instructions on requirements, compiling and installing.
    README.txt     - (This file.)  Game story, usage, rules and controls.
    TODO.txt       - A wish-list for this version of "Super Tux"


RUNNING THE GAME
----------------
  Just type "./supertux" to get started.

  The program accepts some options:

UNDER CONSTRUCTION

    --disable-sound     - If sound support was compiled in, this will
                          disable it for this session of the game.

    --fullscreen        - Run in fullscreen mode.

    --help              - Display a help message summarizing command-line
                          options, copyright, and game controls.

    --usage             - Display a brief message summarizing command-line
                          options.

    --version           - Display the version of Super Tux you're running.


TITLE SCREEN
------------
  Press [RETURN] or a joystick firebutton to begin a game.

  Press [ESCAPE] to quit.

  Note: If you're playing Super Tux in a window, you can also use
  your window manager's close option (usually an "X" button on the window's
  title bar) to quit.  This works at ANY time.


SCREEN LAYOUT
-------------
  UNDER CONSTRUCTION

    +----------------------------------------+
    |SCORE 1230    TIME 128     DISTROS 93   |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    |                                        |
    +----------------------------------------+


  Status
  ------
    Your score is displayed at the upper left.

    The amount of time you have left to complete this level is displayed in
    the center at the top of the screen.  (Note: Time is NOT in seconds!)

    The number of distros (CDs) you have collected is displayed at the upper
    right.


PLAYING THE GAME
----------------
  Controls
  --------
    Joystick        Keyboard     Purpose
    --------        --------     -------
      Left            Left         Face/Move Left
      Right           Right        Face/Move Right
      Button-A        Up           Jump
      Button-B        Control      Run/Fire
      Down            Down         Duck

  Pausing
  -------
    NOT IMPLEMENTED

    To pause the game, you can press either the [TAB] or [P] key.
    To unpause, press one of those keys again.

  Quitting
  --------
    To quit the current game, you can press [ESCAPE].  This forfeits your game
    (as though you ran out of lives).

    To quit Super Tux altogether, you can use your window manager's
    close command.  See the note in the "Title Screen" section, above.


SCORING
-------
  Enemies
  -------
    If you get more than one enemy at a time without landing on the ground,
    each enemy is worth more points.  For example, if there are three in a
    row and you jump on the first, it's worth 50 points.  If you bounce off
    the first and onto the second, it's worth 100 points.  If you also get
    the third, it's worth 150 points.


GAME OVER SCREEN
----------------
  NOT IMPLEMENTED


OPTIONS FILE
------------
  NOT IMPLEMENTED

  When Super Tux starts up, it looks for an options file.

  Under Linux, this file is ".supertux" in your home directory ($HOME).
  Under Windows, this file is "supertux.dat" in the Super Tux folder.

  This file records high score scores and initials, and volume settings.


THE END
-------
  Thanks for trying out Super Tux.

  If you like Super Tux, please rate and review it at the
  Linux Game Tome:

    http://www.happypenguin.org/

  Search for "Super Tux"!


  Thanks!

  Bill Kendrick  <bill@newbreedsoftware.com>
