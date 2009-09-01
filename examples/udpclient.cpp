/**
 * A simple UDP client that sends a string over an UDP socket
 * by BlackLight, (C) 2009
 */

#include <memory.h>
#include <usock.h>
using namespace usock;

main()  {
	UDPSocket s;
	s.send("test","localhost",9999);
	s.close();
}

