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
int peers = 0;
char * screen_data = NULL;
int screen_width;
int screen_height;

#define main_display kCGDirectMainDisplay
// #define main_display CGMainDisplayID

/*
#define RELIABLE_PACKET ENET_PACKET_FLAG_RELIABLE
#define UNRELIABLE_PACKET
#define UNSEQUENCED_PACKET ENET_PACKET_FLAG_UNSEQUENCED
*/

void broadcast_chunk(int x, int y, int len)
{
    DataStream stream;
    int pos = (y * screen_width + x);
    stream.write_uint32(pos);
    char * data = screen_data + pos * 3;
    stream.write(data, len * 3);
    ENetPacket * packet = enet_packet_create(stream.data, stream.size,
        0);
    enet_host_broadcast(host, 0, packet);
}

void broadcast_area(int x1, int y1, int x2, int y2)
{
    if (peers == 0)
        return;

    int chunk_size = int((host->mtu / 3.0f) * 0.95f);

    for (int y = y1; y < y2; y++)
    for (int x = x1; x < x2; x += chunk_size) {
        int len = std::min(x2 - x, chunk_size);
        broadcast_chunk(x, y, len);
    }
}

void update_screen_data()
{
    screen_width = CGDisplayPixelsWide(kCGDirectMainDisplay);
    screen_height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
    CGImageRef image_ref = CGDisplayCreateImage(CGMainDisplayID());
    CGDataProviderRef data_ref = CGImageGetDataProvider(image_ref);
    CFDataRef color_data = CGDataProviderCopyData(data_ref);
    CFRange range = CFRangeMake(0, CFDataGetLength(color_data));
    int bpl = CGImageGetBytesPerRow(image_ref);
    char * data = (char*)CFDataGetBytePtr(color_data);

    unsigned int items = screen_width * screen_height;
    unsigned int new_len = items * 3;

    if (screen_data != NULL)
        delete screen_data;

    screen_data = new char[new_len];
    int s_i, m_i;
    for (int y = 0; y < screen_height; y++)
    for (int x = 0; x < screen_width; x++) {
        s_i = (y * screen_width + x) * 3;
        m_i = y * bpl + x * 4;
        screen_data[s_i] = data[m_i];
        screen_data[s_i + 1] = data[m_i + 1];
        screen_data[s_i + 2] = data[m_i + 2];
    }

    CFRelease(color_data);
    CGImageRelease(image_ref);
}

void update_network()
{
    ENetEvent event;
    while (true) {
        if (enet_host_service(host, &event, 1) <= 0)
            break;
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                peers++;
                enet_peer_throttle_configure(event.peer, 10, 1, 0);
                broadcast_area(0, 0, screen_width, screen_height);
                // event.peer->data
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // peer = event.peer->data;
                // packet = (char*)event.packet->data, event.packet->dataLength;
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                peers--;
                // event.peer->data;
                break;
        }
    }
}

void refresh_callback(CGRectCount count, const CGRect *rects, void *ignore)
{
    for (int i = 0; i < count; i++) {
        const CGRect & r = rects[i];
        int x = r.origin.x;
        int y = r.origin.y;
        int width = r.size.width;
        int height = r.size.height;
        CGImageRef img = CGDisplayCreateImageForRect(main_display, r);
        CGDataProviderRef provider = CGImageGetDataProvider(img);
        CFDataRef data = CGDataProviderCopyData(provider);
        int bpl = CGImageGetBytesPerRow(img);
        const char *src = (char*)CFDataGetBytePtr(data);

        int s_i, m_i;
        for (int yy = 0; yy < height; yy++)
        for (int xx = 0; xx < width; xx++) {
            s_i = ((y + yy) * screen_width + (x + xx)) * 3;
            m_i = yy * bpl + xx * 4;
            screen_data[s_i] = src[m_i];
            screen_data[s_i + 1] = src[m_i + 1];
            screen_data[s_i + 2] = src[m_i + 2];
        }

        CFRelease(data);
        CGImageRelease(img);

        broadcast_area(x, y, x + width, y + height);
    }
}

#define UPDATE_RATE (1 / 60.0f)

int main(int argc, char **argv)
{
    init_time();
    enet_initialize();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 7171;

    host = enet_host_create(&address, MAX_CONNECTIONS, CHANNEL_COUNT, 0, 0);
    enet_host_compress_with_range_coder(host);

    if (host == NULL) {
        std::cerr << "Could not initialize host" << std::endl;
        return 0;
    }

    update_screen_data();
    CGRegisterScreenRefreshCallback(refresh_callback, NULL);

    std::cout << "Running osxrd server on port " << address.port << std::endl;

    double debug_time = get_time();

    while (true) {
        update_network();
        RunCurrentEventLoop(kEventDurationSecond * UPDATE_RATE);
        if (get_time() >= debug_time) {
            debug_time += 1.0f;
            std::cout << "data per second: " << host->totalSentData << std::endl;
            host->totalSentData = 0;
        }
    }

    enet_host_destroy(host);
}