#include <curses.h>
#include <unistd.h>
#include <stdbool.h>

#define BOARD_SIZE       8
#define ARRAY_SIZE       (BOARD_SIZE + 2)

#define EMPTY_CELL       0
#define PLAYER_CELL      1
#define COMPUTER_CELL    2
#define BORDER_CELL      3

#define PLAYER           PLAYER_CELL
#define COMPUTER         COMPUTER_CELL

#define BOARDY			 3
#define BOARDX			 3

#define HUDX             (ARRAY_SIZE * 4 + 2)
#define HUDY             (BOARDY + 1)
#define HUDW            30
#define HUDH            10

#define PLAYER_COLOR		 1
#define COMPUTER_COLOR		 2
#define PLAYER_TEXT_COLOR	 3
#define COMPUTER_TEXT_COLOR	 4
#define HIGHLIGHT_COLOR		 5

// Pairs of numbers representing directions on the board (dx, dy) axes
// i.e. (0, -1) represents "down" direction (x stays the same, y decrements by 1)
// Used in check_direction and flip_direction functions
int const directions[2][8] = {
	{ 0,  0, -1, -1, -1,  1,  1,  1},
	{-1,  1, -1,  0,  1, -1,  0,  1}
};

// Global array holding the state of the game board
int game_board[ARRAY_SIZE][ARRAY_SIZE];

// Initialize curses subroutines
void init_curses() {
	initscr();				// Start curses mode
	raw();					// Line buffering disabled
	keypad(stdscr, TRUE);	// We get F1, F2 etc..
	noecho();				// Don't echo() while we do getch
	nodelay(stdscr,1);
	curs_set(0);
	nonl();
	mousemask (ALL_MOUSE_EVENTS, NULL);

	// Create color pairs
	start_color();			// Initialize color
	init_pair(COMPUTER_COLOR,      COLOR_RED,   COLOR_RED  );
	init_pair(PLAYER_COLOR,        COLOR_GREEN, COLOR_GREEN);
	init_pair(PLAYER_TEXT_COLOR,   COLOR_BLACK, COLOR_GREEN);
	init_pair(COMPUTER_TEXT_COLOR, COLOR_BLACK, COLOR_RED  );
	init_pair(HIGHLIGHT_COLOR,     COLOR_BLACK, COLOR_WHITE);
}

// Print game title
void print_game_title() {
	attron(COLOR_PAIR(HIGHLIGHT_COLOR));
	mvprintw(1, 6, "       THE GAME OF REVERSI       ");
	attroff(COLOR_PAIR(HIGHLIGHT_COLOR));
	refresh();
}

// Return score = number of player's or computer's chips on the game board
// the only valid argument values are COMPUTER and PLAYER
int get_board_score(int cell_type) {
	int score = 0;
	for (size_t y = 1; y <= BOARD_SIZE; ++y) {
		for (size_t x = 1; x <= BOARD_SIZE; ++x) {
			if (game_board[y][x] == cell_type) {
				++score;
			}
		}
	}

	return score;
}

// Print an empty game HUD
void print_hud() {
	// Print top and bottom lines
	for (size_t i = 1; i <= HUDW; ++i) {
		mvaddch(HUDY, HUDX + i, ACS_HLINE);
		mvaddch(HUDY + HUDH, HUDX + i, ACS_HLINE);
	}

	// Print side lines
	for (size_t i = 1; i <= HUDH; ++i) {
		mvaddch(HUDY + i, HUDX, ACS_VLINE);
		mvaddch(HUDY + i, HUDX + HUDW, ACS_VLINE);
	}

	// Print corners
	mvaddch(HUDY, HUDX, ACS_ULCORNER);
	mvaddch(HUDY, HUDX + HUDW, ACS_URCORNER);
	mvaddch(HUDY + HUDH, HUDX, ACS_LLCORNER);
	mvaddch(HUDY + HUDH, HUDX + HUDW, ACS_LRCORNER);

	// Print score
	attron(COLOR_PAIR(HIGHLIGHT_COLOR));
	mvprintw(HUDY + 1, HUDX + 1, "            SCORE            ");
	attroff(COLOR_PAIR(HIGHLIGHT_COLOR));
	mvprintw(HUDY + 3, HUDX + 8, "Player   :%3d", get_board_score(PLAYER));
	mvprintw(HUDY + 4, HUDX + 8, "Computer :%3d", get_board_score(COMPUTER));
	refresh();

	// Clear messages
	attron(COLOR_PAIR(HIGHLIGHT_COLOR));
	mvprintw(HUDY + 6, HUDX + 1, "           MESSAGES          ");
	attroff(COLOR_PAIR(HIGHLIGHT_COLOR));
}

