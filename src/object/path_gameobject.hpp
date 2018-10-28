//  SuperTux
//  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SUPERTUX_OBJECT_PATH_GAMEOBJECT_HPP
#define HEADER_SUPERTUX_OBJECT_PATH_GAMEOBJECT_HPP

#include "supertux/game_object.hpp"

class Path;
class Vector;

class PathGameObject : public GameObject
{
public:
  PathGameObject();
  PathGameObject(const Vector& pos);
  PathGameObject(const ReaderMapping& mapping);

  virtual void update(float elapsed_time) override;
  virtual void draw(DrawingContext& context) override;

  Path& get_path() { return *m_path; }

  virtual void save(Writer& writer) override;

private:
  std::unique_ptr<Path> m_path;

private:
  PathGameObject(const PathGameObject&) = delete;
  PathGameObject& operator=(const PathGameObject&) = delete;
};

#endif

/* EOF */
