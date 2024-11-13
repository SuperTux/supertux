//  SuperTux - End Sequence
//  Copyright (C) 2007 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#ifndef HEADER_SUPERTUX_OBJECT_ENDSEQUENCE_HPP
#define HEADER_SUPERTUX_OBJECT_ENDSEQUENCE_HPP

#include "supertux/game_object.hpp"

#include <unordered_map>

#include "control/codecontroller.hpp"

class EndSequence : public GameObject
{
public:
  EndSequence();
  ~EndSequence() override;
  virtual GameObjectClasses get_class_types() const override { return GameObject::get_class_types().add(typeid(EndSequence)); }

  virtual void update(float dt_sec) override;
  virtual void draw(DrawingContext& context) override;

  void start(); /**< play EndSequence */
  void stop_tux(const UID& player); /**< called when Tux has reached his final position */
  void stop(); /**< stop playing EndSequence, mark it as done playing */
  bool is_running() const; /**< returns true if the ending cinematic started */
  bool is_tux_stopped(const UID& player); /**< returns true if Tux has reached his final position */
  bool is_done() const; /**< returns true if EndSequence has finished playing */

  bool is_singleton() const override { return true; }
  bool is_saveable() const override { return false; }

  const Controller* get_controller(const UID& player);

protected:
  virtual void starting(); /**< called when EndSequence starts */
  virtual void running(float dt_sec); /**< called while the EndSequence is running */
  virtual void stopping(); /**< called when EndSequence stops */

protected:
  struct PlayerData final
  {
    PlayerData();

    bool is_stopped; /**< true while Tux is allowed to walk */
    CodeController controller; /**< the end sequence controller */
  };

protected:
  bool m_is_running; /**< true while EndSequence plays */
  bool m_is_done; /**< true if EndSequence has finished playing */
  std::unordered_map<UID, PlayerData> m_players;

private:
  EndSequence(const EndSequence&) = delete;
  EndSequence& operator=(const EndSequence&) = delete;
};

#endif

/* EOF */
