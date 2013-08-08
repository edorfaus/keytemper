CFLAGS += -std=gnu99 -Wall -pedantic -Wextra

# Which type of HIDAPI lib to use - libusb or hidraw. libusb is recommended.
HIDAPI_TYPE ?= libusb

# Which directory to find HIDAPI in.
HIDAPI_DIR = ../hidapi.git/


CFLAGS += -I${HIDAPI_DIR}hidapi

ifeq (${HIDAPI_TYPE},libusb)
HIDAPI_LIB_DIR = ${HIDAPI_DIR}libusb/.libs
else
ifeq (${HIDAPI_TYPE},hidraw)
HIDAPI_LIB_DIR = ${HIDAPI_DIR}linux/.libs
else
$(error Unknown HIDAPI type: ${HIDAPI_TYPE})
endif
endif

HIDAPI_LIB = hidapi-${HIDAPI_TYPE}

HIDAPI_LDFLAGS = -L${HIDAPI_LIB_DIR} -Wl,-rpath=${HIDAPI_LIB_DIR} -l${HIDAPI_LIB}

.PHONY: all
all: keytemper

keytemper: LDFLAGS += ${HIDAPI_LDFLAGS}

.PHONY: clean
clean:
	${RM} keytemper
