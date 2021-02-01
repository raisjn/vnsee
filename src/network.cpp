#include "network.hpp"
#include <array>
#include <arpa/inet.h>
#include <cerrno>
#include <iterator>
#include <netinet/in.h>
#include <cstdint>
#include <sys/socket.h>
#include <fstream>
#include <string>
#include <system_error>

constexpr auto dhcp_timestamp_size = 8;
constexpr auto dhcp_mac_size = 6;
constexpr auto dhcp_hostname_size = 20;

/**
 * A DHCP client lease from udhcpd.
 * See <udhcp/dhcpd.h>
 */
struct dhcp_lease {
    /** Unix time when lease expires in network order. */
    uint32_t expires;

    /** Client IP in network order. */
    uint32_t lease_nip;

    /** Client MAC. */
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::array<uint8_t, dhcp_mac_size> lease_mac;

    /** Client hostname. */
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::array<char, dhcp_hostname_size> hostname;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::array<uint8_t, 2> pad;
} __attribute__((__packed__));

constexpr auto leases_file_path = "/var/lib/misc/udhcpd.leases";

namespace network
{

auto ip_to_string(const in_addr& ip) -> std::string
{
    std::array<char, INET6_ADDRSTRLEN> str_ip_buf{};

    if (inet_ntop(
        AF_INET,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const void*>(&ip),
        str_ip_buf.data(),
        str_ip_buf.size()
    ) == nullptr)
    {
        throw std::system_error{
            errno,
            std::generic_category(),
            "(network::ip_to_string) Decode client IP"
        };
    }

    return std::string(begin(str_ip_buf), end(str_ip_buf));
}

auto get_usb_hosts() -> std::vector<in_addr>
{
    std::ifstream leases_file{leases_file_path, std::ios::binary};
    leases_file.ignore(dhcp_timestamp_size);

    std::vector<in_addr> results;
    dhcp_lease lease{};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    while (leases_file.read(reinterpret_cast<char*>(&lease), sizeof(lease)))
    {
        in_addr addr{};
        addr.s_addr = lease.lease_nip;
        results.push_back(addr);
    }

    return results;
}

auto tcp_can_connect(in_addr ip, short port) -> bool
{
    auto sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (sock_fd == -1)
    {
        throw std::system_error{
            errno,
            std::generic_category(),
            "(network::tcp_can_connect) Create socket object"
        };
    }

    sockaddr_in address{};
    address.sin_addr = ip;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    return connect(
        sock_fd,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    ) == 0;
}

} // namespace network
