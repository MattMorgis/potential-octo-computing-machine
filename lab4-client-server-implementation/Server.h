#include "DataStructure.h"
#include "AuxFunctions.h"
#include <mutex>
#include <fstream>
#include <ctime>
#ifndef SERVER
#define SERVER
using std::mutex;
using std::ofstream;
using std::clock;
extern Ch upCh, downCh[];
extern mutex uplock, downlock[];
extern ofstream sof, cof[];


// server class
class Server{
private:
	char recource[11]; // structure that keep track of its data
	char buff[500];    // buffer for logging information preparing
	Ch BUF = Ch();     // its buffer queue that keeps the requests that cannot be fulfilled at that time

	// methods that computes the available stock number
	int getstock(){
		int stock = 0;
		for (int i = 0; i<11; i++){ // go through the whole resource
			if (recource[i] == '0'){
				stock++; // increase stock if the element is free
			}
		}
		return stock;
	}


public:
	// constructor
	Server(){
		for (int i = 0; i<10; i++){
			recource[i] = '0';  // set all the resource available
		}
		recource[10] = 0; // ends the array well
	}



	// reply method for the request message
	void reply(){

		int stock = getstock(); // get the available stock number
		
		// cout << "Distributing services... Current stock is " << stock << endl;

		if (check() == NULL){ // if there is no message. Should not be since we checked before
			cout << "Weired where is the message before?" << endl; 
		}


		if (check()[2] - '0' > stock){ // if there is no enough stock
			// cout << stock << " recources cannot meet " << check()[2] << " needs. Postponding.." << endl;
			char* tmp = read(); // takes the message
			BUF.send(tmp, 4);   // put it in the pending buffer temporarily
			return;
		}

		// in ideal situation we get the message first
		char* msg = read(); // get the message first
		// cout << "get message "<<msg << endl;


		// logging procedure
		flushbuff();  // clean up the buffer
		// compose the logging message
		for (int i = 0; i < 25; i++){
			buff[i] = "Distributing resource to "[i];
		}
		buff[25] = msg[0];
		buff[26] = ' ';
		for (int i = 0; i < 10; i++){
			buff[27 + i] = recource[i];
		}
		buff[37] = '.';
		buff[38] = '\n';
		sof << clock() << " " << buff; // send it to our loggin channel


		// process the request message
		char release = '0'; // remember how many resource has been released, starts from zero
		char rply[11] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', 0 };
		for (int i = 0; i<10; i++){
			if (recource[i] == '0'){
				recource[i] = msg[0]; // assign the resource element if it is free
				rply[i] = '0' + i + 1; // put it in the message
				release = release + 1; // increase released
			}
			if (release == msg[2]){ // if enough block has been released
				break;	// stop
			}
		}


		send(msg[0], rply); // send the message

	}


	// response to release method
	void release(){

		// get the mesage first
		char* msg = read();
		// release the resource element in the message
		recource[msg[2] - '0' - 1] = '0';

		// put all request back to the up channel
		while (BUF.check() != NULL){
			upCh.send(BUF.read(), 4);
		}

		// logging part
		int stock = getstock();
		
		flushbuff(); // clean up buffer
		// compose the logging method
		for (int i = 0; i < 19; i++){
			buff[i] = "Free resource from "[i];
		}
		buff[19] = msg[0];
		buff[20] = ' ';
		buff[21] = msg[2];
		for (int i = 0; i < 25; i++){
			buff[i + 22] = ". Current resource state "[i];
		}
		for (int i = 0; i < 10; i++){
			buff[i + 47] = recource[i];
		}
		buff[57] = '\n';
		sof << clock() << " " << buff; // out put the logging message
		// cout << "After release the stock is " << stock << endl;

	}


	// check method, simply check the up channel
	char* check(){
		return upCh.check();
	}

	// read method, read the up channel
	char* read(){
		uplock.lock(); // lock the channel to prevent data race
		char* tmp = upCh.read(); // get the message
		uplock.unlock(); // unlock the channel for later use
		return tmp;
	}


	// send method to the channel
	void send(char chNum, char* msg){

		// logging procedure
		flushbuff(); // clean up the buffer
		for (int i = 0; i < 16; i++){
			buff[i] = "Send message to "[i];
		}
		buff[16] = chNum;
		buff[17] = ' ';
		for (int i = 0; i < 10; i++){
			buff[18 + i] = msg[i];
		}
		buff[28] = '\n';
		sof << clock() << " " << buff; // output the loggin message

		// send the message to proper downward channel. The channel number is computed by id-'a'
		downlock[chNum - 'a'].lock(); // lock the chennel to prevent data race
		downCh[chNum - 'a'].send(msg, 11); // send the message
		downlock[chNum - 'a'].unlock(); // unlock the chennel for later use
	}

	// buffer clean up method, simply assgine everything NULL
	void flushbuff(){
		for (int i = 0; i < 500; i++){
			buff[i] = 0;
		}
	}
};
#endif