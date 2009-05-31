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

#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "usock.h"
#include "usock_exception.h"

using std::string;
using namespace usock;

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

RawSocket::RawSocket (string i) throw()  {	
	if (!(i.empty()))
		iface = i;
	else  {
		int raw;

		if ((raw = socket(inet, sock_stream, tcp)) < 0)
			throw SocketException("socket error");

		bool ok = false;
		struct ifreq ifr;
		struct sockaddr_in *sin = (struct sockaddr_in*) &ifr.ifr_addr;

		// I assume a host has no more than 10 invalid network interfaces before having a valid one
		for (int i=1; i < 10 && !ok; i++)  {
			ifr.ifr_ifindex = i;
			ioctl(raw,SIOCGIFNAME,&ifr);
			ioctl(raw,SIOCGIFADDR,&ifr);

			if ((sin->sin_addr.s_addr) && (ifr.ifr_name))  {
				char *addr = inet_ntoa(sin->sin_addr);
				char *substr = strstr(addr, "127.");

				if ((int) (substr - addr))
					ok = true;
			}
		}

		if (!ok)
			throw SocketException("no valid up & running network interface was found");

		iface = ifr.ifr_name;
		::close(raw);
	}

	is_IPv4 = false;
	is_TCP  = false;
	is_UDP  = false;
	is_ICMPv4 = false;

	head_len=0;
	payload_len=0;
}

string RawSocket::getIPv4addr() throw()  {
	int raw;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in*) &ifr.ifr_addr;
	
	if ((raw = socket(inet, sock_stream, tcp)) < 0)
		throw SocketException("socket error");

	strncpy (ifr.ifr_name, iface.c_str(), sizeof(ifr.ifr_name));

	ioctl(raw,SIOCGIFNAME,&ifr);
	ioctl(raw,SIOCGIFADDR,&ifr);

	if (!sin->sin_addr.s_addr)
		throw SocketException ("could not fetch a valid address for the specified network interface");

	::close(raw);
	return string(inet_ntoa(sin->sin_addr));
}

