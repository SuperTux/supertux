//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_OBJECT_INFOBLOCK_HPP
#define HEADER_SUPERTUX_OBJECT_INFOBLOCK_HPP

#include <memory>

#include "object/block.hpp"

class InfoBoxLine;

class InfoBlock : public Block
{
public:
  InfoBlock(const ReaderMapping& lisp);
  virtual ~InfoBlock();
  void update(float elapsed_time);
  void draw(DrawingContext& context);

  void show_message();
  void hide_message();
  std::string get_class() const {
    return "infoblock";
  }
  std::string get_display_name() const {
    return _("Info block");
  }

  virtual ObjectSettings get_settings();

protected:
  virtual void hit(Player& player);
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit);
  Player* get_nearest_player() const;

protected:
  std::string message;
  //AmbientSound* ringing;
  //bool stopped;
  float shown_pct; /**< Value in the range of 0..1, depending on how much of the infobox is currently shown */
  float dest_pct; /**< With each call to update(), shown_pct will slowly transition to this value */
  std::vector<std::unique_ptr<InfoBoxLine> > lines; /**< lines of text (or images) to display */
  float lines_height;
};

#endif

/* EOF */
