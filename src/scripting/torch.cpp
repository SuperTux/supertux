//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2017 M. Teufel <mteufel@supertux.org>
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

#include "object/torch.hpp"
#include "scripting/torch.hpp"

namespace scripting {

Torch::Torch(::Torch* torch_)
  : torch(torch_)
{ }

Torch::~Torch()
{ }

bool Torch::get_burning() const
{
  return torch->get_burning();
}

void Torch::set_burning(bool burning)
{
  torch->set_burning(burning);
}

}

/* EOF */
