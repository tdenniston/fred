all: pthread-test pthread-test-thread-private test-list test-list-no-malloc syscall-tester pthread-cond-var time many-threads

clean:
	rm -f pthread-test test-list test-list-no-malloc syscall-tester time many-threads

pthread-test: pthread-test.c
	gcc -o pthread-test pthread-test.c -g -O0 -lpthread

pthread-test-thread-private: pthread-test-thread-private.c
	gcc -o pthread-test-thread-private pthread-test-thread-private.c -g -O0 -lpthread

pthread-cond-var: pthread-cond-var.c
	gcc -o pthread-cond-var pthread-cond-var.c -g -O0 -lpthread

test-list: test-list.c
	gcc -o test-list test-list.c -g -O0

test-list-no-malloc: test-list-no-malloc.c
	gcc -o test-list-no-malloc test-list-no-malloc.c -g -O0

syscall-tester: syscall-tester.c
	gcc -o syscall-tester syscall-tester.c -g -O0 -DLINUX -DGLIBC

time: time.c
	gcc -o time time.c -g -O0 -lpthread

many-threads: many-threads.c
	gcc -o many-threads many-threads.c -g -O0 -lpthread
