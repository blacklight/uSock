#include <iostream>
#include <usock.h>
using std::cout;

main()  {
	ServerSocket ss(9999);

	while(1)  {
		Socket s = ss.accept();
		cout << "Connection from " << s.remoteAddr() << endl;
		s << "Hello " + s.remoteAddr() + "\n";
		s.close();
	}

	ss.close();
}

