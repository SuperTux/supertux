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

class InfoBlock final : public Block
{
public:
  InfoBlock(const ReaderMapping& mapping);
  ~InfoBlock() override;

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  virtual std::string get_class() const override { return "infoblock"; }
  virtual std::string get_display_name() const override { return _("Info Block"); }

  virtual ObjectSettings get_settings() override;

  void show_message();
  void hide_message();

private:
  virtual void hit(Player& player) override;
  virtual HitResponse collision(GameObject& other, const CollisionHit& hit) override;

  Player* get_nearest_player() const;

private:
  std::string m_message;
  //AmbientSound* m_ringing;
  //bool m_stopped;
  float m_shown_pct; /**< Value in the range of 0..1, depending on how much of the infobox is currently shown */
  float m_dest_pct; /**< With each call to update(), shown_pct will slowly transition to this value */
  std::vector<std::unique_ptr<InfoBoxLine> > m_lines; /**< lines of text (or images) to display */
  float m_lines_height;
  Color m_frontcolor;
  Color m_backcolor;
  float m_roundness;
  bool m_fadetransition;

private:
  InfoBlock(const InfoBlock&) = delete;
  InfoBlock& operator=(const InfoBlock&) = delete;
};

#endif

/* EOF */
