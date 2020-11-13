
EXEC = qhycmd

#
# Set to 1 if shared object needs to be installed
#
SHARED_MODE=0


COMP_INC1 = /usr/include/
COMP_INC2 = /usr/local/include/

COMP_LIB = /usr/local/lib/


CXX = g++

OPENCV_LIBS = $(shell pkg-config opencv --libs)
OPENCV_CXXFLAGS = $(shell pkg-config opencv --cflags)

# wx-config --libs
WX_LIBS = $(shell wx-config --libs)
# wx-config --cxxflags
WX_CXXFLAGS = $(shell wx-config --cxxflags)

CXXFLAGS = -Wall -Wsign-compare -std=c++11 -DwxUSE_GUI=0 -I. -I $(COMP_INC1)  -I$(COMP_INC2) $(OPENCV_CXXFLAGS) $(WX_CXXFLAGS)
EXTRALIBS = -lqhyccd -lusb-1.0 -pthread -lcfitsio $(OPENCV_LIBS) $(WX_LIBS)

CP = cp -f

OBJA = qhycmd.o qhy_camera.o astro_image.o qhy_utils.o


all: $(EXEC) 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<


$(EXEC): $(OBJA) 
	$(CXX) -o qhycmd $(OBJA) $(EXTRALIBS)


install:
#	$(CP) $(EXEC)

clean:
	-$(RM) $(EXEC)
	-$(RM) *.o
	-$(RM) *~
	-$(RM) *.orig
