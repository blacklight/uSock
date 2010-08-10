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

#include "raii.hh"

using std::string;
using std::stringstream;
using namespace usock;

Socket::Socket() throw() : BaseSocket(inet, sock_stream, tcp)  {}

Socket::Socket (int sd, double timeout) throw()  {
	this->sd = sd;
	domain = AF_INET;
	type = SOCK_STREAM;
	protocol = IPPROTO_TCP;
	this->timeout = timeout;
}

Socket::Socket (const string& host, u_int16_t port, double timeout) throw() : BaseSocket(inet, sock_stream, tcp, timeout)  {
	connect(host,port);
}

void Socket::connect (const string& host, u_int16_t port) throw()  {
	string addr;
	struct sockaddr_in sin;

	addr = getHostByName(host);
	sin.sin_family = domain;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = inet_addr(addr.c_str());

	if (timeout == 0.0)  {
		if (::connect(sd, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0)
			throw SocketException("connect exception");
	} else {
		setBlocking(false);

		if (::connect(sd, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0)  {
			if (errno == EINPROGRESS)  {
				int ret;

				do  {
					struct timeval tv;
					fd_set set;

					tv.tv_sec = (int) timeout;
					tv.tv_usec = (int) ((timeout*1000) - (double) (tv.tv_sec*1000));
					FD_ZERO(&set);
					FD_SET(sd, &set);

					if ((ret = select(sd+1, NULL, &set, NULL, &tv)) < 0)  {
						if (errno == EINTR)
							throw SocketException("connection exception");
					} else if (ret>0)
						break;
					else
						throw SocketException("connection timeout");
				} while(1);
			} else
				throw SocketException("connection exception");
		
			setBlocking(true);
		}
	}
}

void Socket::send (const string& buf) throw()  {
	if (timeout == 0.0)  {
		if (::send(sd, buf.c_str(), buf.length(), 0) < 0)
			throw SocketException("send exception");
	} else {
		setBlocking(false);

		if (::send(sd, buf.c_str(), buf.length(), 0) < 0)  {
			if (errno == EINPROGRESS)  {
				int ret;

				do  {
					struct timeval tv;
					fd_set set;

					tv.tv_sec = (int) timeout;
					tv.tv_usec = (int) ((timeout*1000) - (double) (tv.tv_sec*1000));
					FD_ZERO(&set);
					FD_SET(sd, &set);

					if ((ret = select(sd+1, NULL, &set, NULL, &tv)) < 0)  {
						if (errno == EINTR)
							throw SocketException("connection exception");
					} else if (ret>0)
						break;
					else
						throw SocketException("connection timeout");
				} while(1);
			} else
				throw SocketException("connection exception");
		}
			
		setBlocking(true);
	}
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
	raii_array<char> buf_holder(buf);
	u_int32_t n;

	if (timeout == 0.0)  {
		if ((n = ::recv(sd, buf, nbytes, 0)) < 0)
			throw SocketException("recv exception");
	} else {
		setBlocking(false);

		if ((n = ::recv(sd, buf, nbytes, 0)) < 0)  {
			if (errno == EINPROGRESS)  {
				int ret;

				do  {
					struct timeval tv;
					fd_set set;

					tv.tv_sec = (int) timeout;
					tv.tv_usec = (int) ((timeout*1000) - (double) (tv.tv_sec*1000));
					FD_ZERO(&set);
					FD_SET(sd, &set);

					if ((ret = select(sd+1, NULL, &set, NULL, &tv)) < 0)  {
						if (errno == EINTR)
							throw SocketException("connection exception");
					} else if (ret>0)
						break;
					else
						throw SocketException("connection timeout");
				} while(1);
			} else
				throw SocketException("connection exception");	
		}
		
		setBlocking(true);
	}

	if (!n) return string();
	return string(buf);
}

void Socket::recv (void* buf, u_int32_t size) throw()  {
	u_int32_t n;

	if (timeout == 0.0)  {
		if ((n = ::recv(sd, buf, size, 0)) < 0)
			throw SocketException("recv exception");
	} else {
		setBlocking(false);

		if ((n = ::recv(sd, buf, size, 0)) < 0)  {
			if (errno == EINPROGRESS)  {
				int ret;

				do  {
					struct timeval tv;
					fd_set set;

					tv.tv_sec = (int) timeout;
					tv.tv_usec = (int) ((timeout*1000) - (double) (tv.tv_sec*1000));
					FD_ZERO(&set);
					FD_SET(sd, &set);

					if ((ret = select(sd+1, NULL, &set, NULL, &tv)) < 0)  {
						if (errno == EINTR)
							throw SocketException("connection exception");
					} else if (ret>0)
						break;
					else
						throw SocketException("connection timeout");
				} while(1);
			} else
				throw SocketException("connection exception");
		}
		
		setBlocking(true);
	}

	if (!n) buf = NULL;
}

void Socket::operator>> (string& buf) throw()  {
	buf = recv();
}

string Socket::readline() throw()  {
	string line;
	bool isEOF = false;
	bool isEOL = false;

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

