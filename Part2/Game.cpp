#include "Game.hpp"

#define NUM_OF_NEIGHBORS 8
#define RESURRECTION_NUMBER 3
#define MINIMUM_LIFE_SUSTAINING_NEIGHBORS 2
#define NUM_OF_COLORS 7

void PrintBoard(const GameBoard &board);


static const char *colors[7] = {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};
/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
Game::Game(game_params params) {
    m_thread_num = params.n_thread;
    m_gen_num = params.n_gen;
    file_name = params.filename;
    interactive_on = params.interactive_on;
    print_on = params.print_on;
}

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
    current_board = new GameBoard();
    next_board = new GameBoard();
    int work_lines;
    vector<string> all_lines = utils::read_lines(file_name);
            /* building first board from the file */
    for(auto line = all_lines.begin(); line != all_lines.end(); line++) {
        vector<int> int_vec;
        vector<string> splited_line = utils::split(*(line),' ');
        for(auto character = splited_line.begin(); character != splited_line.end(); character++){
            int num = std::stoi(*character);
            int_vec.push_back(num);
        }
        current_board->push_back(int_vec);
    }
    vector<int> zeroes(current_board[0].size(), 0);
    *next_board = vector<vector<int>>(current_board->size(), zeroes);
    m_thread_num = m_thread_num < current_board->size() ? m_thread_num : current_board->size();
    work_lines = current_board->size() / m_thread_num;
    num_of_work_lines = work_lines;
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
//	for (uint i = 0; i < m_thread_num; ++i) {
//        m_threadpool[i]->join();
//    }
    free(current_board);
    free(next_board);
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
		    PrintBoard(*current_board);

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
        GameOfLifeTask task = tasks.front();
        tasks.pop();
        task.preformTask();
    }
    ///TODO: Wait for it to finish
    swapBoards();
    distributeTasks(GameOfLifeTask::TaskType::DecideSpecies);
    for(int i=0; i<thread_num();i++){
        GameOfLifeTask task = tasks.front();
        tasks.pop();
        task.preformTask();
    }
}

void Game::distributeTasks(Game::GameOfLifeTask::TaskType type) {
    int start = 0;
    int end = num_of_work_lines;
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

uint Game::thread_num() const {
    return m_thread_num;
}

/* Function sketch to use for printing the board. You will need to decide its placement and how exactly
	to bring in the field's parameters. */
void PrintBoard(const GameBoard &board) {
    cout << u8"╔" << string(u8"═") * board[0].size() << u8"╗" << endl;
    for (uint i = 0; i < board.size() ;++i) {
        cout << u8"║";
        for (uint j = 0; j < board[0].size(); ++j) {
            if (board[i][j] > 0)
                cout << colors[board[i][j] % 7] << u8"█" << RESET;
            else
                cout << u8"░";
        }
        cout << u8"║" << endl;
    }
    cout << u8"╚" << string(u8"═") * board[0].size() << u8"╝" << endl;
}


/*--------------------------------------------------------------------------------
					Class GameOfLife Task Methods and Implementation
--------------------------------------------------------------------------------*/

Game::GameOfLifeTask::GameOfLifeTask(TaskType type, int start, int end, GameBoard& source, GameBoard& target):
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
            for(int k = -1; k <= 1; k++){
                for(int t = -1; t <= 1; t++) {
                    if (t == 0 && k == 0) continue;
                    if(i+k < 0 || j+t < 0 || i+k >= source.size() || j+t >= source[0].size()) continue;
                    int value = source[i + k][j + t];
                    if(value < 1 && value > 7) {
                        assert(false);
                    }
                    if (value != 0) {
                        num_of_alive_neighbors++;
                    }
                    histogram[value]++;


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
            if(source[i][j] == 0) {
                target[i][j] = 0;
                continue;
            }
            int num_of_alive_neighbors = 0;
            int sum = 0;
            for(int k = -1; k <= 1; k++){
                for(int t = -1; t <= 1; t++) {
                    if (i + k < 0 || j + t < 0 || i + k >= source.size() || j + t >= source[0].size()) continue;
                    int value = source[i + k][j + t];
                    if (value != 0) {
                        num_of_alive_neighbors++;
                    }
                    sum += value;
                }
            }
            int new_species = (int)round((double)sum/(double)num_of_alive_neighbors);
            target[i][j] = new_species;
        }
    }
}



