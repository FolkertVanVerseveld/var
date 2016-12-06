#include "view.h"
#include <stdio.h>
#if VISUAL
#include <ncurses.h>
#include <string.h>

static WINDOW *scr = NULL;
static int row, col;
static unsigned w, h;
static size_t fpos = 0;

static void reshape(void)
{
	getmaxyx(stdscr, row, col);
	w = col > 0 ? col : 0;
	h = row > 0 ? row : 0;
	clear();
	mvaddstr(0, w - strlen("hello world"), "hello world");
	refresh();
}

static int vmain(void)
{
	int key;
	reshape();
	while ((key = getch()) != 'q') {
		switch (key) {
		case KEY_RESIZE:
			reshape();
			break;
		}
	}
	return 0;
}

#endif

int visual(void)
{
#if VISUAL
	scr = initscr();
	if (!scr) {
		perror("no visual");
		return 1;
	}
	cbreak();
	noecho();
	clear();
	int ret = vmain();
	delwin(scr);
	endwin();
	return ret;
#else
	fputs("not available\n", stderr);
	return 1;
#endif
}
