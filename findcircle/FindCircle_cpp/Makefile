CC = arm-hisiv300-linux-gcc
CXX = arm-hisiv300-linux-g++
AR = arm-hisiv300-linux-ar

ARFLAG = rc
CFLAGS = -O3 -fPIC -Wall -pipe
CXXFLAGS = $(CFLAGS)
CPPFLAGS = -Wp,-DNDEBUG
LDFLAGS = -s -Wl,-O3

INCLDIR = -I. -I./include -I./opencv2

CV_LIBDIR = -L. -L./opencv_lib -lm -lpthread -lz -ldl

CV_LIBS = opencv_lib/libopencv_imgproc.a \
			opencv_lib/libopencv_core.a

TARGET = libFindCircle.a
OBJS = findCircle.o
SRCS = findCircle.cpp

.PHONY:all
all:$(TARGET) $(OBJS)

$(OBJS):$(SRCS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $*.cpp $(INCLDIR)
	
$(TARGET):$(OBJS)
	$(AR) $(ARFLAG) $(TARGET) $(OBJS)
	
clean:
	rm -f $(TARGET) $(OBJS)
	
install : $(TARGET)
	cp -f $(TARGET) /usr/lib