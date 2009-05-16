#include <iostream>
#include <usock.h>

using namespace std;
using namespace usock;

main()  {
	ServerSocket ss(9999);

	while(1)  {
		try  {
			Socket s = ss.accept();
			cout << "Connection from " << s.remoteAddr() << endl;
			s << "Hello " + s.remoteAddr() + "\n";
			s.close();
		}

		catch (exception e)  {}
	}

	ss.close();
}

