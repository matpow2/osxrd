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
#include "datastream.h"
#include <algorithm>

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

ENetHost * host = NULL;

/*
#define RELIABLE_PACKET ENET_PACKET_FLAG_RELIABLE
#define UNRELIABLE_PACKET
#define UNSEQUENCED_PACKET ENET_PACKET_FLAG_UNSEQUENCED
*/

void broadcast_chunk(char * data, unsigned int pos, int len)
{
    DataStream stream;
    stream.write_uint32(pos);
    stream.write(data, len);

    ENetPacket * packet = enet_packet_create(stream.data, stream.size,
        ENET_PACKET_FLAG_UNSEQUENCED);
    enet_host_broadcast(host, 0, packet);
}

void broadcast_screen()
{
    if (host->peerCount <= 0)
        return;

    int width = CGDisplayPixelsWide(CGMainDisplayID());
    int height = CGDisplayPixelsHigh(CGMainDisplayID());
    CGImageRef image_ref = CGDisplayCreateImage(CGMainDisplayID());
    CGDataProviderRef data_ref = CGImageGetDataProvider(image_ref);
    CFDataRef color_data = CGDataProviderCopyData(data_ref);
    CFRange range = CFRangeMake(0, CFDataGetLength(color_data));

    char * screen_data = new char[range.length];
    CFDataGetBytes(color_data, range, (UInt8*)screen_data);
    CFRelease(color_data);
    CGImageRelease(image_ref);

    unsigned int items = range.length / 4;
    unsigned int new_len = items * 3;
    char * new_data = new char[new_len];
    for (int i = 0; i < items; i++) {
        new_data[i * 3] = screen_data[i * 4];
        new_data[i * 3 + 1] = screen_data[i * 4 + 1];
        new_data[i * 3 + 2] = screen_data[i * 4 + 2];
    }
    delete screen_data;

    for (unsigned int i = 0; i < new_len; i += CHUNK_SIZE) {
        unsigned int len = std::min(new_len - i, (unsigned int)(CHUNK_SIZE));
        broadcast_chunk(new_data, i, len);
    }
    delete new_data;
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

    std::cout << "Running osxrd server on port " << address.port << std::endl;

    double send_time = get_time();

    while (true) {
        update_network();
        if (get_time() >= send_time) {
            send_time = get_time() + UPDATE_RATE;
            broadcast_screen();
        }
        // enet_host_flush(host);
    }

    enet_host_destroy(host);
}