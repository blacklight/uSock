/**
 * ======================================
 *  _ _ _                          _    
 * | (_) |                        | |   
 * | |_| |__  _   _ ___  ___   ___| | __
 * | | | '_ \| | | / __|/ _ \ / __| |/ /
 * | | | |_) | |_| \__ \ (_) | (__|   < 
 * |_|_|_.__/ \__,_|___/\___/ \___|_|\_\
 *
 * ======================================
 *
 * The files in this directory and elsewhere which refer to this LICENCE
 * file are part of uSock, the library for the high-level management of
 * network sockets.
 *
 * Copyright (C) 2009 by BlackLight, <blacklight@autistici.org>
 * Web: http://0x00.ath.cx
 *
 * uSock is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 or (at your option) any later 
 * version.
 *
 * uSock is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with uSock; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * As a special exception, if other files instantiate templates or use
 * macros or inline functions from these files, or you compile these
 * files and link them with other works to produce a work based on these
 * files, these files do not by themselves cause the resulting work to be
 * covered by the GNU General Public License. However the source code for
 * these files must still be made available in accordance with section (3)
 * of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 */

#ifndef __cplusplus
#error "usock library can only be used in C++ code"
#endif

#ifndef __USOCK_H
#define __USOCK_H

#include <netinet/in.h>
#include <netpacket/packet.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <string>

#define	BUFRECV_SIZE	1024
#define	DEFAULT_MAXCON	10

#ifndef	__FAVOR_BSD
#define TH_FIN	 0x01
#define TH_SYN	 0x02
#define TH_RST	 0x04
#define TH_PUSH 0x08
#define TH_ACK	 0x10
#define TH_URG	 0x20
#endif

struct icmp_hdr {
	u_int8_t		type;
	u_int8_t		code;
	u_int16_t		checksum;
	u_int16_t		id;
	u_int16_t		sequence;
};

struct pseudohdr  {
	u_int32_t src;
	u_int32_t dst;
	u_int8_t  padd;
	u_int8_t  proto;
	u_int16_t len;
};

/**
 * @class Socket
 * @brief Base socket class for building, by default, TCP sockets
 * @author BlackLight
 */
class Socket  {

protected:
	int sd;
	int domain;
	int type;

public:
	/**
	 * @brief Constructor for the Socket class
	 * @param domain Socket domain (default = AF_INET)
	 * @param type Socket type (default = SOCK_STREAM)
	 */
	Socket (int domain = AF_INET, int type = SOCK_STREAM) throw();

	/**
	 * @brief Constructor for the Socket class, builds a TCP socket and connects onto it
	 * @param host Host name/address
	 * @param port Remote port
	 */
	Socket (const std::string& host, u_int16_t port) throw();

	/**
	 * @brief Constructor for the Socket class using an already existent socket descriptor
	 * @param sd Socket descriptor
	 * @param domain Socket domain
	 * @param type Socket type
	 */
	Socket (int sd, int domain, int type) throw();

	/**
	 * @brief Destroyer for the Socket class (it just destroyes our socket descriptor)
	 */
	~Socket();

	/**
	 * @brief Create a TCP connection on the socket
	 * @param host Host name/address
	 * @param port Remote port
	 */
	void connect (const std::string& host, u_int16_t port) throw();

	/**
	 * @brief Close the socket descriptor
	 */
	void close() throw();

	/**
	 * @brief Send a string onto a TCP socket
	 * @param buf String to send
	 */
	void send (const std::string& buf) throw();

	/**
	 * @brief Send a binary buffer onto a TCP socket
	 * @param buf Buffer to be sent
	 * @param size buf's size
	 */
	void send (const void* buf, u_int32_t size) throw();

	/**
	 * @brief Overloaded operator to send a buffer onto a TCP socket
	 * @param buf Stuff to be sent
	 */
	void operator<< (const char& buf) throw();
	void operator<< (const int& buf) throw();
	void operator<< (const float& buf) throw();
	void operator<< (const double& buf) throw();
	void operator<< (const std::string& buf) throw();

	/**
	 * @brief Overloaded operator to receive a buffer from a socket (default size: BUFRECV_SIZE)
	 * @param buf String object where we're going to put our received stuff
	 */
	void operator>> (std::string& buf) throw();

	/**
	 * @brief Wrap method around gethostbyname() function
	 * @param name Host name
	 * @return IP address of our host name, if found, NULL otherwise
	 */
	std::string getHostByName (const std::string& name) throw();

