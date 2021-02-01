#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <vector>

struct in_addr;

namespace network
{

/**
 * Convert an IP address from binary to text format.
 */
std::string ip_to_string(const in_addr& ip);

/**
 * Get IPs of all hosts currently connected over USB.
 *
 * @return List of IP addresses of connected hosts.
 */
std::vector<in_addr> get_usb_hosts();

/**
 * Test if a TCP connection can be successfully established
 * with a given address.
 *
 * @param ip IP address to try connecting to.
 * @param port Port to try connecting to.
 * @return True if and only if a connection can be established successfully.
 */
bool tcp_can_connect(in_addr ip, short port);

} // namespace network

#endif // NETWORK_HPP
