#define MAX_CONNECTIONS 5
#define MAX_CHANNELS 5

#include <iostream>

int main(int argc, char **argv)
{
    enet_initialize();

    ENetAdress address;
    address.host = ENET_HOST_ANY;
    address.port = 7171;

    ENetHost * host = enet_host_create(&address, MAX_CONNECTIONS, MAX_CHANNELS,
                                       0, 0);

    if (host == NULL) {
        std::cerr << "Could not initialize host" << std::endl;
        return 0;
    }

    enet_host_compress_with_range_coder(host);

    while (true) {
        if (enet_host_service(host, &event, timeout) <= 0)
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

    enet_host_destroy(host);
}