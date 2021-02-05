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

#ifndef HEADER_SUPERTUX_NETWORK_CONNECTION_HPP
#define HEADER_SUPERTUX_NETWORK_CONNECTION_HPP

#include <functional>
#include <mutex>
#include <thread>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

namespace network {

/**
 * A base networking connection. Sends and receives data.
 * 
 * IMPORTANT:
 * 
 * This class isn't made to be used directly, but rather through a ConnectionPtr object.
 * 
 * The reason is that the object's existence is required by Boost to receive data; if the
 * object is deleted directly as with a traditional unique_ptr, Boost would send the
 * received data to a dead object, causing undefined behavior and often a segfault.
 * 
 * The object's deletion is taken care of through the Connection::delete() function. The
 * object will delete itself whenever it is ready to do so. The object must often survive
 * until after its reference is lost or destroyed, thus requiring the need to be handled
 * through a pointer. The ConnectionPtr class takes care of all of this already.
 */
class Connection
{
protected:
  /** Do NOT call this function - use ConnectionPtr's or call destroy() */
  virtual ~Connection();

public:
  Connection(tcp::socket* socket, std::function<void(Connection*, const std::string&)> handler);

  /** Closes the connection */
  virtual void close();

  /** Sends data to the remote instance */
  virtual void send(const std::string& data);

  /** Initiates read/write operations */
  virtual void init();

  /**
   * Changes the handler for receiving data
   * @returns the old handler
   */
  auto swap_handler(std::function<void(Connection*, const std::string&)> handler)
  {
    auto old_handler = m_handler;
    m_handler = handler;
    return old_handler;
  }

  bool is_closed() const { return m_closed; }

  /**
   * Instead of destroying the object the classic way, use this function to
   * not risk causing a segfault (thanks to Boost...)
   */
  void destroy();

  void start_reading();

  const std::string& get_uuid() const { return m_uuid; }
  bool operator ==(Connection& c) const { return m_uuid == c.m_uuid; }
  bool operator !=(Connection& c) const { return m_uuid != c.m_uuid; }

private:
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
  void handle_write(const boost::system::error_code& error);

protected:
  bool m_closed, m_writing_locked, m_ready_for_deletion, m_should_be_destroyed;
  std::function<void(Connection*, const std::string&)> m_handler;
  tcp::socket* m_socket;
  std::mutex m_rw_mutex;

private:
  std::string m_data_queue;
  boost::asio::streambuf m_buffer;
  std::unique_ptr<std::thread> m_runner;
  std::string m_uuid;

public:
  /** Custom connection properties; used by the game to remember details about the connection */
  std::map<std::string,void*> m_properties;

public:
  static const char DELIMITER;

private:
  Connection(const Connection&) = delete;
  Connection operator=(const Connection&) = delete;
};

} // namespace network

#endif

/* EOF */
