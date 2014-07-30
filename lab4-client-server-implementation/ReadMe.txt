Only standard library are used.


compile and link with the following two commands.
g++ -std=c++11 -c ./* -pthread
g++ Source.o -o test -pthread -std=c++11 -Wl,--no-as-needed

Tested on Ubuntu 12.04