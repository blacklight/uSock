/**
 * An elementary application that makes an HTTP request
 * by BlackLight, (C) 2009
 */

#include <iostream>
#include <usock.h>

using namespace std;
using namespace usock;

main()  {
	string buf;
	Socket s("www.google.it", 80, 1.0);

	s << "HEAD / HTTP/1.1\r\n"
		"Host: www.google.it\r\n"
		"Connection: close\r\n\r\n";

	do  {
		buf = s.readline();
		cout << buf << endl;
	} while (!buf.empty());

	s.close();
}

