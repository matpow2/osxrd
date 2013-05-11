/*    
    Copyright (c) 2013 Mathias Kaerlev

    This file is part of osxrd.

    osxrd is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    osxrd is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with osxrd.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <enet/enet.h>
#include "timer.h"
#include "constants.h"

ENetHost * host = NULL;
ENetPeer * peer = NULL;

void update_network()
{
    ENetEvent event;
    while (true) {
        if (enet_host_service(host, &event, 1) <= 0)
            break;
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                // event.peer->data
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // peer = event.peer->data;
                // packet = (char*)event.packet->data, event.packet->dataLength;
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                // event.peer->data;
                break;
        }
    }
}

int main(int argc, char **argv)
{
    enet_initialize();

    host = enet_host_create(NULL, 1, CHANNEL_COUNT, 0, 0);

    if (host == NULL) {
        std::cerr << "Could not initialize host" << std::endl;
        return 0;
    }

    enet_host_compress_with_range_coder(host);

    ENetAddress address;
    enet_address_set_host(&address, "192.168.123.15");
    peer = enet_host_connect(host, &address, CHANNEL_COUNT, 0);

    std::cout << "Running osxrd client" << std::endl;

    while (true) {
        update_network();
    }

    enet_host_destroy(host);
}