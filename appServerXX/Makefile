CXXFLAGS =	-O0 -g -Wall -fmessage-length=0 -DNDEBUG 
OBJS =		appServerXX.o init_configure.o gateway_transport.o consumers.o \
 			app_operate.o 

LIBS =		-lboost_thread -lboost_regex -lboost_system  \
			-llog4cxx -levent \
			-Wl,-rpath,/usr/local/lib

TARGET =	appServerXX

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)  

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
