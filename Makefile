#include ${FSLCONFDIR}/default.mk
#Specify the default compiler
CXX = g++

#Specify the -fpic flag
CXXFLAGS += -fpic

#Additional LDFLAGS for znzlib library
ZNZLIB_LDFLAGS = -L${HOME}/FSL-lesions/znzlib -lfsl-znz

#Define source files
SRCS =lesion_filling.cc 

#Define object files
OBJS = $(SRCS:.cc=.o)

#Define library source files and directories
LIB_DIRS = miscmaths newimage NewNifti utils cprob znzlib
LIB_SRCS = $(foreach dir,$(LIB_DIRS),$(wildcard $(dir)/*.cc))
LIB_OBJS = $(LIB_SRCS:.cc=.o)

#Define targets
all:lesion_filling

#Compile the final executable
lesion_filling: libraries $(OBJS) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIB_OBJS) $(LDFLAGS) $(ZNZLIB_LDFLAGS) -lblas -llapack -lz

#Rule to build object files
%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#Phony target to build all libraries
.PHONY: libraries
libraries:
	@for dir in $(LIB_DIRS); do \
	$(MAKE) -C $$dir CXX=$(CXX) CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)'; \
	done

#Clean rule
clean:
	rm -f lesion_filling $(OBJS) $(LIB_OBJS) $(shell find . -type f \( -name "*.o" -o -name "*.so" \))
