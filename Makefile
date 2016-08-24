meowmeow: meowmeow.cpp constexprhash.h
	${CXX} -std=c++11 -g -Wall -Wextra meowmeow.cpp -o meowmeow

clean:
	rm -f meowmeow

all: meowmeow
