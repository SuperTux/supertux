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

#include "network/server.hpp"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "network/connection.hpp"
#include "util/log.hpp"

namespace network {

using namespace boost::asio;
using ip::tcp;

Server::Server(int port, std::function<void(std::unique_ptr<Connection>)> handler,
         std::function<void(Connection*, std::string)> default_connection_handler) :
  m_port(port),
  m_handler(handler),
  m_default_connection_handler(default_connection_handler),
  m_stopped(false),
  m_runner(),
  io_service(),
  acceptor(io_service, tcp::endpoint(tcp::v4(), static_cast<short unsigned int>(port)))
{
}

Server::~Server()
{
}

void
Server::start()
{
  listen_one();

  m_runner = std::make_unique<std::thread>([this](){
    io_service.run();
  });

  m_runner->detach();
}

void
Server::poll()
{
  io_service.poll();
}

void
Server::listen_one()
{
  // usage of "new" here is intended
  // the async_accept() function cannot deal with unique_ptrs, so it'll be a
  // raw pointer until the other side of the function (in handle_accept()).
  tcp::socket* socket = new tcp::socket(io_service);
  acceptor.async_accept(*socket,
                        boost::bind(&Server::handle_accept,
                                    this,
                                    socket,
                                    boost::asio::placeholders::error));
}

void
Server::handle_accept(tcp::socket* socket, const boost::system::error_code& err)
{
  if (!err)
  {
    std::unique_ptr<Connection> connection = std::make_unique<Connection>(socket, m_default_connection_handler);
    connection->init();
    m_handler(std::move(connection));
  }
  else
  {
    // FIXME: Do not warn if the error is "system:125" (a. k. a. the server was closed)
    log_warning << "Incoming connection errored: " << err << std::endl;
  }

  listen_one();
}

void
Server::stop()
{
  try
  {
    boost::system::error_code ec;
    acceptor.cancel(ec);
    acceptor.close(ec);
  }
  catch(std::exception& e_)
  {
    log_warning << "Could not stop server: " << e_.what() << std::endl;
  }
}


void
Server::close()
{
  try
  {
    stop();

    if(!io_service.stopped())
    {
      io_service.stop();
    }
  }
  catch(std::exception& e_)
  {
    log_warning << "Could not clost server: " << e_.what() << std::endl;
  }
}

} // namespace network

/* EOF */
