//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef __INFOBLOCK_H__
#define __INFOBLOCK_H__

#include "block.hpp"
//#include "object/ambient_sound.hpp"
#include "textscroller.hpp"

class InfoBlock : public Block
{
public:
  InfoBlock(const lisp::Lisp& lisp);
  virtual ~InfoBlock();
  void update(float elapsed_time);
  void draw(DrawingContext& context);

  void show_message();
  void hide_message();

protected:
  virtual void hit(Player& player);
  std::string message;
  //AmbientSound* ringing;
  //bool stopped;
  float shown_pct; /**< Value in the range of 0..1, depending on how much of the infobox is currently shown */
  float dest_pct; /**< With each call to update(), shown_pct will slowly transition to this value */

  Player* get_nearest_player();

  std::vector<InfoBoxLine*> lines; /**< lines of text (or images) to display */
  float lines_height;
};

#endif
