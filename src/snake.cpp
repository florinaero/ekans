#define DEBUG 0

#include <iostream>
#include <chrono>
#include <thread>
// #include <curses.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "snake.hpp"
#include <random>
#include <fstream>

/*
todo: Implement collision with edge.
todo: Reduce size of run() function.
Solved: todo: Remove fixed dimmension of snake_symbol_
Solved: todo: Define window size from begining
todo: Check why arrows are not working(keypad())
Solved: todo: Implement smooth transition for direction change
Solved: bug: Wrong transition at end of screen's right
		-> not observed on ubuntu
Solved: bug: When sense is changed on same direction then the direction's
		change is happening at the opposite head.
Solved: bug: Direction change one after another makes symbols to dissapear. 
Solved bug: Snake dissapears on edge of window if direction is changed
			-> Changed conditions and values for goX(); not considered 
				to subtract edges from playing window 

*/
using namespace std;

// Coordinates constructor
Snake::Coord::Coord(int x_coord, int y_coord):
	x_coord(x_coord),
	y_coord(y_coord)
	{}

Snake::Snake(int waitTimeMills, wchar_t head_position, bool edge_hit_enable):
head_position_(head_position),
width_(0),
height_(0),
wait_time_mills_(waitTimeMills),
uptr_win_(nullptr, [](WINDOW*) {}), 
log_file_("log_snake.txt", ios_base::out),
edge_hit_flag_(false),
edge_hit_enable_(edge_hit_enable)
{
	// Init screen for ncurses
	intitScreen();
	// Define coordinates of asnake on vertical
	for(size_t i=0;i<born_size_;i++){
		// Start from middle of window
		snake_coord_.push_back(Coord(WindowWidth/2, WindowHeight/2+i));
	} 

	// Start running snake
	run();
}

void Snake::intitScreen(){
	// Initialize screen 
	initscr();
	// Get size of terminal window
	getScreenSize();
	// Check size of terminal and create window
	CreateWindow();
	// Disables buffer and makes chars to immediately appear on screen 
	cbreak();
	// Disable printing input on screen 
	noecho();
	// getch() not waits for input, it returns ERR if key is not ready
	nodelay(uptr_win_.get(),true);
	// notimeout(uptr_win_.get(),true);
	// Enable special keys in order to use arrows; todo: is not working 
	// keypad(stdscr, true);
	// Hide cursor
	curs_set(0);
}

void Snake::clearScreen(){
	// Return to original window
	endwin();
}

void Snake::getScreenSize(){
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	height_ = w.ws_row;
	width_ = w.ws_col;
	// TODO: use for debug
	// height_ = 70;
	// width_ = 70;	
	// cout << "width: "<<width_ << "  height: \n" << height_;
}

//TODO: Reduce function size to 40 lines
void Snake::run(){
	wchar_t old_direction = 0;
	bool change_flag = false;
	Coord food_coord = getFoodCoord();
	int score = 0;
	// Exit with STOP_KEY
	while(head_position_!=STOP_KEY){
		// Check if no key or wrong key was pressed, then keep previous command
		if(head_position_==ERR || (head_position_!=UP && head_position_!=DOWN && 
										head_position_!=RIGHT && head_position_!=LEFT)){
			head_position_ = old_direction;
		}

		// Generate new coordinates for food if head reached it
		if(checkFoodReached(food_coord)){
			food_coord = getFoodCoord();
			increaseSize(); // Increase snake's size
			score++;	//Increase score
		}

		ShowScore(score);
		// Place food symbol on screen 
		moveChar(food_coord.y_coord, food_coord.x_coord, food_symbol_);
		wrefresh(uptr_win_.get());

		// Check for collision with body or edges
		if(checkCollisionBody()){
			break;
		}
		// Flag that checks if chosen direction is backwards
		change_flag = false;
		switch(head_position_){
			case UP: 
				if(old_direction!=DOWN){
					goUp();
					change_flag = true;
				}
				break;
			case DOWN:
				if(old_direction!=UP){
					goDown();
					change_flag = true;
				}
				break;
			case LEFT: 
				if(old_direction!=RIGHT){
					goLeft();
					change_flag = true;
				}
				break;
			case RIGHT: 
				if(old_direction!=LEFT){
					goRight();
					change_flag = true;
				}
				break;
			default: // Do nothing
				break;
		}
		if(edge_hit_flag_&&edge_hit_enable_){
			FlashSnake(3);
			break;
		}
		if(change_flag){
			// Store direction that was selected from keyboard
			old_direction = head_position_;
		}	
		// Get direction from input
		head_position_ = wgetch(uptr_win_.get());
	}
	// Return to original screen
	clearScreen();
}

