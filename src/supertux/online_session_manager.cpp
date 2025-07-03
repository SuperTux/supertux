// SuperTux
// Copyright (C) 2025 Martim Ferreira <martim.silva.ferreira@tecnico.ulisboa.pt>
// 2025 Gonçalo Rocha <goncalo.r.f.rocha@tecnico.ulisboa.pt>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "supertux/online_session_manager.hpp"

#include <arpa/inet.h>
#include <condition_variable>
#include <cstring>
#include <ifaddrs.h>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "control/controller.hpp"
#include "control/input_manager.hpp"
#include "control/online_controller.hpp"
#include "object/player.hpp"
#include "supertux/gameconfig.hpp"
#include "supertux/game_manager.hpp"
#include "supertux/game_session.hpp"
#include "supertux/globals.hpp"
#include "supertux/ip_manager.hpp"
#include "supertux/savegame.hpp"
#include "supertux/world.hpp"


OnlineSessionManager::OnlineSessionManager() :
    m_running(false),
    m_server_socket(-1)
{
}

OnlineSessionManager::~OnlineSessionManager()
{
    stop_thread();
}

void OnlineSessionManager::start_thread()
{
    if (m_running) return;

    m_running = true;

    m_server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_server_socket == -1) {
        std::cerr << "Error creating UDP socket" << std::endl;
        m_running = false;
        return;
    }

    int broadcastEnable = 1;
    if (setsockopt(m_server_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) == -1) {
        std::cerr << "Error setting socket to broadcast" << std::endl;
        close(m_server_socket);
        m_running = false;
        return;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, "0.0.0.0", &server_address.sin_addr);
    server_address.sin_port = htons(50001);

    if (bind(m_server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(m_server_socket);
        m_running = false;
        return;
    }

    m_receive_thread = std::thread(&OnlineSessionManager::receive_messages, this);
    m_send_thread = std::thread(&OnlineSessionManager::send_messages, this);
}

void OnlineSessionManager::stop_thread()
{
    if (!m_running) return;

    m_running = false;
    m_cv.notify_all();
    if (m_server_socket != -1) {
        shutdown(m_server_socket, SHUT_RDWR);
        close(m_server_socket);
        m_server_socket = -1;
    }
    if (m_receive_thread.joinable()) {
        m_receive_thread.join();
    }
    if (m_send_thread.joinable()) {
        m_send_thread.join();
    } 
}

void OnlineSessionManager::receive_messages()
{
    //if is_hosting==true 
    //messages are received like this: <mode> ifmode=0:(<what_level_to_go> <what_world_to_go>) ifmode=1:(<x> <y> <last_input>)
    //if is_hosting==false
    //messages are received like this: <player_id> <mode> ifmode=0:(<what_level_to_go>) ifmode=1:(<x> <y> <last_input>)
    while (m_running)
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        sockaddr_in temp_client_address;
        socklen_t client_len = sizeof(temp_client_address);

        int bytes_received = recvfrom(m_server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&temp_client_address, &client_len);
        if (bytes_received < 0)
        {
            if (m_running) {
                std::cerr << "Error receiving UDP data" << std::endl;
            }
            continue;
        }
        else
        {
            bool found = false;
            for (const auto& addr : IPManager::current()->get_ips())
            {
                if (addr.sin_addr.s_addr == temp_client_address.sin_addr.s_addr &&
                    addr.sin_port == temp_client_address.sin_port)
                {
                    found = true;
                    break;
                }
            }
            
            if (g_config->is_hosting)
            {
                if (!found)
                {
                    IPManager::current()->add_ip_addr(temp_client_address);
                    InputManager::current()->push_online_user();
                    if (GameSession::current() && GameSession::current()->get_savegame().get_player_status().m_num_players < InputManager::current()->get_num_users())
                    {
                        GameSession::current()->get_savegame().get_player_status().add_player();
                        InputManager::current()->m_uses_online_controller[1] = true;
                    }
                    else {
                        std::cerr << "Game session not available or player status not initialized." << std::endl;
                    }
                }
                std::string message(buffer);
                std::istringstream iss(message);
                int player_id, mode;
                player_id = 1 + std::distance(
                    IPManager::current()->get_ips().begin(),
                    std::find_if(IPManager::current()->get_ips().begin(), IPManager::current()->get_ips().end(),
                    [&](const sockaddr_in& addr)
                    {
                        return addr.sin_addr.s_addr == temp_client_address.sin_addr.s_addr && addr.sin_port == temp_client_address.sin_port;
                    }
                ));
                iss >> mode;
                if (mode == 0)
                {
                    int what_level_to_go;
                    iss >> what_level_to_go;
                }
                else if (mode == 1)
                {
                    double x, y;
                    std::string last_inputs;
                    iss >> x >> y >> last_inputs;

                    if (player_id >= 0 && player_id < InputManager::current()->get_num_users())
                    {
                        Controller& ctrl = InputManager::current()->get_controller(player_id);
                        OnlineController* online_ctrl = dynamic_cast<OnlineController*>(&ctrl);
                        if (online_ctrl)
                        {
                            online_ctrl->clear_controls();
                            std::istringstream input_stream(last_inputs);
                            std::string token;
                            while (std::getline(input_stream, token, ':'))
                            {
                                try
                                {
                                    int input_num = std::stoi(token);
                                    if (input_num >= 0 && input_num <= static_cast<int>(Control::CONTROLCOUNT))
                                    {
                                        Control input = static_cast<Control>(input_num);
                                        online_ctrl->press(input);
                                    }
                                }
                                catch (const std::invalid_argument&) {
                                    std::cerr << "Invalid input token: " << token << std::endl;
                                }
                            }
                        }
                        else {
                            std::cerr << "Controller for player " << player_id << " is not an OnlineController." << std::endl;
                        }
                        if (GameSession::current() && GameSession::current()->is_playing_in_level())
                        {
                            Player* player = GameSession::current()->get_player(player_id);
                            if (player) {
                                player->set_pos(Vector(x, y));
                            }
                        }
                        
                    }
                    else {
                        std::cerr << "Invalid player_id: " << player_id << std::endl;
                    }
                }
            } 
            else if(!g_config->is_hosting && IPManager::current()->get_ips().size() > 0)
            {
                
                if (IPManager::current()->get_ips().size() > 1) { 
                    IPManager::current()->clear_ips_exept_0();
                }
                std::string message(buffer);
                std::istringstream iss(message);
                int player_id;
                iss >> player_id;
                if (player_id >= InputManager::current()->get_num_users())
                {
                    InputManager::current()->push_online_user();
                    if (GameSession::current() && GameSession::current()->get_savegame().get_player_status().m_num_players < InputManager::current()->get_num_users())
                    {
                        GameSession::current()->get_savegame().get_player_status().add_player();
                        InputManager::current()->m_uses_online_controller[1] = true;
                    }
                    else
                    {
                        std::cerr << "Game session not available or player status not initialized." << std::endl;
                    }
                }
                int mode;
                iss >> mode;
                if (mode == 0)
                {
                    std::string what_level_to_go, what_world_to_go;
                    iss >> what_level_to_go >> what_world_to_go;
                    if (g_config->world.compare(what_world_to_go) != 0 && g_config->level.compare(what_level_to_go) != 0)
                    {
                        g_config->world = what_world_to_go;
                        g_config->level = what_level_to_go;
                        g_config->go_to_level = true;
                    }
                }
                else
                {
                    double x, y;
                    std::string last_inputs;
                    iss >> x >> y >> last_inputs;

                    if (player_id >= 0 && player_id < InputManager::current()->get_num_users())
                    {
                        Controller& ctrl = InputManager::current()->get_controller(player_id);
                        OnlineController* online_ctrl = dynamic_cast<OnlineController*>(&ctrl);
                        if (online_ctrl)
                        {
                            online_ctrl->clear_controls();
                            std::istringstream input_stream(last_inputs);
                            std::string token;
                            while (std::getline(input_stream, token, ':'))
                            {
                                try
                                {
                                    int input_num = std::stoi(token);
                                    if (input_num >= 0 && input_num <= static_cast<int>(Control::CONTROLCOUNT))
                                    {
                                        Control input = static_cast<Control>(input_num);
                                        online_ctrl->press(input);
                                    }
                                }
                                catch (const std::invalid_argument&) {
                                    std::cerr << "Invalid input token: " << token << std::endl;
                                }
                            }
                        }
                        else {
                            std::cerr << "Controller for player " << player_id << " is not an OnlineController." << std::endl;
                        }
                        if (GameSession::current() && GameSession::current()->is_playing_in_level())
                        {
                            Player* player = GameSession::current()->get_player(player_id);
                            if (player) {
                                player->set_pos(Vector(x, y));
                            }
                        }
                        
                    }
                }
            }
        }
    }
}

