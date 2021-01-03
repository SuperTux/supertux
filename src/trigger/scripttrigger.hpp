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

#ifndef HEADER_SUPERTUX_TRIGGER_SCRIPTTRIGGER_HPP
#define HEADER_SUPERTUX_TRIGGER_SCRIPTTRIGGER_HPP

#include "trigger/trigger_base.hpp"

class ReaderMapping;
class Writer;

class ScriptTrigger final : public TriggerBase
{
public:
  ScriptTrigger(const ReaderMapping& reader);
  ScriptTrigger(const Vector& pos, const std::string& script);

  virtual std::string get_class() const override { return "scripttrigger"; }
  std::string get_display_name() const override { return _("Script Trigger"); }
  virtual bool has_variable_size() const override { return true; }

  virtual ObjectSettings get_settings() override;
  virtual void after_editor_set() override;

  virtual void event(Player& player, EventType type) override;
  virtual void draw(DrawingContext& context) override;

  void write(Writer& writer);

private:
  EventType triggerevent;
  std::string script;
  Vector new_size;
  bool must_activate;
  bool oneshot;
  int runcount;
};

#endif

/* EOF */
