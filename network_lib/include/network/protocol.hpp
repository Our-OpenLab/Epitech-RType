#ifndef PROTOCOL_HPP_
#define PROTOCOL_HPP_

#include <cstdint>
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>
#include <span>

namespace network {
template <typename PacketType>
struct Header {
  PacketType type{};
  std::uint32_t size{};
};

template <typename PacketType>
struct Packet {
  Header<PacketType> header{};
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

template <typename PacketType>
inline std::ostream& operator<<(std::ostream& os, const Packet<PacketType>& packet) {
  os << "Type: " << static_cast<uint32_t>(packet.header.type)
     << " Size: " << packet.header.size;
  return os;
}

template <typename PacketType>
class ServerConnection;

template <typename PacketType>
class OwnedPacket {
  public:
    std::shared_ptr<ServerConnection<PacketType>> connection;
    Packet<PacketType> packet;

  OwnedPacket(const std::shared_ptr<ServerConnection<PacketType>>& connection, Packet<PacketType> packet)
      : connection(connection), packet(std::move(packet)) {}

  friend std::ostream& operator<<(std::ostream& os, const OwnedPacket& packet) {
    os << packet.packet;
    return os;
  }
};

template <typename PacketType>
class PacketFactory {
public:
  template <typename T>
  static Packet<PacketType> create_packet(PacketType type, const T& data) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    const auto raw_data = reinterpret_cast<const uint8_t*>(&data);

    Packet<PacketType> packet {
      .header = {
        .type = type,
        .size = sizeof(T),
    },
    .body = std::vector<uint8_t>(raw_data, raw_data + sizeof(T))
};

    return packet;
  }

  template <typename T>
  static Packet<PacketType> create_packet(PacketType type, std::span<T> data) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    const size_t total_size = data.size_bytes();

    Packet<PacketType> packet{
      .header = {
        .type = type,
        .size = static_cast<uint32_t>(total_size),
    },
    .body = std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(data.data()),
                                 reinterpret_cast<const uint8_t*>(data.data()) + total_size)};

    return packet;
  }

  template <typename T>
  static T extract_data(const Packet<PacketType>& packet) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    if (packet.body.size() != sizeof(T)) {
      throw std::out_of_range("Packet body size does not match the expected structure size");
    }

    return *reinterpret_cast<const T*>(packet.body.data());
  }

  template <typename T>
  static std::vector<T> extract_data_array(const Packet<PacketType>& packet) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");

    if (packet.body.size() % sizeof(T) != 0) {
      throw std::out_of_range("Packet body size is not a multiple of the expected structure size");
    }

    const size_t count = packet.body.size() / sizeof(T);
    const auto* raw_data = reinterpret_cast<const T*>(packet.body.data());
    return std::vector<T>(raw_data, raw_data + count);
  }
};

}  // namespace network

#endif  // PROTOCOL_HPP_
