/**
 * An elementary TCP server
 *
 * It just listens for connections on port 9999, and whenever a connection arrives
 * it creates a new process to handle that, using a transparent callback function,
 * sending the listening client some strings.
 *
 * by BlackLight, (C) 2009
 */

#include <iostream>
#include <usock.h>

using namespace std;
using namespace usock;

void foo (Socket& s)  {
	cout << "Connection from " << s.remoteAddr() << endl;
	s << "Hello " + s.remoteAddr() + "\n";
	s.close();
}

main()  {
	ServerSocket ss(9999);

	while(1)  {
		try  {
			ss.accept(foo);
		}

		catch (exception e)  {}
	}

	ss.close();
}

