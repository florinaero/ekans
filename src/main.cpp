#include <iostream>
#include <chrono>
#include <thread>
#include <curses.h>
#include "snake.hpp"

using namespace std;

int main(){
	try{
		// Create snake object with wait time in mills and initial direction
		Snake snake(70, RIGHT, 1);
	}
	catch(WrongSizeException& wre){
		return 1;
	}
	return 0;
}