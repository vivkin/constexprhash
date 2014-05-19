meowmeow: meowmeow.cpp
	${CXX} -std=c++11 -O3 -Wall -Wextra meowmeow.cpp -o meowmeow

clean:
	rm meowmeow

all: meowmeow
