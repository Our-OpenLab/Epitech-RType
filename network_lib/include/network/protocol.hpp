#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstdint>
#include <iostream>
#include <vector>

namespace network {
enum class PacketType : std::uint32_t {
  ServerAccept,
  ServerDeny,
  Ping,
  Pong,
  Disconnect,
  PlayerMove,
  ChatMessage,
  MaxTypes
};

struct Header {
  PacketType type{};
  std::uint32_t size;
};

struct Packet {
  Header header{};
  std::vector<std::uint8_t> body;

  [[nodiscard]] std::size_t size() const {
    return sizeof(header) + body.size();
  }

  template <typename T>
  void push(const T& data) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "DataType must be trivially copyable");

    const size_t currentSize = body.size();

    body.resize(currentSize + sizeof(T));

    std::memcpy(body.data() + currentSize, &data, sizeof(T));

    header.size += sizeof(T);
  }

  template <typename T>
  T extract() {
    static_assert(std::is_trivially_copyable_v<T>,
                  "DataType must be trivially copyable");

    if (body.size() < sizeof(T)) {
      throw std::out_of_range("extractFromBody: Not enough data in body");
    }

    const size_t remainingSize = body.size() - sizeof(T);

    T data;
    std::memcpy(&data, body.data() + remainingSize, sizeof(T));

    body.resize(remainingSize);

    header.size = remainingSize;

    return data;
  }
};

inline std::ostream& operator<<(std::ostream& os, const Packet& packet) {
  os << "Type: " << static_cast<uint32_t>(packet.header.type)
     << "Size: " << packet.header.size;
  return os;
}

class ServerConnection;

class OwnedPacket {
  public:
    std::shared_ptr<ServerConnection> connection;
    Packet packet;

  OwnedPacket(const std::shared_ptr<ServerConnection>& connection, Packet packet)
      : connection(connection), packet(std::move(packet)) {}

  friend std::ostream& operator<<(std::ostream& os, const OwnedPacket& packet) {
    os << packet.packet;
    return os;
  }
};
}  // namespace network

#endif  // PROTOCOL_HPP_