string RawSocket::getHWaddr() throw()  {
	int raw;
	
	if ((raw = socket(inet, sock_dgram, tcp)) < 0)
		throw SocketException("socket error");

	char *hwaddr = new char[ETH_ALEN];
	char *aschwaddr = new char[ETH_ALEN*4];
	struct ifreq ifr;

	strncpy (ifr.ifr_name, iface.c_str(), sizeof(ifr.ifr_name));

	if (ioctl(raw,SIOCGIFHWADDR,&ifr) < 0)
		throw SocketException("ioctl error");

	memcpy (hwaddr, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	snprintf (aschwaddr, ETH_ALEN*4, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			(unsigned char) hwaddr[0],
			(unsigned char) hwaddr[1],
			(unsigned char) hwaddr[2],
			(unsigned char) hwaddr[3],
			(unsigned char) hwaddr[4],
			(unsigned char) hwaddr[5]);

	::close(raw);
	return string(aschwaddr);
}

void RawSocket::buildIPv4 (string dst, string src, u_int8_t proto, u_int8_t ttl, u_int16_t len,
		u_int8_t tos, u_int16_t id, u_int16_t frag, u_int16_t sum)  {

	struct iphdr ip;
	is_IPv4 = true;

	if (src.empty())
		src = getIPv4addr();

	ip.version = 4;
	ip.ihl = 5;
	ip.tos = tos;
	ip.tot_len = len;
	ip.id = id;
	ip.frag_off = frag;
	ip.ttl = ttl;
	ip.protocol = proto;
	ip.check = sum;
	ip.saddr = inet_addr(src.c_str());
	ip.daddr = inet_addr(dst.c_str());

	memcpy (head, &ip, sizeof(struct iphdr));
	head_len += sizeof(struct iphdr);
}

void RawSocket::buildICMPv4 (u_int8_t type, u_int16_t id, u_int16_t seq, u_int8_t code, u_int16_t sum)  {
	struct icmp_hdr icmp;
	is_ICMPv4 = true;

	icmp.type = type;
	icmp.code = code;
	icmp.checksum = sum;
	icmp.id = 1;
	icmp.sequence = seq;

	memcpy (head + ((is_IPv4) ? sizeof(struct iphdr) : sizeof(struct ip6_hdr)), &icmp, sizeof(struct icmp_hdr));
	head_len += sizeof(struct icmp_hdr);
}

void RawSocket::buildUDP (u_int16_t sport, u_int16_t dport, u_int16_t len, u_int16_t sum)  {
	struct udphdr udp;
	is_UDP = true;

	udp.source = htons(sport);
	udp.dest = htons(dport);
	udp.len = htons(len);
	udp.check = sum;

	memcpy (head + ((is_IPv4) ? sizeof(struct iphdr) : sizeof(struct ip6_hdr)), &udp, sizeof(struct udphdr));
	head_len += sizeof(struct udphdr);
}

void RawSocket::buildTCP (u_int16_t sport, u_int16_t dport, u_int8_t flags, u_int32_t seq, u_int32_t ack,
			u_int16_t window, u_int16_t urgent, u_int16_t sum)  {

	struct tcphdr tcp;
	is_TCP = true;
	memset (&tcp, 0x0, sizeof(struct tcphdr));

	tcp.source = htons(sport);
	tcp.dest = htons(dport);

	if (!seq)  {
		srand(time(NULL));
		tcp.seq = htonl(rand());
	} else
		tcp.seq = seq;

	tcp.ack_seq = ack;
	tcp.doff = 5;

	tcp.fin = (flags & TH_FIN)  ? 1 : 0;
	tcp.syn = (flags & TH_SYN)  ? 1 : 0;
	tcp.rst = (flags & TH_RST)  ? 1 : 0;
	tcp.psh = (flags & TH_PUSH) ? 1 : 0;
	tcp.ack = (flags & TH_ACK)  ? 1 : 0;
	tcp.urg = (flags & TH_URG)  ? 1 : 0;

	tcp.window = window;
	tcp.check = sum;
	tcp.urg_ptr = urgent;

	memcpy (head + ((is_IPv4) ? sizeof(struct iphdr) : sizeof(struct ip6_hdr)), &tcp, sizeof(struct tcphdr));
	head_len += sizeof(struct tcphdr);
}

void RawSocket::setPayload (void *payload, int length)  {
	this->payload = new u_int8_t[length];
	payload_len = length;
	memcpy (this->payload, payload, length);
}

void RawSocket::setPayload (string payload)  {
	this->payload = (u_int8_t*) strdup(payload.c_str());
}

u_int16_t RawSocket::csum (u_int16_t *buf, int nwords)  {
	u_int32_t sum;

	for (sum = 0; nwords > 0; nwords--)
		sum += *buf++;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return ~sum;
}

void RawSocket::write() throw()  {
	u_int32_t len = head_len + payload_len;
	u_int8_t *pkt = new u_int8_t[len];
	u_int32_t opt = 1;
	
	struct iphdr ip;
	struct tcphdr tcp;
	struct udphdr udp;
	struct icmp_hdr icmp;

	memcpy (pkt, head, head_len);

	if (payload_len > 0)
		memcpy (pkt+head_len, payload, payload_len);

	if (is_IPv4)  {
		memcpy (&ip, head, sizeof(struct iphdr));

		if (!ip.tot_len)
			ip.tot_len = head_len + payload_len;

		if (!ip.check)  {
			u_int8_t *buf = new u_int8_t[sizeof(struct iphdr) + payload_len];
			memcpy (buf, &ip, sizeof(struct iphdr));
			memcpy (buf+sizeof(struct iphdr), payload, payload_len);

			ip.check = csum((u_int16_t*) buf, ip.tot_len >> 1);
			memcpy (pkt, &ip, sizeof(struct iphdr));
			delete [] buf;
		}
	}

	if (is_ICMPv4)  {
		memcpy (&icmp, head+sizeof(struct iphdr), sizeof(struct icmp_hdr));

		if (!icmp.checksum)  {
			int buflen = sizeof(struct icmp_hdr) + payload_len;
			u_int8_t *buf = new u_int8_t[buflen];
			memcpy (buf, &icmp, sizeof(struct icmp_hdr));
			memcpy (buf+sizeof(struct icmp_hdr), payload, payload_len);

			icmp.checksum = csum((u_int16_t*) buf, buflen >> 1);
			memcpy (pkt+sizeof(struct iphdr), &icmp, sizeof(struct icmp_hdr));
			delete [] buf;
		}
	}

	if (is_UDP)  {
		if (is_IPv4)
			memcpy (&udp, head + sizeof(struct iphdr), sizeof(struct udphdr));
		else
			memcpy (&udp + sizeof(struct ip6_hdr), head, sizeof(struct udphdr));

		if (!udp.len)
			udp.len = htons(sizeof(struct udphdr) + payload_len);

		if (!udp.check)  {
			u_int32_t buflen = ((is_IPv4) ? sizeof(struct iphdr) : sizeof(struct ip6_hdr)) +
				sizeof(struct udphdr) + payload_len;

			u_int8_t *buf = new u_int8_t[buflen];
			struct pseudohdr pseudo;

			if (is_IPv4)  {
				pseudo.src = ip.saddr;
				pseudo.dst = ip.daddr;
				pseudo.padd = (!(payload_len%2) || !payload_len) ? 0 : 1;
				pseudo.proto = ip.protocol;
				pseudo.len = htons(sizeof(struct udphdr) + payload_len);
			}

			memcpy (buf, &pseudo, sizeof(struct pseudohdr));
			memcpy (buf+sizeof(struct pseudohdr), &udp, sizeof(struct udphdr));

			if (payload_len > 0)
				memcpy (buf+sizeof(struct pseudohdr)+sizeof(struct udphdr), payload, payload_len);

			udp.check = csum((u_int16_t*) buf, buflen >> 1);
			delete [] buf;
		}

		if (is_IPv4)
			memcpy (pkt+sizeof(struct iphdr), &udp, sizeof(struct udphdr));
		else
			memcpy (pkt+sizeof(struct ip6_hdr), &udp, sizeof(struct udphdr));
	}

	if (is_TCP)  {
		if (is_IPv4)
			memcpy (&tcp, head + sizeof(struct iphdr), sizeof(struct tcphdr));
		else
			memcpy (&tcp + sizeof(struct ip6_hdr), head, sizeof(struct tcphdr));

		if (!tcp.check)  {
			u_int32_t buflen = ((is_IPv4) ? sizeof(struct iphdr) : sizeof(struct ip6_hdr)) +
				sizeof(struct tcphdr) + payload_len;

			u_int8_t *buf = new u_int8_t[buflen];
			struct pseudohdr pseudo;

			if (is_IPv4)  {
				pseudo.src = ip.saddr;
				pseudo.dst = ip.daddr;
				pseudo.padd = (!(payload_len%2) || !payload_len) ? 0 : 1;
				pseudo.proto = ip.protocol;
				pseudo.len = htons(sizeof(struct tcphdr) + payload_len);
			}

			memcpy (buf, &pseudo, sizeof(struct pseudohdr));
			memcpy (buf+sizeof(struct pseudohdr), &tcp, sizeof(struct tcphdr));

			if (payload_len > 0)
				memcpy (buf+sizeof(struct pseudohdr)+sizeof(struct tcphdr), payload, payload_len);

			tcp.check = csum((u_int16_t*) buf, buflen >> 1);
			delete [] buf;
		}

		if (is_IPv4)
			memcpy (pkt+sizeof(struct iphdr), &tcp, sizeof(struct tcphdr));
		else
			memcpy (pkt+sizeof(struct ip6_hdr), &tcp, sizeof(struct tcphdr));
	}
	
	if (is_IPv4)  {
		memcpy (&ip, head, sizeof(struct iphdr));
		
		if ((sd = socket(inet, sock_raw, ip.protocol)) < 0)
			throw SocketException("socket error");

		if (::setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0)
			throw SocketException("setsockopt error");

		struct sockaddr_in sin;
		sin.sin_family = inet;

		if (is_ICMPv4)
			sin.sin_port = 0;
		else  {
			if (is_TCP)
				sin.sin_port = tcp.dest;
			else if (is_UDP)
				sin.sin_port = udp.dest;
		}

		sin.sin_addr.s_addr = ip.daddr;

		if (timeout == 0.0)  {
			if (sendto(sd, pkt, len, 0, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0)
				throw SocketException("sendto error");
		} else {
			if (sendto(sd, pkt, len, 0, (struct sockaddr*) &sin, sizeof(struct sockaddr)) < 0)  {
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
		}
	}
}

void* RawSocket::read (u_int32_t len, const string& host) throw()  {
	u_int8_t *buf = NULL;

	if (len)
		buf = new u_int8_t[len];

	struct sockaddr_in sin;
	int opt = 1;
	socklen_t slen = sizeof(struct sockaddr_in);

	if (is_IPv4)  {
		struct iphdr ip;
		memcpy (&ip, head, sizeof(struct iphdr));

		if (!buf)  {
			len = ip.tot_len;
			buf = new u_int8_t[len];
		}

		if ((sd = socket(inet, sock_raw, ip.protocol)) < 0)
			throw SocketException("socket error");
	
		if (::setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0)
			throw SocketException("setsockopt error");

		sin.sin_family = inet;
		sin.sin_port = 0;

		if (host.empty())
			sin.sin_addr.s_addr = any;
		else
			sin.sin_addr.s_addr = inet_addr(getHostByName(host.c_str()).c_str());

		if (timeout == 0.0)  {
			if (recvfrom(sd, buf, len, 0, (struct sockaddr*) &sin, &slen) < 0)
				throw SocketException("recvfrom error");
		} else {
			if (recvfrom(sd, buf, len, 0, (struct sockaddr*) &sin, &slen) < 0)  {
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
		}
	}

	return (void*) buf;
}

