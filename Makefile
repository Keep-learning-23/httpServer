target = ./bin/server
depend = ./build/epoll.o ./build/http_conn.o ./build/locker.o \
./build/server.o ./build/threadpool.o ./build/utils.o ./build/sorted_timer.o ./build/main.o

CC = g++
$(target) : $(depend)
	$(CC) $(depend) -o $(target)
./build/%.o : ./src/%.cpp
	$(CC) -c $< -o $@

clean :
	cd ./test && make clean && cd .. && rm -rf $(depend) $(target)