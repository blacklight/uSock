/**
 * A simple traceroute program
 * It just requires the host to traceroute, usock does anything else.
 * by BlackLight, (C) 2009
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <usock.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>

using namespace std;
using namespace usock;

int main(int argc, char **argv)  {
	if (!argv[1])  {
		cerr << "Usage: " << argv[0] << " <host>\n";
		return 1;
	}

	RawSocket ss;
	string addr = ss.getHostByName(argv[1]);
	ss.close();

	if (addr.empty())  {
		cerr << "Unable to resolve " << argv[1] << endl;
		return 1;
	}

	u_int8_t payload[56];

	for (int i=0; i < sizeof(payload); i++)
		payload[i] = i;

	bool ended = false;
	struct timeval t1, t2;
	cout << "Tracerouting " << argv[1] << " (" << addr << ")\n\n";

	for (int i=1; !ended; i++)  {
		RawSocket s;
		s.buildIPv4(addr, s.getIPv4addr(), RawSocket::icmp, i);
		s.buildICMPv4(ICMP_ECHO);
		s.setPayload(payload, sizeof(payload));
		s.write();

		gettimeofday (&t1, NULL);
		bool recvd = false;
		
		while (!recvd)  {
			try  {
				struct iphdr ip;
				struct icmphdr icmp;

				u_int8_t* pkt = (u_int8_t*) s.read(sizeof(struct iphdr)+sizeof(struct icmphdr));
				memcpy (&ip, pkt, sizeof(struct iphdr));
				memcpy (&icmp, pkt+sizeof(struct iphdr), sizeof(struct icmphdr));

				if (ip.protocol == RawSocket::icmp ||
						(icmp.type == ICMP_ECHOREPLY && !(addr.compare(s.ntoa(ip.saddr)))) )  {
						
					gettimeofday (&t2, NULL);
					time_t interval = (t2.tv_sec - t1.tv_sec)*1000 + (t2.tv_usec - t1.tv_usec)/1000;
					
					cout << i << ":\t" << s.ntoa(ip.saddr) << " (" << s.getHostByAddr(s.ntoa(ip.saddr)) << ")"
						<< " - reached in " << interval << "ms\n";
					
					recvd = true;

					if (icmp.type == ICMP_ECHOREPLY)
						ended = true;
				}
			}
			
			catch (exception e)  {}
		}

		s.close();
	}

	return 0;
}

