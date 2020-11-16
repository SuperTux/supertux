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

#ifndef HEADER_SUPERTUX_NETWORK_SERVER_HPP
#define HEADER_SUPERTUX_NETWORK_SERVER_HPP

#include "network/connection.hpp"

#include <functional>
#include <memory>
#include <thread>

namespace network {

// TODO: Make ServerTCP and ServertUDP
class Server
{
public:
  /* @param port The port to listen to
     @param handler What to do when a new connection arrives.
            WARNING: the handler will be called form a DIFFERENT thread! */
  Server(int port, std::function<void(std::unique_ptr<Connection>)> handler,
         std::function<void(Connection*, std::string)> default_connection_handler);

  virtual ~Server();

  /** Start accepting connections */
  virtual void start();
  /** Stop accepting connections */
  virtual void stop();
  /** Stop, then shut down all connections (Note: Calls stop()) */
  virtual void close();
  /** Polls for connections */
  virtual void poll();

private:
  /* Handle incoming connection */
  void handle_accept(tcp::socket* socket, const boost::system::error_code& err);

  /** Starts the async listener for the next incoming connection */
  void listen_one();

protected:
  int m_port;

  /** What to do when a connection begins
   *  @param The Connection object which was created
   */
  std::function<void(std::unique_ptr<Connection>)> m_handler;

  /** What handler to attach to the new connections by default
   *  @param The Connection object which was created
   *  @param The data received from the connection
   */
  std::function<void(Connection*, std::string)> m_default_connection_handler;

  /** Whether or not the server is listening for new connections */
  bool m_stopped;

  /** Thread used to handle connections */
  std::unique_ptr<std::thread> m_runner;

  // Internal - used by Boost
  boost::asio::io_service io_service;
  tcp::acceptor acceptor;

private:
  Server(const Server&) = delete;
  Server operator=(const Server&) = delete;
};

} // namespace network

#endif

/* EOF */
