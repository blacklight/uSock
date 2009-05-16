#include <usock.h>

using namespace usock;

main()  {
	UDPSocket s;
	s.send("test","localhost",9999);
	s.close();
}

