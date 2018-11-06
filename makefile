CC 		= gcc -std=gnu99 -w
INCLUDE = include
SRC 	= src
BUILD	= build
CCFLAGS = -I$(INCLUDE)

ALLBUILD = $(BUILD)/util.o				\
		   $(BUILD)/epoll.o				\
		   $(BUILD)/http.o				\
		   $(BUILD)/http_parse.o		\
		   $(BUILD)/http_request.o		\
		   $(BUILD)/timer.o				\
		   $(BUILD)/priority_queue.o	\
		   $(BUILD)/threadpool.o		\
		   $(BUILD)/rio.o

all : $(BUILD)/door wb

wb : $(SRC)/wb.c
	$(CC) $(CCFLAGS) -o $@ $(SRC)/wb.c

$(BUILD)/door : $(ALLBUILD) $(SRC)/door.c
	$(CC) $(CCFLAGS) -o $@ $(BUILD)/*.o $(SRC)/door.c -lpthread

$(BUILD)/util.o : $(SRC)/util.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/util.c

$(BUILD)/epoll.o : $(SRC)/epoll.c $(BUILD)/util.o
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/epoll.c

$(BUILD)/http.o : $(SRC)/http.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/http.c

$(BUILD)/http_parse.o : $(SRC)/http_parse.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/http_parse.c

$(BUILD)/http_request.o : $(SRC)/http_request.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/http_request.c

$(BUILD)/timer.o : $(SRC)/timer.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/timer.c

$(BUILD)/priority_queue.o : $(SRC)/priority_queue.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/priority_queue.c

$(BUILD)/threadpool.o : $(SRC)/threadpool.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/threadpool.c

$(BUILD)/rio.o : $(SRC)/rio.c
	$(CC) $(CCFLAGS) -c -o $@ $(SRC)/rio.c
clean:
	rm $(BUILD)/* wb
