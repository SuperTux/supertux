//  SuperTux
//  Copyright (C) 2024 Vankata453
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

#ifndef HEADER_SUPERTUX_SUPERTUX_MENU_ASYNC_DIALOG_HPP
#define HEADER_SUPERTUX_SUPERTUX_MENU_ASYNC_DIALOG_HPP

#include "gui/dialog.hpp"

#include <functional>
#include <future>

#include "gui/menu_manager.hpp"

template<typename T>
class AsyncDialog final : public Dialog
{
public:
  AsyncDialog(const std::string& text, std::future<T> future, std::function<void(T)> callback);

  void update() override;

private:
  std::future<T> m_future;
  const std::function<void(T)> m_callback;

private:
  AsyncDialog(const AsyncDialog&) = delete;
  AsyncDialog& operator=(const AsyncDialog&) = delete;
};


/** SOURCE */

template<typename T, typename... Args>
inline std::unique_ptr<AsyncDialog<T>> make_async_dialog(Args&&... args)
{
  return std::make_unique<AsyncDialog<T>>(std::forward<Args>(args)...);
}

template<typename T>
AsyncDialog<T>::AsyncDialog(const std::string& text, std::future<T> future, std::function<void(T)> callback) :
  m_future(std::move(future)),
  m_callback(std::move(callback))
{
  set_text(text);
}

template<typename T>
void
AsyncDialog<T>::update()
{
  using namespace std::chrono_literals;

  if (m_future.wait_for(0ms) != std::future_status::ready)
    return;

  m_callback(std::move(m_future.get()));
  MenuManager::instance().set_dialog({});
}

#endif

/* EOF */
