CC = arm-hisiv300-linux-gcc
CXX = arm-hisiv300-linux-g++
AR = arm-hisiv300-linux-ar

ARFLAG = rc
CFLAGS = -O3 -Wall -pipe
CXXFLAGS = $(CFLAGS)
CPPFLAGS = -Wp,-DNDEBUG
LDFLAGS = -s -Wl,-O3

INCLDIR = -I.

TARGET = libzq_CV.a
OBJS = zq_CV.o
SRCS = zq_CV.c

.PHONY:all
all:$(TARGET) $(OBJS)

$(OBJS):$(SRCS)
	$(CC) $(CFLAGS) -c $*.c $(INCLDIR)
	
$(TARGET):$(OBJS)
	$(AR) $(ARFLAG) $(TARGET) $(OBJS)
	
clean:
	rm -f $(TARGET) $(OBJS)
	
install : $(TARGET)
	cp -f $(TARGET) /usr/lib