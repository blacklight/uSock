#include <usock.h>

main()  {
	string buf;
	Socket s("www.google.it",80);

	s << "HEAD / HTTP/1.1\r\n"
		"Host: www.google.it\r\n"
		"Connection: close\r\n\r\n";

	do  {
		buf = s.readline();
		cout << buf << endl;
	} while (!buf.empty());

	s.close();
}