	/**
	 * @brief Receive a buffer from a TCP socket
	 * @param nbytes Number of bytes to be read
	 * @return A string containing the bytes read from the socket
	 */
	std::string recv (u_int32_t nbytes = BUFRECV_SIZE) throw();

	/**
	 * @brief Receive a binary buffer from a TCP socket
	 * @param buf Buffer where the read stuff will be placed
	 * @param size Number of bytes to be read
	 */
	void recv (void* buf, u_int32_t size) throw();

	/**
	 * @brief Set or unset the blocking flag on a socket
	 * @param f Boolean flag (true/false)
	 */
	void setBlocking(bool f = true) throw();

	/**
	 * @brief Checks whether a socket is blocking
	 * @return true if the socket is blocking, false otherwise
	 */
	bool isBlocking() throw();

	/**
	 * @brief Read an ASCII line from the socket
	 * @return String containing the read line
	 */
	std::string readline() throw();

	/**
	 * @brief Return the local address assigned to a socket descriptor
	 */
	std::string localAddr() throw();

	/**
	 * @brief Return the remote address to which the socket is linked
	 */
	std::string remoteAddr() throw();

	/**
	 * @brief Return the local port
	 */
	u_int16_t localPort() throw();

	/**
	 * @brief Return the remote port
	 */
	u_int16_t remotePort() throw();

	/**
	 * @brief Wrap around getsockopt() function
	 */
	void getSockOpt (int level, int optname, void* optval, socklen_t* optlen) throw();

	/**
	 * @brief Wrap around setsockopt() function
	 */
	void setSockOpt (int level, int optname, void* optval, socklen_t optlen) throw();

	/**
	 * @brief Wrap around inet_ntoa() function
	 */
	std::string ntoa (in_addr_t addr) throw();
	
};

/**
 * @class ServerSocket
 * @brief Class for managing server sockets
 * @author BlackLight
 */
class ServerSocket : public Socket  {

private:
	u_int32_t maxconn;
	u_int32_t sock_index;

public:
	/**
	 * @brief ServerSocket constructor
	 * @param domain Socket domain
	 * @param type Socket type
	 * @param m Maximum number of allowed connections
	 */
	ServerSocket (int domain, int type, int m) throw();

	/**
	 * @brief ServerSocket high-level constructor
	 * @param port Port the server will listen onto
	 * @param m Maximum number of allowed connections
	 */
	ServerSocket (u_int16_t port, u_int32_t m = DEFAULT_MAXCON) throw();

	/**
	 * @brief Wrap around accept() function
	 */
	Socket accept() throw();

	/**
	 * @brief Bind ServerSocket onto a port
	 * @param port Port
	 */
	void bind (u_int16_t port) throw();

	/**
	 * @brief Wrap around listen() function
	 */
	void listen() throw();
};

/**
 * @class UDPSocket
 * @brief Class for managing UDP sockets
 * @author BlackLight
 */
class UDPSocket : public Socket  {

public:
	/**
	 * @brief UDPSocket constructor
	 * @param type Socket type (default = AF_INET)
	 */
	UDPSocket (int type = AF_INET) throw();

	/**
	 * @brief Send a string onto an UDP socket
	 * @param buf String to be sent
	 * @param host Remote host name/address
	 * @param port Remote port
	 */
	void send (const std::string& buf, const std::string& host, u_int16_t port) throw();

	/**
	 * @brief Send a binary buffer onto an UDP socket
	 * @param buf Binary buffer to be sent
	 * @param size buf's size
	 * @param host Remote host name/address
	 * @param port Remote port
	 */
	void send (const void* buf, u_int32_t size, const std::string& host, u_int16_t port) throw();

	/**
	 * @brief Bind an UDP socket onto a port
	 * @param Port
	 */
	void bind (u_int16_t port) throw();
	
	/**
	 * @brief Receive a binary buffer from an UDP socket
	 * @param buf Buffer where we're going to place our data
	 * @param size Number of bytes to be read
	 * @param host Remote host name/address
	 * @param port Remote port
	 */
	void recv (void* buf, u_int32_t size, const std::string& host = "", u_int16_t port = 0) throw();

	/**
	 * @brief Receive an ASCII string from an UDP socket
	 * @param host Remote host name/address
	 * @param port Remote port
	 * @return String received
	 */
	std::string recv(const std::string& host = "", u_int16_t port = 0) throw();

