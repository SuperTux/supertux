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

// TODO: Make ClientTCP and ClientUDP
class Connection
{
public:
  Connection(tcp::socket* socket, std::function<void(Connection*, const std::string&)> handler);
  virtual ~Connection();

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

  void start_reading();

  const std::string& get_uuid() const { return m_uuid; }
  bool operator ==(Connection& c) const { return m_uuid == c.m_uuid; }
  bool operator !=(Connection& c) const { return m_uuid != c.m_uuid; }

private:
  void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
  void handle_write(const boost::system::error_code& error);

public:
  static std::unique_ptr<Connection> connect_to(int port, const std::string& remote_address,
                                             std::function<void(Connection*, const std::string&)> handler);

protected:
  bool m_closed, m_writing_locked;
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
