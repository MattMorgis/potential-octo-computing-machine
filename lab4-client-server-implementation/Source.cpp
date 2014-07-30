#include "DataStructure.h"
#include "AuxFunctions.h"
#include "Server.h"
#include "Client.h"
#include <stdlib.h>
#include <thread>
#include <mutex>
using std::cout;
using std::endl;
using std::cerr;
using std::rand;
using std::thread;
using std::mutex;

// this sets the waiting time speed up, 1 is normal speed, 100 is 100 times faster.
#define SPEED 100


// globale variables
Ch upCh; // upchannel
Ch downCh[4]; // four downward chennels, 0 for client 'a', 1 for 'b', 2 for 'c', 3 for 'd'
bool flags[4]; // flags that keep track whether each client has finished 20 cycles or not
mutex uplock, downlock[4]; // 5 locks for the 5 shared chennels

ofstream sof, cof[4];  // 5 logging streams for the 5 threads



// server procedure
void serverProc(Server s){
	bool gFlag = false;

	while (!gFlag){ // loop only when some client that haven't done 20 cycles

		char* msg = s.check(); // check the message
		if ( msg != NULL){ // if there are something
			switch (msg[1]){
			case 'Q':	
				s.reply();  // reply when it is a request message
				break;
			case 'L':
				s.release(); // release when it si a release message
				break;
			default:
				cerr<< "invalid message format!"<<endl; // must be error for other cases
			}
		}
		else{}
		gFlag = flags[0] && flags[1] && flags[2] && flags[3]; // check whether all clients are done
	}
}


// client procedure
void clientProc(Client c, int sleepMin, int sleepMax, int t2, int reqBaase){
	int count = 0; // total cycle counter
	bool gFlag = false; // continue check

	while (!gFlag){ // only continue when there are people haven't done 20 cycles

		// REQUEST
		count = count + 1; // cycle increasement
		cof[c.getid()-'a'] << c.getid() << "'s " << count << "th cycle." /*<< count << " " << count << " " << count << " " << count << " " << count << " " << count << " " << count << " "*/<<endl;
		c.requestQ(count % 2 + reqBaase); // request certain number of resources as described in the lab intro


		// WAITING FOR RESPONSE		
		cof[c.getid()-'a'] << c.getid() << " is waiting for message from sever." << endl;
		while (c.check() == NULL){}; // waitting for server response
		cof[c.getid()-'a'] << c.getid() << " gets message from server." << endl;


		// RANDOM WAITING
		char* msg = c.read(); // get the message
		for (int i = 0; i < 10; i++){
			c.own[i] = msg[i]; // keep record of assgined resources
		}
		cof[c.getid()-'a'] << c.getid()<<"'s resources "<< c.own << endl;

		int t = rand()%sleepMax+sleepMin; // generate random waiting rime
		cof[c.getid()-'a'] << "Random delay of "<<c.getid() << endl;
		sleep(t); // wait
		cof[c.getid()-'a'] << "Random delay of " << c.getid() << " ends." <<endl;


		// RELEASE ALL THE RESOURCES
		for (int i = 0; i<10; i++){	
			if (c.own[i] != '0'){ // for resource that is owned
				c.requestL(c.own[i]); // request release
				c.own[i] = '0';  // mark released.
			}
		};


		// FIXED WAITING
		cof[c.getid()-'a'] << "Fixed delay of " << c.getid() << endl;
		sleep(t2); // wating
		cof[c.getid()-'a'] << "Fixed delay of " << c.getid() << " ends."<<endl;


		// update our own completion flag
		flags[c.getid()-'a'] = (count >20); 

		// update overall completion flag
		gFlag = flags[0] && flags[1] && flags[2] && flags[3];;
	}

}



// major method
int main(void){

	Client a = Client('a'), b = Client('b'), c = Client('c'), d = Client('d'); // generate clients
	Server s = Server();	// generate erver
	thread st, at, bt, ct, dt; // prepare thread

	// prepare logging channels
	sof.open("Server.log");
	cof[0].open("a_log.log");
	cof[1].open("b_log.log");
	cof[2].open("c_log.log");
	cof[3].open("d_log.log");

	// generate all the threads
	st = thread(serverProc, s);
	at = thread(clientProc, a, 500 / SPEED, 5000 / SPEED, 2000 / SPEED, 2);
	bt = thread(clientProc, b, 500 / SPEED, 5000 / SPEED, 2000 / SPEED, 2);
	ct = thread(clientProc, c, 1000 / SPEED, 3000 / SPEED, 3000 / SPEED, 3);
	dt = thread(clientProc, d, 1000 / SPEED, 4000 / SPEED, 3000 / SPEED, 3);

	sleep(15);

	// join all threads
	st.join();
	at.join();
	bt.join();
	ct.join();
	dt.join();

	// mark completion of program
	cout << "ends" << endl;

	// close all logging stream
	sof.close();
	for (int i = 0; i < 4; i++){
		cof[i].close();
	}

	// give user a short response time.
	sleep(15);
	exit(0);
	return 0;
}
