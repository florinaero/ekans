#include <vector>
#include <utility>
#include <deque>
#include <functional>
#include <curses.h>
#include <fstream>

using namespace std;

// Define keys(inline constexpr available only for c++17) 
inline constexpr wchar_t UP = 'w';
inline constexpr wchar_t DOWN = 's';
inline constexpr wchar_t LEFT = 'a';
inline constexpr wchar_t RIGHT = 'd';
inline constexpr wchar_t STOP_KEY = 'q';

// Symbols used for showing snake on screen 
struct SnakeSymbols{
	static const wchar_t head = 'o';
	static const wchar_t body = '*'; 
};

// Store coordinate of ncurses window 
struct WindowCoord{
	int width_start;
	int width_end;
	int height_start;
	int height_end;
};

class Snake{
private:
	struct Coord{
		int x_coord;
		int y_coord;
		Coord(int x_coord, int y_coord);
		inline bool operator==(const Coord& other) const{
			return ((x_coord==other.x_coord) && (y_coord==other.y_coord));
		}
	};

	wchar_t head_position_;
	int pace_;
	int width_;
	int height_;
	int wait_time_mills_;
	int WindowWidth = 60;
	int WindowHeight = 30;
	// Deleter for window declare as function
	function<void(WINDOW*)> del_win_;// = [](WINDOW* ptr_win) {delete ptr_win;};
   	unique_ptr<WINDOW, decltype(del_win_)> uptr_win_;
	WindowCoord win_coord_;
	ofstream log_file_;
	// Coordinates of snake on screen
	deque<Coord> snake_coord_;
	// Flag used to detect collision with edges
	bool edge_hit_flag_;
	// Flag that enables collision with edges
	bool edge_hit_enable_;
	static const wchar_t food_symbol_ = '$';
	static const int born_size_ = 5;

	// Intialize screen using curses
	void intitScreen();
	// Return to original screen
	void clearScreen();
	// Define start position of snake on screen 
	void defineStartPosition();
	// Get window size 
	void getScreenSize();
	// Refresh screen and wait time 
	void refreshAndWait();
	// Update position of chars on screen 
	void moveChar(int yCoord, int xCoord, char symbol);
	// Update coordinates according to the new direction 
	void updateCoord(int new_xcoord, int new_ycoord);
	// Control direction of snake, act as main function called 
	//	during construction
	void run();
	void goDown();
	void goUp();
	void goLeft();
	void goRight();
	// Place food symbol on screen using random coorinates
	Coord getFoodCoord();
	// Check if head reached food 
	bool checkFoodReached(const Coord& food_coord);
	// Increase size of snake by addsing an element to deque 
	void increaseSize();
	// Check if head hits body by looking for duplicate in list
	bool checkCollisionBody();
	// Check if head hits edges
	bool checkCollisionEdge();
	// Create window in terminal 
	void CreateWindow();
	// Flash image of snake on screen, used for collisions
	void FlashSnake(int no_flashes);
	// Calculate score and show it on stdscr
	bool ShowScore(int score);

public:
	// Ctor
	Snake(int waitTimeMills, wchar_t head_position, bool edge_hit_enable);
};

// Reset terminal in case size of initial terminal is smaller than window size
struct WrongSizeException : public exception{
	const char * what () const throw (){
		return "WrongSizeException";
	}
};