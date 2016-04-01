CC=g++
CFLAGS=-std=c++11 -fPIC -g
LFLAGS=-levent -lpthread
INC=-I./ -I./src -I/usr/local/include
BIN=test
LIBS=libasio_fw.so
LIB_OBJS=src/net_listener.o src/session_worker.o src/http_parser.o src/http_meta.o src/n0_string.o src/worker.o
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
src/http_parser.o:src/http_parser.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

src/http_meta.o:src/http_meta.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

src/n0_string.o:src/n0_string.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}
src/worker.o:src/worker.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

tests/test.o:tests/test.cc
	${CC} -o $@ -c $< ${INC} ${CFLAGS}

all:${LIBS} ${BIN}

clean:
	rm -f ${OBJS} ${BIN} ${LIBS}

.PHONY:clean all
