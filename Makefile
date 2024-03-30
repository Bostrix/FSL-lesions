include ${FSLCONFDIR}/default.mk

PROJNAME = lesions
XFILES   = lesion_filling
LIBS     = -lfsl-newimage -lfsl-miscmaths -lfsl-NewNifti \
           -lfsl-znz -lfsl-cprob -lfsl-utils

all: ${XFILES}

lesion_filling:	lesion_filling.o
	${CXX}  ${CXXFLAGS} -o $@ $^ ${LDFLAGS}
