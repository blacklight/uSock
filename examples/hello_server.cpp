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

