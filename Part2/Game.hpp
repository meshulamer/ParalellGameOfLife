#ifndef __GAMERUN_H
#define __GAMERUN_H

#include "../Part1/Headers.hpp"
#include "Thread.hpp"

/*--------------------------------------------------------------------------------
								  Species colors
--------------------------------------------------------------------------------*/
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black - 7 */
#define RED     "\033[31m"      /* Red - 1*/
#define GREEN   "\033[32m"      /* Green - 2*/
#define YELLOW  "\033[33m"      /* Yellow - 3*/
#define BLUE    "\033[34m"      /* Blue - 4*/
#define MAGENTA "\033[35m"      /* Magenta - 5*/
#define CYAN    "\033[36m"      /* Cyan - 6*/


/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
typedef vector<vector<int>> GameBoard;

struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint n_gen;
    uint n_thread;
    string filename;
    bool interactive_on;
    bool print_on;
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {
public:

	Game(game_params);
	~Game();
	void run(); // Runs the game
	const vector<double> gen_hist() const; // Returns the generation timing histogram  
	const vector<double> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)


protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game(); 
	inline void print_board(const char* header);
	void distributeWork();
	void swapBoards();


    GameBoard* current_board;
    GameBoard* next_board;

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	int num_of_work_lines;
	vector<double> m_tile_hist; 	 // Shared Timing history for tiles: First (2 * m_gen_num) cells are the calculation durations for tiles in generation 1 and so on. 
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<double> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool. 

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)
	string filename;

	class GameOfLifeTask{
    public:
	    typedef enum{ AdvanceGeneration, DecideSpecies} TaskType;
	    GameOfLifeTask(TaskType type, int start, int end, GameBoard& target, GameBoard& source);
	    ~GameOfLifeTask() = default;
	    void preformTask();
	    GameOfLifeTask(const GameOfLifeTask& other) = default;
	private:
	    TaskType type;
	    int start_index, end_index;
	    const GameBoard& source;
	    GameBoard& target;
	    void advanceGeneration();
	    void decideSpecies();
	};

    void distributeTasks(GameOfLifeTask::TaskType type);

	//PCQueue<GameOfLifeTask> tasks;
    queue<GameOfLifeTask> tasks;
};
#endif
