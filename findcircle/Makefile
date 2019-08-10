# Hisilicon Hi3516 sample Makefile

include ../Makefile.param
#ifeq ($(SAMPLE_PARAM_FILE), )
#     SAMPLE_PARAM_FILE:=../Makefile.param
#     include $(SAMPLE_PARAM_FILE)
#endif

LDFLAGS = -s -Wl,-O3

# target source
SRC  := $(wildcard *.c) 
OBJ  := $(SRC:%.c=%.o)

TARGET := $(OBJ:%.o=%)
.PHONY : clean all

all: $(TARGET)

MPI_LIBS := $(REL_LIB)/libmpi.a
XDJ_LIB = FindCircle_cpp/libFindCircle.a
ZQ_LIB = Zq_CV/libzq_CV.a
CV_LIBDIR = -L. -L/FindCircle_cpp/opencv_lib

CV_LIBS = FindCircle_cpp/opencv_lib/libopencv_imgproc.a \
			FindCircle_cpp/opencv_lib/libopencv_core.a \
			FindCircle_cpp/opencv_lib/libz.a
		
$(TARGET):%:%.o $(COMM_OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) -lpthread -lm -o $@ $^ \
	$(XDJ_LIB) $(ZQ_LIB) $(CV_LIBDIR) $(CV_LIBS) \
	$(MPI_LIBS) $(AUDIO_LIBA) $(SENSOR_LIBS)

clean:
	@rm -f $(TARGET)
	@rm -f $(OBJ)
	@rm -f $(COMM_OBJ)

cleanstream:
	@rm -f *.h264
	@rm -f *.jpg
	@rm -f *.mjp
	@rm -f *.mp4