void Snake::refreshAndWait(){
	box(uptr_win_.get(),'*','*');
	// Refresh screen 
	wrefresh(uptr_win_.get());
	// Wait time which gives snake's pace
	this_thread::sleep_for(chrono::milliseconds(wait_time_mills_));
	// Clear screen
	wclear(uptr_win_.get());
}

void Snake::moveChar(int yCoord, int xCoord, char symbol){
	mvwaddch(uptr_win_.get(), yCoord, xCoord, symbol);
}

void Snake::updateCoord(int new_xcoord, int new_ycoord){
	// Create a copy of head at the end of deque
	snake_coord_.emplace_back(snake_coord_.back());
	// Change y coordinate of the new head
	snake_coord_.back().y_coord = new_ycoord;
	// Change x coordinate of the new head
	snake_coord_.back().x_coord = new_xcoord;
	// Remove snake's tail
	snake_coord_.pop_front();

	wchar_t symbol = 'x';
	// Update coordinates of symbols by changing head's coord and copying coordinates
	//	of symbols from tail to head, follow head.
	for(auto it=snake_coord_.rbegin();it!=snake_coord_.rend();++it){
		// Choose symbols of snake according to deque coordinates sequence
		if(it==snake_coord_.rbegin()){
			symbol = SnakeSymbols::head;
		}
		else{
			symbol = SnakeSymbols::body;
		}
		moveChar(it->y_coord, it->x_coord, symbol); 

	}
	refreshAndWait();
}

void Snake::goDown(){
	int new_ycoord = 0;
	// Update head position
	head_position_ = DOWN;

	new_ycoord = snake_coord_.back().y_coord;
	new_ycoord++;
	// Check not to get out of screen
	if(new_ycoord>=WindowHeight-1){
		new_ycoord = 1;
		edge_hit_flag_ = true;
	}
	// Check if collision is active and snake hit the edge
	if(!(edge_hit_flag_ && edge_hit_enable_)){
		updateCoord(snake_coord_.back().x_coord, new_ycoord);
	}
}

void Snake::goUp(){
	int new_ycoord = 0;
	// Update head position
	head_position_ = UP;

	new_ycoord = snake_coord_.back().y_coord;
	new_ycoord--;
	if(new_ycoord<1){
		// Subtract 2 cols due to box dimenssion
		new_ycoord = WindowHeight - 2;
		edge_hit_flag_ = true;
	}
	if(!(edge_hit_flag_ && edge_hit_enable_)){
		updateCoord(snake_coord_.back().x_coord, new_ycoord);
	}
}

void Snake::goLeft(){
	int new_xcoord = 0;
	// Update head position
	head_position_ = LEFT;

	new_xcoord = snake_coord_.back().x_coord;
	new_xcoord--;
	if(new_xcoord<1){		
		// Subtract 2 cols due to box dimenssion
		new_xcoord = WindowWidth - 2;
		edge_hit_flag_ = true;
	}
	if(!(edge_hit_flag_ && edge_hit_enable_)){
		updateCoord(new_xcoord, snake_coord_.back().y_coord);
	}
}

void Snake::goRight(){
	int new_xcoord = 0;
	// Update head position
	head_position_ = RIGHT;

	new_xcoord = snake_coord_.back().x_coord;
	new_xcoord++;
	if(new_xcoord>=WindowWidth-1){
		new_xcoord = 1;
		edge_hit_flag_ = true;
	}
	if(!(edge_hit_flag_ && edge_hit_enable_)){
		updateCoord(new_xcoord, snake_coord_.back().y_coord);
	}
}

Snake::Coord Snake::getFoodCoord(){
	Coord food_coord(0,0);

	random_device rd;  //Will be used to obtain a seed for the random number engine
    mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    // Avoid placing food symbol on window edges by adding and 
    //	subtracting 2 beacause there are 2 edges of box
    uniform_int_distribution<> dis_x(1, WindowWidth-2);
    uniform_int_distribution<> dis_y(1, WindowHeight-2);
    
    food_coord.x_coord = dis_x(gen);
    food_coord.y_coord = dis_y(gen);
    log_file_ << "food_coord.x_coord = " << food_coord.x_coord << endl;
    log_file_ << "food_coord.y_coord = " << food_coord.y_coord << endl;
    log_file_ << "food coord end" << endl;
    return food_coord;
}

