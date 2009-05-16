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

