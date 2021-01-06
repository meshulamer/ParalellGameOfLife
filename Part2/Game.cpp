#include "Game.hpp"
#define NUM_OF_NEIGHBORS 8
#define RESURRECTION_NUMBER 3
#define MINIMUM_LIFE_SUSTAINING_NEIGHBORS 2
#define NUM_OF_COLORS 7

static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((double)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(nullptr);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {
	// Create game fields - Consider using utils:read_file, utils::split
	// Create & Start threads
	// Testing of your implementation will presume all threads are started here
}

void Game::_step(uint curr_gen) {
	// Push jobs to queue
    distributeWork();
	// Wait for the workers to finish calculating
	///TODO: wait
	// Swap pointers between current and next field
	swapBoards();
	// NOTE: Threads must not be started here - doing so will lead to a heavy penalty in your grade 
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// All threads must be joined here
	for (uint i = 0; i < m_thread_num; ++i) {
        m_threadpool[i]->join();
    }
}

/*--------------------------------------------------------------------------------
					Game Methods And Implementation
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on)
			system("clear");

		// Print small header if needed
		if (header != nullptr)
			cout << "<------------" << header << "------------>" << endl;
		
		// TODO: Print the board 

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}

void Game::swapBoards() {
    GameBoard* temp = current_board;
    current_board = next_board;
    next_board = temp;

}

void Game::distributeWork() {
    distributeTasks(GameOfLifeTask::TaskType::AdvanceGeneration);
    for(int i=0; i<thread_num();i++){
        GameOfLifeTask task = tasks.pop();
        task.preformTask();
    }
    ///TODO: Wait for it to finish
    swapBoards();
    distributeTasks(GameOfLifeTask::TaskType::DecideSpecies);
    for(int i=0; i<thread_num();i++){
        GameOfLifeTask task = tasks.pop();
        task.preformTask();
    }
    swapBoards();
}

void Game::distributeTasks(Game::GameOfLifeTask::TaskType type) {
    int start = 0;
    int end = num_of_work_lines - 1;
    for(int i=0; i < thread_num() -1; i++){ //Jobs for Balanced WorkThreads
        GameOfLifeTask new_task(type, start, end,
                *current_board, *next_board);
        tasks.push(new_task);
        start += num_of_work_lines;
        end += num_of_work_lines;
    }
    GameOfLifeTask new_task(type, start, current_board->size(), *current_board, *next_board);
    tasks.push(new_task);
}

/* Function sketch to use for printing the board. You will need to decide its placement and how exactly 
	to bring in the field's parameters. 

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
                if (field[i][j] > 0)
                    cout << colors[field[i][j] % 7] << u8"█" << RESET;
                else
                    cout << u8"░";
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/


/*--------------------------------------------------------------------------------
					Class GameOfLife Task Methods and Implementation
--------------------------------------------------------------------------------*/

Game::GameOfLifeTask::GameOfLifeTask(TaskType type, int start, int end, GameBoard& target, GameBoard& source):
type(type), start_index(start), end_index(end), source(source), target(target){}

void Game::GameOfLifeTask::preformTask() {
    if(type == TaskType::AdvanceGeneration){
        advanceGeneration();
    }
    else{
        decideSpecies();
    }
}

void Game::GameOfLifeTask::advanceGeneration() {
    for(int i = start_index; i < end_index; i++){
        for(int j = 0; j < source[0].size(); j++){
            int num_of_alive_neighbors = 0;
            vector<int> histogram = vector<int>(NUM_OF_COLORS + 1, 0);
            for(int k = -1; k < 1; k++){
                for(int t = -1; t < 1; t++){
                    if(t==k==0) continue;
                    try {
                        int value = source[i + k][j + t];
                        if (value != 0) {
                            num_of_alive_neighbors++;
                        }
                        histogram[value]++;
                    }
                    catch(...){
                        ///We are out of bounds, and so the cell is dead. do nothing
                    }
                }
            }
            if(source[i][j] == 0){
                if(num_of_alive_neighbors == RESURRECTION_NUMBER){
                    int dominant_species = 0;
                    int dominant_score = 0;
                    for(int color = 1; color < NUM_OF_COLORS; color++){
                        int current_score = histogram[color]*color;
                        if(dominant_score < current_score){
                            dominant_species = color;
                            dominant_score = current_score;
                        }
                    }
                    target[i][j] = dominant_species;
                }
                else {
                    target[i][j] = 0;
                }
            }
            else if(num_of_alive_neighbors == MINIMUM_LIFE_SUSTAINING_NEIGHBORS ||
                    num_of_alive_neighbors == RESURRECTION_NUMBER){
                target[i][j] = source[i][j];
            }
            else{
                target[i][j] = 0;
            }
        }
    }
}

void Game::GameOfLifeTask::decideSpecies() {
    for(int i = start_index; i < end_index; i++){
        for(int j = 0; j < source[0].size(); j++){
            int num_of_alive_neighbors = 0;
            int sum = 0;
            for(int k = -1; k < 1; k++){
                for(int t = -1; t < 1; t++){
                    try {
                        int value = source[i + k][j + t];
                        if (value != 0) {
                            num_of_alive_neighbors++;
                        }
                        sum += source[i + k][j + t];
                    }
                    catch(...){
                        ///We are out of bounds, and so the cell is dead. do nothing
                    }
                }
            }
            int new_species = (int)round((double)sum/num_of_alive_neighbors);
            target[i][j] = new_species;
        }
    }
}


