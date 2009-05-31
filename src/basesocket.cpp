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

#include <sstream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>

#include "usock.h"
#include "usock_exception.h"

using std::string;
using std::stringstream;
using namespace usock;

BaseSocket::BaseSocket (int domain, int type, int protocol, double timeout) throw()  {
	this->domain = domain;
	this->type = type;
	this->protocol = protocol;
	this->timeout = timeout;
	
	if ((sd = socket(domain, type, protocol)) < 0)
		throw SocketException("socket error");
}

BaseSocket::~BaseSocket()  { close(); }

string BaseSocket::getHostByName (const string& name) throw()  {
	char* addr = new char[INET6_ADDRSTRLEN];
	struct hostent *host;

	if (! (host = gethostbyname(name.c_str())) )
		return string("");

	snprintf (addr, INET6_ADDRSTRLEN, "%u.%u.%u.%u",
			(unsigned char) host->h_addr[0],
			(unsigned char) host->h_addr[1],
			(unsigned char) host->h_addr[2],
			(unsigned char) host->h_addr[3]);

	return string(addr);
}

string BaseSocket::getHostByAddr (const string& addr) throw()  {
	unsigned int tmp[4];
	struct hostent *host;
	in_addr_t inaddr = inet_addr(addr.c_str());

	if (sscanf(addr.c_str(), "%u.%u.%u.%u", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) != 4)
		throw SocketException("invalid IPv4 address");

	for (int i=0; i<4; i++)
		if (tmp[i] > 255)
			throw SocketException("invalid IPv4 address");

	if (! (host = gethostbyaddr(&inaddr, 4, AF_INET)))
		return string("");

	if (host->h_name)
		return string(host->h_name);
	else
		return string("");
}

void BaseSocket::getSockOpt (int level, int optname, void* optval, socklen_t* optlen) throw()  {
	if (::getsockopt(sd, level, optname, optval, optlen) < 0)
		throw SocketException("getsockopt error");
}

void BaseSocket::setSockOpt (int level, int optname, void* optval, socklen_t optlen) throw()  {
	if (::setsockopt(sd, level, optname, optval, optlen) < 0)
		throw SocketException("setsockopt error");
}

void BaseSocket::close()  { ::close(sd); }

string BaseSocket::remoteAddr() throw()  {
	struct in_addr addr;
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr_in);

	if (getpeername(sd, (struct sockaddr*) &sock, &len) < 0)  {
		if (errno == ENOTCONN) return string("");
		else throw SocketException("getpeername exception");
	}

	addr.s_addr = sock.sin_addr.s_addr;
	return string(inet_ntoa(addr));
}

string BaseSocket::localAddr() throw()  {
	struct in_addr addr;
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr_in);

	if (getsockname(sd, (struct sockaddr*) &sock, &len) < 0)  {
		if (errno == ENOTCONN) return string("");
		else throw SocketException("getsockname exception");
	}

	addr.s_addr = sock.sin_addr.s_addr;
	return string(inet_ntoa(addr));
}

u_int16_t BaseSocket::remotePort() throw()  {
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr_in);

	if (getpeername(sd, (struct sockaddr*) &sock, &len) < 0)  {
		if (errno == ENOTCONN) return 0;
		else throw SocketException("getpeername exception");
	}

	return htons(sock.sin_port);
}

u_int16_t BaseSocket::localPort() throw()  {
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr_in);

	if (getsockname(sd, (struct sockaddr*) &sock, &len) < 0)  {
		if (errno == ENOTCONN) return 0;
		else throw SocketException("getsockname exception");
	}

	return htons(sock.sin_port);
}

void BaseSocket::setBlocking (bool f) throw()  {
	u_int32_t flags;
	
	if ((flags = fcntl(sd, F_GETFL)) < 0)
		throw SocketException("fcntl exception");

	if (f)  {
		if (fcntl(sd, F_SETFL, flags & ~O_NONBLOCK) < 0)
			throw SocketException("fcntl exception");
	} else {
		if (fcntl(sd, F_SETFL, flags | O_NONBLOCK) < 0)
			throw SocketException("fcntl exception");
	}
}

bool BaseSocket::isBlocking() throw()  {
	u_int32_t flags;

	if ((flags = fcntl(sd, F_GETFL, 0)) < 0)
		throw SocketException("fcntl exception");

	return ((flags & O_NONBLOCK) ? true : false);
}

string BaseSocket::ntoa (in_addr_t addr) throw()  {
	in_addr a;
	a.s_addr = addr;
	char* str = inet_ntoa(a);

	if (!str)
		throw SocketException("inet_ntoa exception");

	return string(str);
}

void BaseSocket::setTimeout (double timeout) throw()  { this->timeout = timeout; }

