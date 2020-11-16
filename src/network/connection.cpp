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

#include "network/connection.hpp"

#include "util/log.hpp"
#include "util/uuid.hpp"

#include <iostream>
#include <thread>

namespace network {

const char Connection::DELIMITER = '\n';

Connection::Connection(tcp::socket* socket, std::function<void(Connection*, std::string)> handler) :
  m_closed(false),
  m_writing_locked(false),
  m_handler(handler),
  m_socket(socket),
  m_rw_mutex(),
  m_data_queue(),
  m_buffer(),
  m_runner(),
  m_uuid(UUID::create_uuid()),
  m_properties()
{
}

Connection::~Connection()
{
  if (!is_closed())
    close();
}

void
Connection::init()
{
  start_reading();
}

void
Connection::start_reading()
{
  boost::asio::async_read_until(*m_socket, m_buffer, DELIMITER,
              boost::bind(&Connection::handle_read,
                          this,
                          boost::asio::placeholders::error,
                          boost::asio::placeholders::bytes_transferred));
}

void
Connection::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
  if (error)
  {
    if (error != boost::asio::error::eof)
      log_warning << "Error when reading from socket: " << error.message() << std::endl;

    close();
  }
  else
  {
    std::istream is(&m_buffer);
    std::string line; 
    std::getline(is, line);
    m_handler(this, line);
    m_buffer.consume(line.length());

    start_reading();
  }
}

void
Connection::handle_write(const boost::system::error_code& error)
{
  if (error)
  {
    log_warning << "Error when writing to socket: " << error << std::endl;
  }

  //FIXME: Race condition
  //m_rw_mutex.unlock();
}

void
Connection::close()
{
  m_closed = true;
  try {
    m_socket->close();
  } catch (std::exception& e) {
    log_info << "Attempt to close socket failed: " << e.what() << std::endl;
  }
}

void
Connection::send(const std::string& data)
{
  if (m_closed)
    // Just ignore it, until I find why some connections become corrupted at some point
    return;

  //FIXME: Race condition
  //m_rw_mutex.lock();

  boost::asio::async_write(*m_socket,
                           boost::asio::buffer(data + DELIMITER),
                           boost::bind(&Connection::handle_write,
                                       this,
                                       boost::asio::placeholders::error));
}

} // namespace network

/* EOF */
