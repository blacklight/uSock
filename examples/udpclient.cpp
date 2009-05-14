#include <usock.h>

main()  {
	UDPSocket s;
	s.send("test","localhost",9999);
	s.close();
}

