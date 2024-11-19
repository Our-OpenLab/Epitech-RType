#include <iostream>
#include <network/protocol.hpp>
#include <network/concurrent_queue.hpp>

int main() {

    std::cout << "Hello World!" << std::endl;

    network::Packet packet;

    packet.header.type = network::PacketType::Connect;

    int a = 1;
    bool b = false;
    float c = 3.14159f;

    packet.push(a);
    packet.push(b);
    packet.push(c);

    std::cout << packet.extract<float>() << std::endl;
    std::cout << packet.extract<bool>() << std::endl;
    std::cout << packet.extract<int>() << std::endl;

    return 0;
}
