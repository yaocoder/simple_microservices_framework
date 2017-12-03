CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG 
OBJS =		logicXX.o init_configure.o local_transport.o consumers.o \
 			logic_operate.o 

LIBS =		-lboost_thread -lboost_regex -lboost_system  \
			-llog4cxx -levent \
			-Wl,-rpath,/usr/local/lib

TARGET =	logicXX

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
