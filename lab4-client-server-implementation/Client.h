#include "DataStructure.h"
#include "AuxFunctions.h"
#include <mutex>
#include <ctime>
#include <fstream>
#ifndef _CLIENT
#define _CLIENT
using std::mutex;
using std::clock;
using std::ofstream;
extern Ch upCh, downCh[];
extern mutex uplock, downlock[];
extern ofstream sof, cof[];

// client class
class Client{
private:
	char id; // client id
	Ch* ownCh; // client channel which is actually the global downCh[id-'a']. its reference is here.
	char buff[500]; // a buffer used to append char arrays for outputs
	int t2;  // the fixed delay time.
public:
	char own[11]; // its owned resource list. own[11] is NULL for array ending. only first 10 chars will be used.
	
	// constructor	
	Client(char nid){
		id = nid; // assigns client id
		ownCh = &downCh[nid - 'a']; // assigns its channel 
	}
	
	char getid(){
		return id; // id accessor
	}


	// resource request method	
	void requestQ(int num){
		// loggin part
		flushbuff(); // clean up the buffer
		// compose the logging message
		buff[0] = id;
		for (int i = 0; i < 10; i++){
			buff[i + 1] = " requests "[i];
		}
		buff[11] = num + '0';
		for (int i = 0; i < 12; i++){
			buff[1 + 10 + 1 + i] = " resources.\n"[i];
		}
		// ouput the loggin message
		cof[id - 'a'] << clock() << " " << buff;

		// send request
		char tmp[4] = { id, 'Q', num + 0, 0 }; // compose the request message, i
												 //d for id, 
												 //'Q' for request type, n
												 //um+'0' for number of requested resources
												 // 0 (NULL) ends the char array
		uplock.lock(); // lock the channel to prevent data race
		upCh.send(tmp, 4); // send the message
		uplock.unlock(); // unlock the channel to let others to use.

	}

	// chesk its channel
	char* check(){
		return ownCh->check();
	}


	// read from its channel
	char* read(){
		// loggin procedure
		flushbuff(); // clean up the buffer
		// compose the logging message
		buff[0] = id;
		char* temp = " read message from server";
		for (int i = 1; i < 1 + 25; i++){
			buff[i] = temp[i - 1];
		}
		buff[26] = ' ';
		for (int i = 0; i < 10; i++){
			buff[i + 1 + 26] = ownCh->check()[i];
		}
		buff[1 + 26 + 10] = '\n';
		cof[id - 'a'] << clock() << " " << buff; // outputs the logging message

		// read the message
		downlock[id - 'a'].lock(); // lock the channel to prevent data race
		char* rlt = ownCh->read(); // read the data
		downlock[id - 'a'].unlock(); // unlock the channel for later use.
		return rlt;

	}

	// request release method
	void requestL(char eleNum){
		// logging part
		flushbuff(); // clean up the buffer
		// compose the message
		buff[0] = id;
		for (int i = 0; i < 18; i++){
			buff[i + 1] = " returns resource "[i];
		}
		buff[1 + 18] = eleNum;
		buff[20] = '\n';
		cof[id - 'a'] << clock() << " " << buff; // output the logging message

		char tmp[4] = { id, 'L', eleNum, 0 }; // compose the request message
											  // id is the id
											  // "L" for the release request type
											  // eleNum for the element id that will be released
											  // 0/NULL ends the array.
		// send the request
		uplock.lock();   // lock the channel to prevent data race
		upCh.send(tmp, 4); // send the message
		uplock.unlock(); // ulock the channel for later use
	}


	// buff clean up method, simply assgine all the elements NULL.
	void flushbuff(){
		for (int i = 0; i < 500; i++){
			buff[i] = 0;
		}
	}

};
#endif