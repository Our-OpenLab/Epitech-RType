
#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstdint>
#include <vector>
#include <iostream>

namespace network {

    enum class PacketType : std::uint32_t {
        Connect = 0x01,
        Disconnect = 0x02,
        Heartbeat = 0x03,

        StartGame = 0x10,
        EndGame = 0x11,
        PauseGame = 0x12,

        PlayerState = 0x20,
        PlayerAction = 0x21,

        SpawnObject = 0x30,
        UpdateObject = 0x31,
        DestroyObject = 0x32,

        ChatMessage = 0x40,
        SystemMessage = 0x41
    };

    struct Header {
        PacketType type {};
        std::uint32_t size;
    };

    struct Packet {
        Header header {};
        std::vector<std::uint8_t> body;

        [[nodiscard]] std::size_t size() const { return sizeof(header) + body.size(); }

        template <typename T>
        void push(const T& data)
        {
            static_assert(std::is_trivially_copyable_v<T>, "DataType must be trivially copyable");

            const size_t currentSize = body.size();

            body.resize(currentSize + sizeof(T));

            std::memcpy(body.data() + currentSize, &data, sizeof(T));
        }

        template <typename T>
        T extract()
        {
            static_assert(std::is_trivially_copyable_v<T>, "DataType must be trivially copyable");

            if (body.size() < sizeof(T)) {
                throw std::out_of_range("extractFromBody: Not enough data in body");
            }

            const size_t remainingSize = body.size() - sizeof(T);

            T data;
            std::memcpy(&data, body.data() + remainingSize, sizeof(T));

            body.resize(remainingSize);

            return data;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Packet& packet)
    {
        os << "Type: " << static_cast<uint32_t>(packet.header.type)
           << "Size: " << packet.header.size;
        return os;
    }

    class Connection;

    class OwnedPacket
    {
        std::shared_ptr<Connection> connection;
        Packet packet;

        public:
            OwnedPacket(const std::shared_ptr<Connection>& connection, Packet packet): connection(connection), packet(std::move(packet)) {}

        friend std::ostream& operator<<(std::ostream& os, const OwnedPacket& packet)
        {
            os << packet.packet;
            return os;
        }
    };
}

#endif  // PROTOCOL_HPP_
