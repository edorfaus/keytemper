CFLAGS += -std=gnu99 -Wall -pedantic -Wextra

HIDAPI_DIR = ../hidapi.git/
CFLAGS += -I${HIDAPI_DIR}hidapi

HIDAPI_LIB_DIR = ${HIDAPI_DIR}linux/.libs
HIDAPI_LIB = hidapi-hidraw

.PHONY: all
all: keytemper

keytemper: LDFLAGS += -L${HIDAPI_LIB_DIR} -Wl,-rpath=${HIDAPI_LIB_DIR} -l${HIDAPI_LIB}

.PHONY: clean
clean:
	${RM} keytemper
