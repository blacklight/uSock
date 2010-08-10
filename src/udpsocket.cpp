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

#include <arpa/inet.h>
#include "usock.h"
#include "usock_exception.h"

#include "raii.hh"

using std::string;
using namespace usock;

UDPSocket::UDPSocket (int domain) throw() : BaseSocket(domain, SOCK_DGRAM, IPPROTO_UDP)  {}

void UDPSocket::send (const string& buf, const string& host, u_int16_t port) throw()  {
	string addr = getHostByName(host);
	struct sockaddr_in sock;

	sock.sin_family = domain;
	sock.sin_port = htons(port);
	sock.sin_addr.s_addr = inet_addr(addr.c_str());

	if (sendto(sd, buf.c_str(), buf.length(), 0, (struct sockaddr*) &sock, sizeof(struct sockaddr)) < 0)
		throw SocketException("send exception");
}

void UDPSocket::send (const void* buf, u_int32_t size, const string& host, u_int16_t port) throw()  {
	string addr = getHostByName(host);
	struct sockaddr_in sock;

	sock.sin_family = domain;
	sock.sin_port = htons(port);
	sock.sin_addr.s_addr = inet_addr(addr.c_str());

	if (sendto(sd, buf, size, 0, (struct sockaddr*) &sock, sizeof(struct sockaddr)) < 0)
		throw SocketException("send exception");
}

void UDPSocket::recv (void* buf, u_int32_t size, const string& host, u_int16_t port) throw()  {
	string addr;
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr);

	if (!host.empty())
		addr = getHostByName(host);
	else
		addr = "";

	if (port)  {
		sock.sin_family = domain;
		sock.sin_port = htons(port);
		sock.sin_addr.s_addr = (!addr.empty()) ? inet_addr(addr.c_str()) : INADDR_ANY;
	}

	if (recvfrom(sd, buf, size, 0, (struct sockaddr*) &sock, &len) < 0)
		throw SocketException("recv exception");
}

string UDPSocket::recv (const string& host, u_int16_t port) throw()  {
	char* buf = new char[BUFRECV_SIZE];
	raii_array<char> buf_holder(buf);
	u_int32_t n;

	struct sockaddr_in sock;
	string addr;
	socklen_t len = sizeof(struct sockaddr);

	if (!host.empty())
		addr = getHostByName(host);
	else
		addr = "";

	if (port)  {
		sock.sin_family = domain;
		sock.sin_port = htons(port);
		sock.sin_addr.s_addr = (!addr.empty()) ? inet_addr(addr.c_str()) : INADDR_ANY;
	}

	if ((n = recvfrom(sd, buf, BUFRECV_SIZE, 0, (struct sockaddr*) &sock, &len)) < 0)
		throw SocketException("recv exception");

	if (!n) return string();
	return string(buf);
}

string UDPSocket::readline(const string& host, u_int16_t port) throw()  {
	string line, addr;
	bool isEOF = false;
	bool isEOL = false;

	struct sockaddr_in sock;

	if (!host.empty())
		addr = getHostByName(host);

	socklen_t len = sizeof(struct sockaddr);

	if (port)  {
		sock.sin_family = domain;
		sock.sin_port = htons(port);
		sock.sin_addr.s_addr = (!addr.empty()) ? inet_addr(addr.c_str()) : INADDR_ANY;
	}

	while (!isEOL && !isEOF)  {
		char buf;
		u_int32_t n = recvfrom(sd, &buf, 1, 0, (struct sockaddr*) &sock, &len);
		
		if (n < 1)
			isEOF = true;
		else if (buf == '\n')
			isEOL = true;
		else  {
			if (buf != '\r')
				line += buf;
		}
	}

	if (line.length() < 1)  {
		if (isEOF) line = "";
		else line = "\r";
	}

	return string(line);
}

void UDPSocket::bind (u_int16_t port) throw()  {
	struct sockaddr_in sock;

	sock.sin_family = domain;
	sock.sin_port = htons(port);
	sock.sin_addr.s_addr = INADDR_ANY;

	if (::bind(sd, (struct sockaddr*) &sock, sizeof(struct sockaddr)) < 0)
		throw SocketException("bind exception");
}

