CC=g++
CFLAGS=-std=c++11 -fPIC -g
LFLAGS=-levent -lpthread
INC=-I./ -I./src -I/usr/local/include
BIN=test
LIBS=libasio_fw.so
LIB_OBJS=src/net_listener.o src/session_worker.o
OBJS=tests/test.o ${LIB_OBJS}

libasio_fw.so:${LIB_OBJS}
	${CC} -shared -o $@ $^

test:tests/test.o
	${CC} -o $@ $^ ${LFLAGS} -L./ -lasio_fw
	mv $@ tests/$@

src/net_listener.o:src/net_listener.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

src/session_worker.o:src/session_worker.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

tests/test.o:tests/test.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

all:${LIBS} ${BIN}

clean:
	rm -f ${OBJS} ${BIN} ${LIBS}

.PHONY:clean all
