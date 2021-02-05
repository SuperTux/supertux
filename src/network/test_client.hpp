//  SuperTux
//  Copyright (C) 2020 A. Semphris <semphris@protonmail.com>
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

#ifndef HEADER_SUPERTUX_NETWORK_TEST_CLIENT_HPP
#define HEADER_SUPERTUX_NETWORK_TEST_CLIENT_HPP

#include "network/client.hpp"
#include "supertux/screen.hpp"

#include <string>

class TestClient final : public Screen
{
public:
  TestClient();
  virtual ~TestClient();

  virtual void setup() override;

  virtual void draw(Compositor& compositor) override {}
  virtual void update(float dt_sec, const Controller& controller) override {}

  virtual IntegrationStatus get_status() const override { return IntegrationStatus(); }

private:
  network::Client* m_client;

private:
  TestClient(const TestClient&) = delete;
  TestClient& operator=(const TestClient&) = delete;
};

#endif

/* EOF */