// Print game board
void print_board() {

	// Print a row of cell numbers on X axis
	for (size_t i = 1; i <= BOARD_SIZE; ++i) {
		mvaddch(BOARDY, BOARDX + i * 4 + 1, i + 48);
	}

	// Print a row of cell numbers on Y axis
	for (size_t i = 1; i <= BOARD_SIZE; ++i) {
		mvaddch(BOARDY + i * 2, BOARDX + 1, i + 48);
	}

	// Print top line
	mvaddch(BOARDY + 1, BOARDX + 3, ACS_ULCORNER);
	for (size_t i = 1; i <= BOARD_SIZE; ++i) {
		mvaddch(BOARDY + 1, BOARDX + i * 4 + 0, ACS_HLINE);
		mvaddch(BOARDY + 1, BOARDX + i * 4 + 1, ACS_HLINE);
		mvaddch(BOARDY + 1, BOARDX + i * 4 + 2, ACS_HLINE);
		mvaddch(BOARDY + 1, BOARDX + i * 4 + 3, ACS_TTEE);
		if (i == BOARD_SIZE) {
			mvaddch(BOARDY + 1, BOARDX + i * 4 + 3, ACS_URCORNER);
		}
	}

	// Print empty rows
	for (size_t i = 1; i <= BOARD_SIZE; ++i) {
		mvaddch(BOARDY + 0 + i * 2, BOARDX + 3, ACS_VLINE);
		mvaddch(BOARDY + 1 + i * 2, BOARDX + 3, ACS_LTEE);
		if (i == BOARD_SIZE) {
			mvaddch(BOARDY + 1 + i * 2, BOARDX + 3, ACS_LLCORNER);
		}
		for (size_t j = 1; j <= BOARD_SIZE; ++j) {
			mvaddch(BOARDY + 0 + i * 2, BOARDX + j * 4 + 0, ' ');
			mvaddch(BOARDY + 0 + i * 2, BOARDX + j * 4 + 1, ' ');
			mvaddch(BOARDY + 0 + i * 2, BOARDX + j * 4 + 2, ' ');
			mvaddch(BOARDY + 0 + i * 2, BOARDX + j * 4 + 3, ACS_VLINE);
			mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 0, ACS_HLINE);
			mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 1, ACS_HLINE);
			mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 2, ACS_HLINE);
			mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 3, ACS_PLUS);
			if (j == BOARD_SIZE) {
				mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 3, ACS_RTEE);
			}
			if (i == BOARD_SIZE) {
				mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 3, ACS_BTEE);
			}
			if (i == BOARD_SIZE && j == BOARD_SIZE) {
				mvaddch(BOARDY + 1 + i * 2, BOARDX + j * 4 + 3, ACS_LRCORNER);
			}
		}
	}

	// Print game pieces for computer and player
	for (size_t y = 1; y <= BOARD_SIZE; ++y) {
		for (size_t x = 1; x <= BOARD_SIZE; ++x) {
			switch (game_board[y][x]) {
				case EMPTY_CELL:
					mvaddch(BOARDY + 0 + y * 2, BOARDX + x * 4 + 1, ' ');
					break;
				case COMPUTER_CELL:
					mvaddch(BOARDY + 0 + y * 2, BOARDX + x * 4 + 1, ' ' | COLOR_PAIR(COMPUTER_COLOR));
					break;
				case PLAYER_CELL:
					mvaddch(BOARDY + 0 + y * 2, BOARDX + x * 4 + 1, ' ' | COLOR_PAIR(PLAYER_COLOR));
					break;
			}
		}
	}

	// Output ncurses buffer to the screen
	refresh();
}

