CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG

OBJS =		logicDispatch.o init_configure.o master_thread.o \
			worker_threads.o json_opt.o local_transport.o

LIBS =		-llog4cxx -levent -ljson -Wl,-rpath,/usr/local/lib		

TARGET =	logicDispatch

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)   

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