	/**
	 * @brief Read an ASCII line from an UDP socket
	 * @param host Remote host name/address
	 * @param port Remote port
	 * @return String received
	 */
	std::string readline(const std::string& host = "", u_int16_t port = 0) throw();
};

/**
 * @class RawSocket
 * @brief Class for managing raw sockets
 * @author BlackLight
 */
class RawSocket : public Socket  {
	std::string iface;
	u_int8_t head[1024];
	u_int8_t *payload;
	int head_len, payload_len;
	bool is_IPv4, is_TCP, is_UDP, is_ICMPv4;

public:
	/**
	 * @brief RawSocket constructor
	 * @param i Interface on which we're going to bind our raw socket
	 */
	RawSocket (std::string i = "lo");

	/**
	 * @brief Get the IPv4 address associated to the network interface
	 * @return IPv4 address, if the interface is valid, up and running
	 */
	std::string getIPv4addr() throw();

	/**
	 * @brief Get the HW address associated to the network interface
	 * @return The HW/MAC address, if the interface is valid, up and running
	 */
	std::string getHWaddr() throw();

	/**
	 * @brief Compute the checksum of a buffer
	 * @param buf Buffer
	 * @param nwords Number of 16-bits words inside buf
	 * @return Checksum
	 */
	u_int16_t csum (u_int16_t *buf, int nwords);

	/**
	 * @brief Build an IPv4 header for the raw socket
	 * @param dst Destination hostname/address
	 * @param src Source hostname/address (default: IPv4 address associated to the network interface)
	 * @param proto Transport protocol (default: TCP)
	 * @param len Total length of the IPv4 datagram (default: header length + payload length)
	 * @param ttl Time To Live (default: 32)
	 * @param tos Type Of Service (default: 0)
	 * @param id Datagram ID (default: 0)
	 * @param frag Fragmentation flag (default: 0)
	 * @param sum IPv4 checksum (default: auto-computed)
	 */
	void buildIPv4 (std::string dst, std::string src = "", u_int8_t proto = IPPROTO_TCP, u_int16_t len = 0, u_int8_t ttl = 32,
			u_int8_t tos = 0, u_int16_t id = 0, u_int16_t frag = 0, u_int16_t sum = 0);

	/**
	 * @brief Build a TCP header for the raw socket
	 * @param sport Source port
	 * @param dport Destination port
	 * @param flags TCP flags
	 * @param seq Sequence number (default: randomly generated)
	 * @param ack ACK number (default: 0)
	 * @param window Window size (default: 16)
	 * @param urgent Urgent pointer (default: 0)
	 * @param sum TCP checksum (default: auto-computed)
	 */
	void buildTCP (u_int16_t sport, u_int16_t dport, u_int8_t flags, u_int32_t seq = 0, u_int32_t ack = 0,
			u_int16_t window = 0x1000, u_int16_t urgent = 0, u_int16_t sum = 0);

	/**
	 * @brief Build an UDP header for the raw socket
	 * @param sport Source port
	 * @param dport Destination port
	 * @param len UDP header + payload length (default: auto-computed)
	 * @param sum UDP checksum (default: auto-computed)
	 */
	void buildUDP (u_int16_t sport, u_int16_t dport, u_int16_t len = 0, u_int16_t sum = 0);

	/**
	 * @brief Build an ICMPv4 header for the raw socket
	 * @param type ICMP type
	 * @param id Packet ID (default: 0x100)
	 * @param seq Sequence number (default: 0x100)
	 * @param code ICMP code (default: 0)
	 * @param sum ICMP checksum (default: auto-computed)
	 */
	void buildICMPv4 (u_int8_t type, u_int16_t id = 0x100, u_int16_t seq = 0x100, u_int8_t code = 0, u_int16_t sum = 0);

	/**
	 * @brief Set a binary payload for the raw socket
	 * @param payload Binary payload
	 * @param length payload's length
	 */
	void setPayload (void* payload, int length);

	/**
	 * @brief Set a payload for the raw socket as a string
	 * @param payload Our payload
	 */
	void setPayload (std::string payload);

	/**
	 * @brief Write the raw packet onto the network interface
	 */
	void write() throw();

	/**
	 * @brief Read binary data from the raw socket
	 * @param len Number of bytes to be read
	 * @param host Host name/address we're going to receive our packet from (default: any)
	 */
	void* read (u_int32_t len, const std::string& host = "") throw();
};

#endif