// Initialize game board array
void init_board() {

	// initialize the game board
	memset(game_board, EMPTY_CELL, sizeof game_board);

	// place initial game pieces for computer and player
	int init_position = (BOARD_SIZE) / 2;
	game_board[init_position    ][init_position    ] = PLAYER_CELL;
	game_board[init_position + 1][init_position    ] = COMPUTER_CELL;
	game_board[init_position    ][init_position + 1] = COMPUTER_CELL;
	game_board[init_position + 1][init_position + 1] = PLAYER_CELL;

	// create internal "borders"
	for (size_t i = 0; i < ARRAY_SIZE; ++i) {
		game_board[i][0]=BORDER_CELL;
		game_board[0][i]=BORDER_CELL;
		game_board[i][ARRAY_SIZE - 1]=BORDER_CELL;
		game_board[ARRAY_SIZE - 1][i]=BORDER_CELL;
	}
}

// Traverse direction determined by dx and dy to see if the move can flip opponent's chips
// (note: valid moves must always flip some chips)
// Returns "direction score": number of chips that can be flipped or 0 for none
int check_direction (int x, int y, int dx, int dy, int player_type) {

	bool in_sequence = false;
	int direction_score = 0;

	while (true) {
		x += dx;
		y += dy;

		if ((game_board[x][y] == EMPTY_CELL) || (game_board[x][y] == BORDER_CELL)) {
			direction_score = 0;
			break;
		}

		if (player_type == PLAYER) {
			if (game_board[x][y] == COMPUTER_CELL) {
				in_sequence = true;
				direction_score++;
			} else if ((game_board[x][y] == PLAYER_CELL) && in_sequence) {
				break;
			} else if ((game_board[x][y] == PLAYER_CELL) && !in_sequence) {
				direction_score = 0;
				break;
			}

		} else {
			if (game_board[x][y] == PLAYER_CELL) {
				in_sequence = true;
				direction_score++;
			} else if ((game_board[x][y] == COMPUTER_CELL) && in_sequence) {
				break;
			} else if ((game_board[x][y] == COMPUTER_CELL) && !in_sequence) {
				direction_score = 0;
				break;
			}
		}
	}
	return direction_score;
}

// Traverse direction determined by dx and dy and flip oponent's chips on the way
// (we already determined that this is a valid direction by calling check_direction before)
void flip_direction (int x, int y, int dx, int dy, int player_type) {

	if (player_type == PLAYER) {
		game_board[x][y] = PLAYER_CELL;
		do {
			if (game_board[x][y] == COMPUTER_CELL) {
				game_board[x][y] = PLAYER_CELL;
			}
			x += dx;
			y += dy;
		} while (game_board[x][y] != PLAYER_CELL);
	} else {
		game_board[x][y] = COMPUTER_CELL;
		do {
			if (game_board[x][y] == PLAYER_CELL) {
				game_board[x][y] = COMPUTER_CELL;
			}
			x += dx;
			y += dy;
		} while (game_board[x][y] != COMPUTER_CELL);
	}
}

// Iteratively flip oponents cells in all applicable directions
void flip_cell(int x, int y, int player_type) {
	for (size_t i = 0; i < 8; i++) {
		if (check_direction(x, y, directions[0][i], directions[1][i], player_type) > 0) {
			flip_direction(x, y, directions[0][i], directions[1][i], player_type);
		}
	}
}

// Returns the number of opponent's chips a move to this cell can flip
int cell_score (int x, int y, int player_type) {

	if (game_board[x][y] != EMPTY_CELL)
		return 0;

	int cell_score = 0;

	for (size_t i = 0; i < 8; i++) {
		cell_score += check_direction(x, y, directions[0][i], directions[1][i], player_type);
	}

	return cell_score;
}

// Returns true if moves are possible for computer or player at all
// Used to determine if a move needs to be skipped
// (If neither computer, nor player have moves available, it's game over)
bool is_move_possible(int player_type) {

	for (size_t i = 1; i <= BOARD_SIZE; i++) {
		for (size_t j = 1; j <= BOARD_SIZE; j++) {
			if (cell_score(i, j, player_type) > 0)
				return true;
		}
	}

	return false;
}

