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

#include "usock.h"
#include "usock_exception.h"

Socket::Socket (int domain, int type) throw()  {
	this->domain = domain;
	this->type = type;
	
	if ((sd = socket(domain, type, 0)) < 0)
		throw SocketException("socket error");
}

Socket::Socket (const string& host, u_int16_t port)  throw()  {
	this->domain = AF_INET;
	this->type = SOCK_STREAM;
	
	if ((sd = socket(domain, type, 0)) < 0)
		throw SocketException("socket error");

	connect(host,port);
}

Socket::Socket (int sd, int domain, int type) throw()  {
	this->sd = sd;
	this->domain = domain;
	this->type = type;
}

Socket::~Socket()  { ::close(sd); }

string Socket::getHostByName (const string& name) throw()  {
	char* addr = new char[INET6_ADDRSTRLEN];
	struct hostent *host;

	if (! (host = gethostbyname(name.c_str())) )
		throw SocketException("gethostbyname exception");

	snprintf (addr, INET6_ADDRSTRLEN, "%u.%u.%u.%u",
			(unsigned char) host->h_addr[0],
			(unsigned char) host->h_addr[1],
			(unsigned char) host->h_addr[2],
			(unsigned char) host->h_addr[3]);

	return string(addr);
}

void Socket::connect (const string& host, u_int16_t port) throw()  {
	string addr;
	struct sockaddr_in sin;

	try  { addr = getHostByName(host); }
	catch (exception e)  { return; }

	sin.sin_family = domain;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(addr.c_str());

	if (::connect(sd, (struct sockaddr*) &sin, sizeof(struct sockaddr))<0)
		throw SocketException("connect exception");
}

void Socket::send (const string& buf) throw()  {
	if (::send(sd, buf.c_str(), buf.length(), 0) < 0)
		throw SocketException("send exception");
}

void Socket::operator<< (const string& buf) throw()  { send(buf); }

void Socket::operator<< (const char& buf) throw()  {
	stringstream ss(stringstream::in | stringstream::out);
	ss << buf;
	send(ss.str());
}

void Socket::operator<< (const int& buf) throw()  {
	stringstream ss(stringstream::in | stringstream::out);
	ss << buf;
	send(ss.str());
}

void Socket::operator<< (const float& buf) throw()  {
	stringstream ss(stringstream::in | stringstream::out);
	ss << buf;
	send(ss.str());
}

void Socket::operator<< (const double& buf) throw()  {
	stringstream ss(stringstream::in | stringstream::out);
	ss << buf;
	send(ss.str());
}

string Socket::recv (u_int32_t nbytes) throw()  {
	char* buf = new char[nbytes];
	u_int32_t n;

	if ((n = ::recv(sd, buf, nbytes, 0)) < 0)
		throw SocketException("recv exception");

	if (!n) return string("");
	return string(buf);
}

void Socket::recv (void* buf, u_int32_t size) throw()  {
	u_int32_t n;

	if ((n = ::recv(sd, buf, size, 0)) < 0)
		throw SocketException("recv exception");

	if (!n) buf = NULL;
}

void Socket::getSockOpt (int level, int optname, void* optval, socklen_t* optlen) throw()  {
	if (::getsockopt(sd, level, optname, optval, optlen) < 0)
		throw SocketException("getsockopt error");
}

void Socket::setSockOpt (int level, int optname, void* optval, socklen_t optlen) throw()  {
	if (::setsockopt(sd, level, optname, optval, optlen) < 0)
		throw SocketException("setsockopt error");
}

void Socket::operator>> (string& buf) throw()  {
	buf = recv();
}

string Socket::readline() throw()  {
	string line;
	bool isEOF = false;
	bool isEOL = false;
	u_int32_t size = 0;

	while (!isEOL && !isEOF)  {
		char buf;
		u_int32_t n = ::recv(sd, &buf, 1, 0);
		
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

void Socket::close() throw()  {
	if (::close(sd)<0)
		throw SocketException("close exception");
}

string Socket::remoteAddr() throw()  {
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

string Socket::localAddr() throw()  {
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

u_int16_t Socket::remotePort() throw()  {
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr_in);

	if (getpeername(sd, (struct sockaddr*) &sock, &len) < 0)  {
		if (errno == ENOTCONN) return 0;
		else throw SocketException("getpeername exception");
	}

	return htons(sock.sin_port);
}

u_int16_t Socket::localPort() throw()  {
	struct sockaddr_in sock;
	socklen_t len = sizeof(struct sockaddr_in);

	if (getsockname(sd, (struct sockaddr*) &sock, &len) < 0)  {
		if (errno == ENOTCONN) return 0;
		else throw SocketException("getsockname exception");
	}

	return htons(sock.sin_port);
}

void Socket::setBlocking (bool f) throw()  {
	u_int32_t flags;
	
	if ((flags = fcntl(sd, F_GETFL, 0)) < 0)
		throw SocketException("fcntl exception");

	if (fcntl(sd, F_SETFL, (f ? flags & ~O_NONBLOCK : flags & O_NONBLOCK)) < 0)
		throw SocketException("fcntl exception");
}

bool Socket::isBlocking() throw()  {
	u_int32_t flags;

	if ((flags = fcntl(sd, F_GETFL, 0)) < 0)
		throw SocketException("fcntl exception");

	return ((flags & O_NONBLOCK) ? true : false);
}

string Socket::ntoa (in_addr_t addr) throw()  {
	in_addr a;
	a.s_addr = addr;
	char* str = inet_ntoa(a);

	if (!str)
		throw SocketException("inet_ntoa() exception");

	return string(str);
}

