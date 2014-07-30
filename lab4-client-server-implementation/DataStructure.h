#include <iostream>
#ifndef DATA
#define DATA
//#define LOCK
using std::cout;
using std::endl;


// Channel element class, should be list nodes by i just called it queue...
class Queue{
public:
	Queue* prec; // link to prevoius node
	Queue* next; // link to next node
	char* msg;   // content message which can be 4 char long (up message) or 11 char long (down message)

	// constructor
	Queue(){
		prec = NULL;
		next = NULL;
		msg = NULL;
	}
};




class Ch{
private:
	Queue* top; // notes the queue top

// debuging code, useless and is also dsiabled. wont note later.
#ifdef LOCK
	bool lock; 
#endif
public:
	// constructor
	Ch(){
		top = NULL;
#ifdef LOCK
		lock = false;
#endif
	}

	// check method, looks at the top message but never modify
	char* check(){
		if (top == NULL){
			return NULL;
		}
		return top->msg;
		// debuging code
		//cout << "Checking dump";
		// dump();
	}

	// read method, take the top message and removes it from the channel
	char* read(){
#ifdef LOCK
		cout << "check locked" << endl;
		while (lock);
		cout << "not locked" << endl;
		lock = true;
#endif

		// if the channel is empty, we must deal with properly
		if (top == NULL){
			return NULL;
		}
		Queue* tmp = top;
		top = top->next;

		if (top != NULL){
			top->prec = NULL;
		}
#ifdef LOCK
		lock = false;
#endif
		// debugging code
		// cout << "Reading dump";
		// dump();
		return tmp->msg;
	}


	// send method, appends the message at the end of the queue
	void send(char* msg, int type){
#ifdef LOCK
		cout << "check locked" << endl;
		while (lock);
		cout << "not locked" << endl;
		lock = true;
#endif

		// compose the node that is about to be appeneded
		Queue* tmp = new Queue;
		char* mssg = new char[type];
		for (int i = 0; i < type; i++){
			mssg[i] = msg[i];

		}
		tmp->msg = mssg;
		tmp->next = NULL;

		// if the channel is empty we need ot deal properly
		if (top == NULL){
			top = tmp;
		}
		else{
			// normally, we get to the end of the queue
			Queue* iter = top;
			while (iter->next != NULL){
				iter = iter->next;
			}
			// append our new node here.
			iter->next = tmp;
		}
#ifdef LOCK
		lock = false;
#endif
		// debugging function
		// cout << "Sending dump ";
		// dump();
	}


	// debugging function, that prints the channel
	void dump(){
		if (top == NULL){
			cout << "" << endl;
			return;
		}
		Queue* cur = top;
		while (cur != NULL){
			cout << cur->msg;
			cur = cur->next;
		}
		cout << endl;

	}
};
#endif