void OnlineSessionManager::send_messages()
{
    while (m_running)
    {
        try
        {
            std::string message;
            if (GameSession::current() && GameSession::current()->is_playing_in_level())
            {
                std::string level_filename = GameSession::current()->get_level_file();
                size_t last_slash = level_filename.find_last_of('/');
                std::string level = (last_slash != std::string::npos) ? level_filename.substr(last_slash + 1) : level_filename;
                std::string world = GameSession::current()->get_current_world();
                int client_pid = 0;
                if (g_config->is_hosting)
                {
                    for (const auto& client_address : IPManager::current()->get_ips())
                    {
                        client_pid++;
                        bool has_passed_clientid = false;
                        for (int player_id = 0; player_id <= IPManager::current()->get_ips().size(); player_id++)
                        {
                            if (client_pid != player_id)
                            {
                                Player* player = GameSession::current()->get_player(player_id);
                                if (!player) continue;
                                std::string position = std::to_string(player->get_pos().x) + " " + std::to_string(player->get_pos().y);
                                std::string last_input = InputManager::current()->get_controller(player_id).get_last_input();
                                if(!has_passed_clientid)
                                    player_id ++;
                                message = std::to_string(player_id) + " 0 " + level + " " + world;
                                sendto(m_server_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&client_address, sizeof(client_address));
                                message = std::to_string(player_id) + " 1 " + position + " " + last_input;
                                sendto(m_server_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&client_address, sizeof(client_address));
                                if (!has_passed_clientid)
                                    player_id--;
                                InputManager::current()->get_controller(player_id).clear_last_input();
                            }
                            else {
                                has_passed_clientid = true;
                            }
                        }
                    }
                }
                else
                {
                    const sockaddr_in& client_address = IPManager::current()->get_ips().front();
                    std::string position = std::to_string(GameSession::current()->get_player(client_pid)->get_pos().x) + " " + std::to_string(GameSession::current()->get_player(client_pid)->get_pos().y);
                    std::string last_input = InputManager::current()->get_controller(client_pid).get_last_input();
                    message = "0 " + level + " " + world;
                    sendto(m_server_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&client_address, sizeof(client_address));
                    message = "1 " + position + " " + last_input;
                    sendto(m_server_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&client_address, sizeof(client_address));
                    InputManager::current()->get_controller(client_pid).clear_last_input();
                }
            }
            else if (g_config->is_joining)
            {
                const sockaddr_in& client_address = IPManager::current()->get_ips().front();
                message = "2";
                sendto(m_server_socket, message.c_str(), message.length(), 0, (struct sockaddr*)&client_address, sizeof(client_address));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
        
    }
}

/* EOF */