// Obtains input from player, validates it, and make a player move
void make_player_move() {
int input, movex, movey;
MEVENT mouseinput;

	do {
		// Prompt user to make a move
		attron(COLOR_PAIR(PLAYER_TEXT_COLOR));
		mvprintw(HUDY + 8, HUDX + 1, "          Your Move          ");
		attroff(COLOR_PAIR(PLAYER_TEXT_COLOR));
		refresh();

		input = getch();
		nc_getmouse (&mouseinput);
		refresh();

		if (input == KEY_MOUSE) {
			// convert mouseinput coordinates into board cells
			movex = (mouseinput.x + 1) / 4 - 1;
			movey = mouseinput.y / 2 - 1;
			// check if we clicked outside of board area
			if (movex < 1 || movey < 1 || movex > BOARD_SIZE || movey > BOARD_SIZE) {
				movex = 0; movey = 0;
			}
			// TODO: check if we clicked on a line

			refresh();

			// validate user move and if it is - make the move
			if (game_board[movey][movex] != EMPTY_CELL) {continue; }

			if (cell_score(movey, movex, PLAYER) > 0) {
				flip_cell(movey, movex, PLAYER);
				print_board();
				print_hud();
				mvprintw(HUDY + 8, HUDX + 1, "                             ");
				refresh();
				break;
			}
		}

	} while (true);
}

// Makes computer move
void make_computer_move() {

	// delay with a meaninful message in the HUD
	// this is necessary, because computer's move is too fast
	attron(COLOR_PAIR(COMPUTER_TEXT_COLOR));
	mvprintw(HUDY + 8, HUDX + 1, "        Computer Move        ");
	attroff(COLOR_PAIR(COMPUTER_TEXT_COLOR));
	refresh();
	usleep(800000);
	mvprintw(HUDY + 8, HUDX + 1, "                             ");
	refresh();

	// check every cell of the board and find one with max score
	int max_score = 0;
	int x_max = 0;
	int y_max = 0;

	for (size_t i = 1; i <= BOARD_SIZE; i++) {
		for (size_t j = 1; j <= BOARD_SIZE; j++) {
			int score = cell_score(i, j, COMPUTER);
			if (score > max_score) {
				max_score = score;
				x_max = i;
				y_max = j;
			}
		}
	}

	if (max_score > 0) {
		flip_cell(x_max, y_max, COMPUTER);
	}
}

int main() {

	int input;
	// Initialize the game and display game board and the HUD
	init_curses();
	init_board();

	// Display game title, the game board and the HUD
	print_game_title();
	print_board();
	print_hud();

	bool computer_move_possible = true;
	bool player_move_possible = true;

	while (computer_move_possible || player_move_possible) {
		print_board();
		print_hud();

		if (is_move_possible(PLAYER)) {
			make_player_move();
		} else {
			player_move_possible = false;
		}

		if (is_move_possible(COMPUTER)) {
			make_computer_move();
		} else {
			computer_move_possible = false;
		}
	}

	// Display final message
	if (get_board_score(COMPUTER) > get_board_score(PLAYER)) {
		attron(COLOR_PAIR(COMPUTER_TEXT_COLOR));
		mvprintw(HUDY + 8, HUDX + 1, "   GAME OVER: COMPUTER WON   ");
		attroff(COLOR_PAIR(COMPUTER_TEXT_COLOR));
	} else if (get_board_score(COMPUTER) < get_board_score(PLAYER)) {
		attron(COLOR_PAIR(PLAYER_TEXT_COLOR));
		mvprintw(HUDY + 8, HUDX + 1, "     GAME OVER: YOU WON      ");
		attroff(COLOR_PAIR(PLAYER_TEXT_COLOR));
	} else {
		attron(COLOR_PAIR(PLAYER_TEXT_COLOR));
		mvprintw(HUDY + 8, HUDX + 1, "    GAME OVER: IT'S A TIE    ");
		attroff(COLOR_PAIR(PLAYER_TEXT_COLOR));
	}

	attron(COLOR_PAIR(HIGHLIGHT_COLOR));
	mvprintw(HUDY + 9, HUDX + 1, "   PRESS SPACE KEY TO EXIT   ");
	attroff(COLOR_PAIR(HIGHLIGHT_COLOR));

	refresh();

	// wait for user to press Space key
	do {
		input = getch();
	} while (input != ' ');

	// Clean up and exit the program
	endwin();
	return 0;
}
