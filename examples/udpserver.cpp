/**
 * A simple UDP server that read a string over an UDP socket
 * by BlackLight, (C) 2009
 */

#include <iostream>
#include <usock.h>

using namespace std;
using namespace usock;

main()  {
	UDPSocket s;
	s.bind(9999);

	while(1)  {
		string line = s.recv();
		cout << line << endl;
	}

	s.close();
}

