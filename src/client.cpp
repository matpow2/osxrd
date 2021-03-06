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
#include "include_gl.h"
#include <fstream>
#include "tinythread.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

#include "datastream.h"

#ifndef GL_BGRA
#define GL_BGR 0x80E0
#endif

ENetHost * host = NULL;
ENetPeer * peer = NULL;
GLFWwindow * window = NULL;
char * screen_data;
GLuint screen_tex;
bool application_exit = false;

void write_file(const char * filename, char * data, unsigned int len)
{
    std::ofstream fp(filename, std::ios::binary | std::ios::out);
    fp.write(data, len);
    fp.close();
}

void set_screen_data(char * data, unsigned int len)
{
    DataStream stream(data, len);
    int pos = stream.read_uint32();

    data += 4;
    len -= 4;

    memcpy(screen_data + pos * 3, data, len);
}

void network_loop(void * arg)
{
    ENetEvent event;
    while (!application_exit) {
        if (enet_host_service(host, &event, 10) <= 0)
            continue;
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                // event.peer->data
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                // peer = event.peer->data;
                // packet = (char*)event.packet->data, event.packet->dataLength;
                set_screen_data((char*)event.packet->data, 
                                event.packet->dataLength);
                enet_packet_destroy(event.packet);
                // std::cout << "Received update" << std::endl;
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                // event.peer->data;
                break;
        }
    }
}

void _error_callback(int error, const char * msg)
{
    std::cout << "Window error (" << error << "): " << msg << std::endl;
}

void draw()
{

    glViewport(0, 0, 1024, 768);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1024, 0, 768, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, screen_tex);

    int w = 1024;
    int h = 768;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGR,
        GL_UNSIGNED_BYTE, screen_data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    float tex_coords[8];
    tex_coords[0] = 0.0; tex_coords[1] = 1.0;
    tex_coords[2] = 1.0; tex_coords[3] = 1.0;
    tex_coords[4] = 1.0; tex_coords[5] = 0.0;
    tex_coords[6] = 0.0; tex_coords[7] = 0.0;

    int width = 1024;
    int height = 768;

    glBegin(GL_QUADS);
    glTexCoord2f(tex_coords[0], tex_coords[1]);
    glVertex2f(0, 0);
    glTexCoord2f(tex_coords[2], tex_coords[3]);
    glVertex2f(width, 0);
    glTexCoord2f(tex_coords[4], tex_coords[5]);
    glVertex2f(width, height);
    glTexCoord2f(tex_coords[6], tex_coords[7]);
    glVertex2f(0, height);
    glEnd();

    glfwSwapBuffers(window);
}

int main(int argc, char **argv)
{
    init_time();
    enet_initialize();

    host = enet_host_create(NULL, 1, CHANNEL_COUNT, 0, 0);
    enet_host_compress_with_range_coder(host);

    if (host == NULL) {
        std::cerr << "Could not initialize host" << std::endl;
        return 0;
    }

    ENetAddress address;
    enet_address_set_host(&address, "192.168.2.4");
    address.port = 7171;
    peer = enet_host_connect(host, &address, CHANNEL_COUNT, 0);

    // set up window

    glfwSetErrorCallback(_error_callback);

    if(!glfwInit()) {
        return 0;
    }

    window = glfwCreateWindow(1024, 768, "osxrd", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (false) // vsync
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

/*    glfwSetMouseButtonCallback(window, _mouse_callback);
    glfwSetCursorPosCallback(window, _mouse_move_callback);
    glfwSetKeyCallback(window, _button_callback);
    glfwSetCharCallback(window, _char_callback);
    glfwSetScrollCallback(window, _mouse_scroll_callback);
    glfwSetWindowSizeCallback(window, _resize_callback);*/

    // OpenGL init
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glGenTextures(1, &screen_tex);
    screen_data = new char[1024 * 768 * 3];

    // start new thread for networking
    tthread::thread network_thread(network_loop, NULL);

    std::cout << "Running osxrd client" << std::endl;

    double t = get_time();

    while (true) {
        glfwPollEvents();
        if (glfwWindowShouldClose(window))
            break;
        draw();
        std::cout << "FPS: " << (1 / (get_time() - t)) << std::endl;
        t = get_time();
    }

    application_exit = true;
    network_thread.join();
    enet_host_destroy(host);
}