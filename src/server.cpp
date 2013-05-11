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

#define MAX_CONNECTIONS 5

#include <iostream>
#include <enet/enet.h>
#include "timer.h"
#include "constants.h"

#include <Carbon/Carbon.h>

ENetHost * host = NULL;
void * screen_data = NULL;

/*
#define RELIABLE_PACKET ENET_PACKET_FLAG_RELIABLE
#define UNRELIABLE_PACKET
#define UNSEQUENCED_PACKET ENET_PACKET_FLAG_UNSEQUENCED
*/

void broadcast_screen()
{
    int width = CGDisplayPixelsWide(CGMainDisplayID());
    int height = CGDisplayPixelsHigh(CGMainDisplayID());
    CGImageRef image_ref = CGDisplayCreateImage(CGMainDisplayID());
    CGDataProviderRef data_ref = CGImageGetDataProvider(image_ref);
    CFDataRef color_data = CGDataProviderCopyData(data_ref);
    CFRange range = CFRangeMake(0, CFDataGetLength(color_data));
    
    if (screen_data != NULL)
        free(screen_data);

    screen_data = malloc(range.length);
    CFDataGetBytes(color_data, range, (UInt8*)screen_data);
    CFRelease(color_data);
    CGImageRelease(image_ref);
    ENetPacket * packet = enet_packet_create(screen_data, range.length,
        ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(host, 0, packet);
}

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

#define UPDATE_RATE (1 / 30.0f)

int main(int argc, char **argv)
{
    init_time();
    enet_initialize();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 7171;

    host = enet_host_create(&address, MAX_CONNECTIONS, CHANNEL_COUNT, 0, 0);

    if (host == NULL) {
        std::cerr << "Could not initialize host" << std::endl;
        return 0;
    }

    enet_host_compress_with_range_coder(host);

    std::cout << "Running osxrd server on port " << address.port << std::endl;

    double send_time = get_time();

    while (true) {
        update_network();
        if (get_time() < send_time)
            continue;
        send_time = get_time() + UPDATE_RATE;
        broadcast_screen();
        enet_host_flush(host);
    }

    enet_host_destroy(host);
}