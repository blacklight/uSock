SRCDIR=src
INCLUDEDIR=include
PREFIX=/usr/local
LIB=usock

all:
	g++ -Wall -ansi -pedantic -pedantic-errors -I$(INCLUDEDIR) -fPIC -g -c $(SRCDIR)/basesocket.cpp
	g++ -Wall -ansi -pedantic -pedantic-errors -I$(INCLUDEDIR) -fPIC -g -c $(SRCDIR)/socket.cpp
	g++ -Wall -ansi -pedantic -pedantic-errors -I$(INCLUDEDIR) -fPIC -g -c $(SRCDIR)/rawsocket.cpp
	g++ -Wall -ansi -pedantic -pedantic-errors -I$(INCLUDEDIR) -fPIC -g -c $(SRCDIR)/serversocket.cpp
	g++ -Wall -ansi -pedantic -pedantic-errors -I$(INCLUDEDIR) -fPIC -g -c $(SRCDIR)/udpsocket.cpp
	g++ -shared -Wl,-soname,lib$(LIB).so.1 -o lib$(LIB).so.1.0.0 socket.o rawsocket.o serversocket.o udpsocket.o basesocket.o
	ar rcs lib$(LIB).a socket.o rawsocket.o serversocket.o udpsocket.o basesocket.o

install:
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/$(INCLUDEDIR)
	install -m 0644 lib$(LIB).a $(PREFIX)/lib
	install -m 0755 lib$(LIB).so.1.0.0 $(PREFIX)/lib
	ln -sf $(PREFIX)/lib/lib$(LIB).so.1.0.0 $(PREFIX)/lib/lib$(LIB).so.1
	install -m 0644 $(INCLUDEDIR)/usock.h $(PREFIX)/$(INCLUDEDIR)
	install -m 0644 $(INCLUDEDIR)/usock_exception.h $(PREFIX)/$(INCLUDEDIR)
	ldconfig

clean:
	rm *.o
	rm lib$(LIB).a
	rm lib$(LIB).so.1.0.0

uninstall:
	rm $(PREFIX)/lib/lib$(LIB).so.1
	rm $(PREFIX)/lib/lib$(LIB).so.1.0.0
	rm $(PREFIX)/lib/lib$(LIB).a
	rm $(PREFIX)/$(INCLUDEDIR)/usock.h
	rm $(PREFIX)/$(INCLUDEDIR)/usock_exception.h
