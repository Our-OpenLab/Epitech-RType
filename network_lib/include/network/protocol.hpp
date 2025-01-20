#ifndef NETWORK_PROTOCOL_HPP_
#define NETWORK_PROTOCOL_HPP_

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace network {

/**
 * @brief Header structure for packets.
 *
 * @tparam PacketType Enum type representing the packet type.
 */
template <typename PacketType>
struct Header {
  PacketType type{};       ///< The type of the packet.
  uint32_t size{};         ///< The size of the packet body in bytes.
};

/**
 * @brief Generic Packet structure.
 *
 * Combines a header and a body to represent a packet.
 *
 * @tparam PacketType Enum type representing the packet type.
 */
template <typename PacketType>
struct Packet {
  Header<PacketType> header{};  ///< Header of the packet.
  std::vector<uint8_t> body;    ///< Body of the packet.

  /**
   * @brief Returns the total size of the packet (header + body).
   *
   * @return Total size in bytes.
   */
  [[nodiscard]] size_t Size() const { return sizeof(header) + body.size(); }

  /**
   * @brief Pushes data to the packet body.
   *
   * @tparam T Type of data to push. Must be trivially copyable.
   * @param data Data to push into the body.
   */
  template <typename T>
  void Push(const T& data) {
    static_assert(std::is_trivially_copyable_v<T>, "DataType must be trivially copyable");
    const size_t current_size = body.size();
    body.resize(current_size + sizeof(T));
    std::memcpy(body.data() + current_size, &data, sizeof(T));
    header.size += sizeof(T);
  }

  /**
   * @brief Extracts data from the packet body.
   *
   * @tparam T Type of data to extract. Must be trivially copyable.
   * @return Extracted data.
   * @throws std::out_of_range If the body size is insufficient.
   */
  template <typename T>
  T Extract() {
    static_assert(std::is_trivially_copyable_v<T>, "DataType must be trivially copyable");
    if (body.size() < sizeof(T)) {
      throw std::out_of_range("Extract: Not enough data in the body");
    }
    const size_t remaining_size = body.size() - sizeof(T);
    T data;
    std::memcpy(&data, body.data() + remaining_size, sizeof(T));
    body.resize(remaining_size);
    header.size = remaining_size;
    return data;
  }

  /**
   * @brief Converts the packet into a byte vector.
   *
   * @return Byte vector representation of the packet.
   */
  [[nodiscard]] std::vector<uint8_t> Data() const {
    std::vector<uint8_t> packet_data(Size());
    auto* dest = packet_data.data();
    std::memcpy(dest, &header, sizeof(header));
    if (!body.empty()) {
      std::memcpy(dest + sizeof(header), body.data(), body.size());
    }
    return packet_data;
  }
};

/**
 * @brief Output stream operator for Packet.
 *
 * @tparam PacketType Enum type representing the packet type.
 * @param os Output stream.
 * @param packet Packet to output.
 * @return Reference to the output stream.
 */
template <typename PacketType>
std::ostream& operator<<(std::ostream& os, const Packet<PacketType>& packet) {
  os << "Type: " << static_cast<uint32_t>(packet.header.type)
     << " Size: " << packet.header.size;
  return os;
}

template <typename PacketType>
class TcpServerConnection;

/**
 * @brief Represents a TCP-owned packet.
 *
 * @tparam PacketType Enum type representing the packet type.
 */
template <typename PacketType>
class OwnedPacketTCP {
 public:
  std::shared_ptr<TcpServerConnection<PacketType>> connection;  ///< TCP connection.
  Packet<PacketType> packet;  ///< Packet data.

  OwnedPacketTCP(const std::shared_ptr<TcpServerConnection<PacketType>>& connection,
                 Packet<PacketType> packet)
      : connection(connection), packet(std::move(packet)) {}

  friend std::ostream& operator<<(std::ostream& os, const OwnedPacketTCP& owned_packet) {
    os << owned_packet.packet;
    return os;
  }
};

/**
 * @brief Represents a UDP-owned packet.
 *
 * @tparam PacketType Enum type representing the packet type.
 */
template <typename PacketType>
class OwnedPacketUDP {
 public:
  asio::ip::udp::endpoint endpoint;  ///< UDP endpoint.
  Packet<PacketType> packet;         ///< Packet data.

  OwnedPacketUDP(asio::ip::udp::endpoint endpoint, Packet<PacketType> packet)
      : endpoint(std::move(endpoint)), packet(std::move(packet)) {}

  friend std::ostream& operator<<(std::ostream& os, const OwnedPacketUDP& owned_packet) {
    os << "From: " << owned_packet.endpoint.address().to_string() << ":"
       << owned_packet.endpoint.port() << " - " << owned_packet.packet;
    return os;
  }
};

/**
 * @brief Utility class for creating and extracting packets.
 *
 * @tparam PacketType Enum type representing the packet type.
 */
template <typename PacketType>
class PacketFactory {
 public:
  /**
   * @brief Creates a packet with a single data element.
   *
   * @tparam T Type of the data. Must be trivially copyable.
   * @param type Packet type.
   * @param data Data to include in the packet.
   * @return The created packet.
   */
  template <typename T>
  static Packet<PacketType> CreatePacket(PacketType type, const T& data) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    const auto raw_data = reinterpret_cast<const uint8_t*>(&data);

    return Packet<PacketType>{
        .header = {.type = type, .size = sizeof(T)},
        .body = std::vector<uint8_t>(raw_data, raw_data + sizeof(T))};
  }

  /**
   * @brief Creates a packet from a span of data.
   *
   * @tparam T Type of the data in the span. Must be trivially copyable.
   * @param type Packet type.
   * @param data Data span to include in the packet.
   * @return The created packet.
   */
  template <typename T>
  static Packet<PacketType> CreatePacket(PacketType type, std::span<T> data) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    const size_t total_size = data.size_bytes();

    return Packet<PacketType>{
        .header = {.type = type, .size = static_cast<uint32_t>(total_size)},
        .body = std::vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(data.data()),
            reinterpret_cast<const uint8_t*>(data.data()) + total_size)};
  }

  /**
 * @brief Extracts a single data element from a packet.
 *
 * @tparam T Type of the data. Must be trivially copyable.
 * @param packet The packet to extract data from.
 * @return An optional containing the extracted data, or std::nullopt if the size is invalid.
 */
  template <typename T>
  static std::optional<T> ExtractData(const Packet<PacketType>& packet) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    if (packet.body.size() != sizeof(T)) {
      return std::nullopt;
    }

    return *reinterpret_cast<const T*>(packet.body.data());
  }


  /**
 * @brief Extracts an array of data elements from a packet.
 *
 * @tparam T Type of the data. Must be trivially copyable.
 * @param packet The packet to extract data from.
 * @return An optional vector containing the extracted data, or std::nullopt if the size is invalid.
 */
  template <typename T>
  static std::optional<std::vector<T>> ExtractDataArray(const Packet<PacketType>& packet) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    if (packet.body.size() % sizeof(T) != 0) {
      return std::nullopt;
    }

    const size_t count = packet.body.size() / sizeof(T);
    const auto* raw_data = reinterpret_cast<const T*>(packet.body.data());

    return std::vector<T>(raw_data, raw_data + count);
  }

};

}  // namespace network

#endif  // NETWORK_PROTOCOL_HPP_
