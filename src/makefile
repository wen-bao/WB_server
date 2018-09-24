CC = gcc -std=gnu99

WB:main.o util.o epoll.o http.o http_parse.o http_request.o timer.o priority_queue.o threadpool.o rio.o
	$(CC) *.o -o WB -lpthread

main.o:main.c
	$(CC) -c main.c

util.o:util.c util.h
	$(CC) -c util.c

epoll.o:epoll.c epoll.h
	$(CC) -c epoll.c

http.o:http.c http.h
	$(CC) -c http.c

http_parse.o:http_parse.c http_parse.h
	$(CC) -c http_parse.c

http_request.o:http_request.c http_request.h
	$(CC) -c http_request.c

timer.o:timer.c timer.h
	$(CC) -c timer.c

priority_queue.o:priority_queue.h priority_queue.c
	$(CC) -c priority_queue.c

threadpool.o:threadpool.c threadpool.h
	$(CC) -c threadpool.c

rio.o:rio.h rio.c
	$(CC) -c rio.c
clean:
	rm *.o WB
