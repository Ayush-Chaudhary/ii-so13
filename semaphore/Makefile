CFLAGS=-march=native -O3 -pipe -fomit-frame-pointer -Wall -Wshadow -Woverflow -Wextra -Werror -Wstrict-overflow=5 -fPIC -DHELIOS -D_XOPEN_SOURCE=700 -std=c11
CXXFLAGS=-march=native -O3 -pipe -fomit-frame-pointer -Wall -Wshadow -Woverflow -Wextra -Werror -Wstrict-overflow=5 -fPIC -DHELIOS -D_XOPEN_SOURCE=700 -std=c++11

all: simple producer-consumer producer-consumer-sys

simple: simple.c semaphore_list.h semaphore_spin.h
	gcc ${CFLAGS} -lm -lpthread -pthread simple.c -o simple

producer-consumer: producer-consumer.cpp semaphore_list.h semaphore_spin.h
	g++ ${CXXFLAGS} -lm -lpthread -pthread producer-consumer.cpp -o producer-consumer

producer-consumer-sys: producer-consumer-sys.cpp
	g++ ${CXXFLAGS} -lm -lpthread -pthread producer-consumer-sys.cpp -o producer-consumer-sys

clean:
	-rm -f simple producer-consumer producer-consumer-sys
