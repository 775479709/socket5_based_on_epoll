server_test:test_server.o epoll_master_thread.o epoll_work_thread.o epoll_common.o
	g++ -std=c++11 test_server.o epoll_master_thread.o epoll_work_thread.o  epoll_common.o -o server_test -lpthread
test_server.o:test_server.cpp epoll_master_thread.o
	g++ -c -std=c++11 test_server.cpp -o test_server.o
epoll_master_thread.o:epoll_master_thread.cpp epoll_master_tread.h epoll_work_thread.o epoll_common.o
	g++ -c -std=c++11 epoll_master_thread.cpp -o epoll_master_thread.o
epoll_work_thread.o:epoll_work_thread.cpp epoll_work_thread.h epoll_common.o
	g++ -c -std=c++11 epoll_work_thread.cpp -o epoll_work_thread.o
epoll_common.o:epoll_common.cpp epoll_common.h
	g++ -c -std=c++11 epoll_common.cpp -o epoll_common.o
clean:
	rm -rf *.o server_test
