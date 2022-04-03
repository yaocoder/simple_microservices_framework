CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG

OBJS =		gatewayServer.o init_configure.o master_thread.o \
			worker_threads.o json_opt.o appServer_transport.o

LIBS =		-llog4cxx -levent -ljson -Wl,-rpath,/usr/local/lib		

TARGET =	gatewayServer

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)   

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
