/**
 * An elementary TCP client
 * It just connects to a server on 'localhost' and reads a string from the socket
 * by BlackLight, (C) 2009
 */

#include <iostream>
#include <usock.h>

using namespace std;
using namespace usock;

main()  {
	Socket s("localhost",9999);
	string line = s.readline();
	cout << "The server sent: " << line << endl;
	s.close();
}

