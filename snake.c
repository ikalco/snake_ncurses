#include "snake.h"

int screen_width;
int screen_height;

WINDOW* snake_win;
int snake_win_width = 60;
int snake_win_height = 30;

enum Dir {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct Snake {
	struct Snake *next;
	enum Dir dir;
	enum Dir prev_dir;
	int x;
	int y;
};

struct Snake *snake;
int apple_x, apple_y;

void init_ncurses() {
	// initialize ncurses
	initscr();						// initialize ncurses
	raw();							// make all kb input go to program, by not buffering until \n
	noecho();						// don't print chars typed by user

	keypad(stdscr, TRUE);					// enable reading of special keys (mainly arrow keys for us)
	halfdelay(5);						// wait a five tenths of a sec for keypress otherwise continue
	curs_set(0);						// make the cursor invisible

	getmaxyx(stdscr, screen_height, screen_width);		// get row and col size of ncurses screen

	// Create snake window, extra two is for border
	snake_win = newwin(
			snake_win_height,
			snake_win_width,
			(screen_height-snake_win_height)/2,
			(screen_width-snake_win_width)/2
	);

	// weird bug where you can't draw on entire window
	snake_win_height -= 2;
	snake_win_width -= 2;
}

void init_snake() {
	snake = malloc(sizeof(struct Snake));
	snake->next = NULL;
	snake->dir = RIGHT;
	snake->prev_dir = RIGHT;
	snake->x = 0;
	snake->y = 0;
}

void init_apple() {
	apple_x = snake_win_width - 1;
	apple_y = snake_win_height - 1;
}

void draw_instructions() {
	mvprintw((screen_height - 5)/2 + 0, 5, "Press 'q' to quit.");
	mvprintw((screen_height - 5)/2 + 1, 5, "Press up arrow to move snake up.");
	mvprintw((screen_height - 5)/2 + 2, 5, "Press down arrow to move snake down.");
	mvprintw((screen_height - 5)/2 + 3, 5, "Press left arrow to move snake left.");
	mvprintw((screen_height - 5)/2 + 4, 5, "Press right arrow to move snake right.");
}

void draw_title(char title[]) {
	mvprintw(1, (screen_width - strlen(title))/ 2, title);
}

void init()
{
	init_ncurses();	
	draw_instructions();
	draw_title("Snake");
	refresh(); // draw to 'screen'

	init_snake();
	init_apple();
}

void free_snake(struct Snake *s) {
	if (s->next != NULL) {
		free_snake(s->next);
	}
	
	free(s);
}

void cleanup() {
	free_snake(snake);

	nocbreak();				// disable halfdelay
	delwin(snake_win);
	endwin();
}

enum Dir opposite_dir(enum Dir dir) {
	switch (dir) {
		case RIGHT: return LEFT;
		case LEFT: return RIGHT;
		case UP: return DOWN;
		case DOWN: return UP;
		default: return -1;
	}
}

void move_snake_next(struct Snake *s) {
	switch (s->dir) {
		case UP:
			s->y += -1;
			break;
		case DOWN:
			s->y += 1;
			break;
		case LEFT:
			s->x += -1;
			break;
		case RIGHT:
			s->x += 1;
			break;
	}
}

void grow_snake() {
	struct Snake *last = snake;
	
	while(last->next != NULL) {
		last = last->next;
	}
	
	struct Snake *new = malloc(sizeof(struct Snake));

	new->next = NULL;
	new->x = last->x;
	new->y = last->y;
	new->dir = opposite_dir(last->dir);
	new->prev_dir = new->dir;

	move_snake_next(new);

	new->dir = last->dir;
	new->prev_dir = new->dir;

	last->next = new;
}

void draw_snake() {
	bool first = true;

	mvprintw(1, 1, "x: %d | y: %d | dir: %d", snake->x, snake->y, snake->dir);
	
	// draw body
	for (struct Snake* curr = snake; curr != NULL; curr = curr->next) {
		if (first) {
			mvwprintw(snake_win, curr->y+1, curr->x+1, "0");
			first = false;
		} else {
			mvwprintw(snake_win, curr->y+1, curr->x+1, "#");
		}
	}
}

void move_snake() {
	for (struct Snake* curr = snake; curr != NULL; curr = curr->next) {
		if (curr->next != NULL) {
			curr->next->dir = curr->prev_dir;
		}

		curr->prev_dir = curr->dir;

		move_snake_next(curr);
	}
}

bool snake_has_collided() {
	if (snake->x < 0 ||
	    snake->y < 0 ||
	    snake->x >= snake_win_width ||
	    snake->y >= snake_win_height
	) {
		return true;
	}

	for (struct Snake* curr = snake->next; curr != NULL; curr = curr->next) {
		if (curr->x == snake->x && curr->y == snake->y) return true;
	}

	return false;
}

void check_has_eaten() {
	if (snake->x != apple_x || snake->y != apple_y) return;

	apple_x = rand() % snake_win_width;
	apple_y = rand() % snake_win_height;

	grow_snake();
}

void loop()
{
	int ch;
	bool quit = false;

	while (!quit && (ch = getch()))
	{
		switch (ch)
		{
			case 'q':	quit = true;        break;
			case KEY_LEFT:	snake->dir = LEFT;  break;
			case KEY_RIGHT:	snake->dir = RIGHT; break;
			case KEY_UP:	snake->dir = UP;    break;
			case KEY_DOWN:	snake->dir = DOWN;  break;
		}

		// move the snake, check for colllisions 
		move_snake();
		quit |= snake_has_collided();
		
		// check for apple eatings
		check_has_eaten();
		
		// clear, draw border, draw snake, draw apple, draw to window
		wclear(snake_win);
		box(snake_win, 0, 0);
		draw_snake();
		wrefresh(snake_win);
	}
}

int main()
{
	init();
	loop();
	cleanup();

	return 0;
}
