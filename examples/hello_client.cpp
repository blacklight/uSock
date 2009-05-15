#include <iostream>
#include <usock.h>
using namespace std;

main()  {
	Socket s("localhost",9999);
	string line = s.readline();
	cout << "The server sent: " << line << endl;
	s.close();
}

