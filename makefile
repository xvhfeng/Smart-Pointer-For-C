.SUFFIXES: .c .o

SMART_SRC_PATH = ./src/
SMART_HEADER_PATH = ./src/
SMART_TEST_SRC_PATH = ./test/
SMART_TEST_HEADER_PATH = ./test/
TARGET = libSpxSmart.so
TARGET_PATH = /usr/local/bin
HEADER_TARGET_PATH = /usr/local/include/

CC = gcc
CFLAG = -pipe -o0  -W -Wall -Werror -g \
		-Wpointer-arith -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wunused-value
INC_PATH = -I/usr/local/include -I/usr/include \
			-I$(SMART_HEADER_PATH)
LIB_PATH = -L/usr/local/lib
TEST_LIB_PATH = -L/usr/local/lib -lcheck
DEFS := -DSpxDEBUG 

SYS := $(strip $(shell uname -s | tr '[:upper:]' '[:lower:]'))
ifeq ($(SYS), linux)
	DEFS += -DSpxLinux
else
	ifeq ($(SYS),unix)
		DEFS += -DSpxUnix
	else
		ifeq ($(SYS),darwin)
			DEFS += -DSpxMac
		endif
	endif
endif

BITS := $(findstring 64,$(shell uname -m ))
ifeq (64, $(BITS))
	DEFS += -DSpx64
else
	ifeq (32, $(BITS))
		DEFS += -DSpx32
	else
		DEFS += -DSpx32
	endif
endif

SRC_FILE = $(wildcard *.c) \
			$(wildcard $(SMART_SRC_PATH)/*.c)

HEADER_FILE = $(wildcard *.h) \
			$(wildcard $(SMART_HEADER_PATH)/*.h)

SHARED_OBJS = $(patsubst %.c,%.o,$(SRC_FILE) )

TEST_SRC_FILE = $(wildcard $(SMART_TEST_SRC_PATH)/*.c) \
				$(wildcard $(SMART_SRC_PATH)/*.c)

TEST_SHARED_OBJS = $(patsubst %.c,%.o,$(TEST_SRC_FILE) )


all:$(TARGET)

%.o: %.c
	$(CC) $(CFLAG) -c $< -o $@ $(LIB_PATH) $(INC_PATH) $(DEFS)

$(TARGET) : $(SHARED_OBJS)
	$(CC) $(CFLAG) $(SHARED_OBJS) $(LIB_PATH) $(INC_PATH) -o $(TARGET) $(DEFS)

test : $(TEST_SHARED_OBJS)
	$(CC) $(CFLAG) -I$(SMART_TEST_HEADER_PATH) $(TEST_SHARED_OBJS) $(TEST_LIB_PATH) $(INC_PATH) -o spx_smart_test $(DEFS) -lcheck

install:
	cp -f $(TARGET) $(TARGET_PATH)
	cp -f $(HEADER_FILE) $(HEADER_TARGET_PATH)
clean:
	rm -f $(SHARED_OBJS) $(TARGET)
	rm -f $(TEST_SHARED_OBJS) $(TARGET)
uninstall:
	cd $(TARGET_PATH)
	rm -f $(TARGET)
	cd $(HEADER_TARGET_PATH)
	rm -f $(HEADER_FILE)


