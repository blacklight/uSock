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

#include <netinet/in.h>
#include "usock.h"
#include "usock_exception.h"

ServerSocket::ServerSocket (int domain, int type, int m) throw() : Socket(domain, type)  {
	maxconn = m;
	sock_index = 0;
}

ServerSocket::ServerSocket (u_int16_t port, u_int32_t m) throw() : Socket(domain = AF_INET, type = SOCK_STREAM)  {
	struct sockaddr_in sin;

	sin.sin_family = domain;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	maxconn = m;
	sock_index = 0;

	if (::bind(sd, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0)
		throw SocketException("bind error");

	if (::listen(sd, maxconn) < 0)
		throw SocketException("listen error");
}

void ServerSocket::bind (u_int16_t port) throw()  {
	struct sockaddr_in sin;

	sin.sin_family = domain;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;

	if (::bind(sd, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0)
		throw SocketException("bind error");
}

void ServerSocket::listen() throw()  {
	if (::listen(sd, maxconn) < 0)
		throw SocketException("listen error");
}

Socket ServerSocket::accept() throw()  {
	int new_sd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(struct sockaddr);

	if ( (new_sd = ::accept(sd, (struct sockaddr*) &addr, &len)) < 0)
		throw SocketException("accept error");

	return Socket(new_sd, domain, type);
}