bool Snake::checkFoodReached(const Coord& food_coord){
	// Compare coord of head with food coord
	return (snake_coord_.back() == food_coord);
}

void Snake::increaseSize(){
	// Create a copy of head to increase size of snake
	snake_coord_.emplace_back(snake_coord_.back());
	// Define position of newest head by checking actual direction 
	switch(head_position_){
		case UP:
			snake_coord_.back().y_coord--;
			break;
		case DOWN:
			snake_coord_.back().y_coord++;
			break;
		case LEFT: 
			snake_coord_.back().x_coord--;
			break;
		case RIGHT:
			snake_coord_.back().x_coord++;
			break;
		default:	// Do nothing
			break; 
	}
}

// Check if head hits body by checking duplicate in list
bool Snake::checkCollisionBody(){
	auto head = snake_coord_.back();
	// Check each element against last elem(head) 
	for(size_t i=0;i<snake_coord_.size()-1;i++){
		if(head==snake_coord_.at(i)){
			FlashSnake(3);
			return true;	
		}
	}
	return false;
}

// Check if head hits edges
bool Snake::checkCollisionEdge(){
	auto head = snake_coord_.back();
	log_file_ << "w_w = " << WindowWidth << " w_h = " << WindowHeight << endl;
	log_file_ << "head.x = " << head.x_coord << "  head.y = " << head.y_coord << endl;
	if(head.x_coord==1 || head.x_coord==WindowWidth-2 ||
		head.y_coord==1 || head.y_coord==WindowHeight-2){
		FlashSnake(3);
		return true;
	}
return false;
}

// Flash image of snake on screen, used for collisions
void Snake::FlashSnake(int no_flashes){
	// Wait time in milliseconds
	const int wait_time = 300; 
	for(int i=0;i<no_flashes;i++){
		// Clear screen
		wclear(uptr_win_.get());
		box(uptr_win_.get(),'*','*');
		wrefresh(uptr_win_.get());
		this_thread::sleep_for(chrono::milliseconds(wait_time));
		for(auto & it : snake_coord_){
			moveChar(it.y_coord, it.x_coord, SnakeSymbols::body);
		}
		// Refresh screen 
		wrefresh(uptr_win_.get());
		this_thread::sleep_for(chrono::milliseconds(wait_time));
	}
}

void Snake::CreateWindow(){
	// Check if size of terminal is enough for window
	if((width_<WindowWidth) || (height_<WindowHeight)){
		endwin();
		cout << "\n\nTerminal size must be larger than " << WindowWidth 
			<< "x" << WindowHeight << "!"<< endl;
		cout << "Actual size is " << width_ << "x" << height_ << ".\n\n" << endl;
		throw WrongSizeException();
	}
	
	// Initialize coord. in order to center window in respect to terminal size
	win_coord_.width_start = (width_ - WindowWidth)/2;
	win_coord_.width_end = win_coord_.width_start + WindowWidth;
	// Add offset to leave space for upper text
	win_coord_.height_start = (height_ - WindowHeight)/2+1;
	win_coord_.height_end = win_coord_.height_start + WindowHeight;
	
	log_file_ <<"hello snake addicts!\n";
	log_file_ << "width_ = " << width_ << endl;
	log_file_ << "height_ = " << height_ << endl;
	log_file_ << "win_coord_.width_start = " << win_coord_.width_start << endl;
	log_file_ << "win_coord_.width_end = " << win_coord_.width_end << endl;
	log_file_ << "win_coord_.height_start = " << win_coord_.height_start << endl;
	log_file_ << "win_coord_.height_end = " << win_coord_.height_end << endl;
	log_file_ << "w_w = " << WindowWidth << "w_h = " << WindowHeight << endl;

	// Pointer to WIDNOW struct with game's window coordinates
	uptr_win_.reset(newwin(WindowHeight, WindowWidth, win_coord_.height_start, win_coord_.width_start));

	box(uptr_win_.get(),'*','*');
}

// Compute score and display it on screen(stdscr)
bool Snake::ShowScore(int score){
	string score_text = "score: " + to_string(score);
	const char* ptr_score = score_text.c_str();
	mvaddstr(0,0,ptr_score);
	refresh();
}