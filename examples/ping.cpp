/**
 * Sample pinger program
 * It just requires a host to ping, and nothing more
 * by BlackLight, (C) 2009
 */

#include <iostream>
#include <usock.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

using namespace std;
using namespace usock;

int main(int argc, char **argv)  {
	if (argc<2)  {
		cerr << "Usage: " << argv[0] << " <host>\n";
		return 1;
	}

	u_int8_t payload[56];

	for (int i=0; i < sizeof(payload); i++)
		payload[i] = i;

	RawSocket s;
	s.buildIPv4(s.getHostByName(argv[1]), s.getIPv4addr(), RawSocket::icmp);
	s.buildICMPv4(ICMP_ECHO);
	s.setPayload(payload, sizeof(payload));
	
	cout << "Sending a ping request to " << argv[1] << endl;
	s.setTimeout(1.0);

	try  {
		s.write();
		
		unsigned char* buf = (unsigned char*) s.read(64, argv[1]);
		struct iphdr ip;
		memcpy (&ip, buf, sizeof(struct iphdr));
		string addr = s.ntoa(ip.saddr);

		cout << "Reply from " << addr << endl;
	}

	catch (exception e)  {
		cerr << e.what() << endl;
	}

	s.close();
}

