#include "cpoint.h"
#include "screen.h"

#include <ctime>
#include <ctype.h>
#include <list>
#include <string>

using namespace std;

class CView {
  protected:
	CRect geom;

  public:
	CView(CRect g) : geom(g){};
	virtual void paint() = 0;
	virtual bool handleEvent(int key) = 0;
	virtual void move(const CPoint& delta)
	{
		geom.topleft += delta;
	};
	virtual ~CView(){};
};

class CWindow : public CView {
  protected:
	char c;

  public:
	CWindow(CRect r, char _c = '*') : CView(r), c(_c){};
	void paint()
	{
		for (int i = geom.topleft.y; i < geom.topleft.y + geom.size.y; i++) {
			gotoyx(i, geom.topleft.x);
			for (int j = 0; j < geom.size.x; j++)
				printw("%c", c);
		};
	};
	bool handleEvent(int key)
	{
		switch (key) {
		case KEY_UP:
			move(CPoint(0, -1));
			return true;
		case KEY_DOWN:
			move(CPoint(0, 1));
			return true;
		case KEY_RIGHT:
			move(CPoint(1, 0));
			return true;
		case KEY_LEFT:
			move(CPoint(-1, 0));
			return true;
		};
		return false;
	};
};

class CFramedWindow : public CWindow {
  public:
	CFramedWindow(CRect r, char _c = '\'') : CWindow(r, _c){};
	void paint()
	{
		for (int i = geom.topleft.y; i < geom.topleft.y + geom.size.y; i++) {
			gotoyx(i, geom.topleft.x);
			if ((i == geom.topleft.y) ||
				(i == geom.topleft.y + geom.size.y - 1)) {
				printw("+");
				for (int j = 1; j < geom.size.x - 1; j++)
					printw("-");
				printw("+");
			}
			else {
				printw("|");
				for (int j = 1; j < geom.size.x - 1; j++)
					printw("%c", c);
				printw("|");
			}
		}
	};
};

class CInputLine : public CFramedWindow {
	string text;

  public:
	CInputLine(CRect r, char _c = ',') : CFramedWindow(r, _c){};
	void paint()
	{
		CFramedWindow::paint();
		gotoyx(geom.topleft.y + 1, geom.topleft.x + 1);

		for (unsigned j = 1, i = 0;
			 (j + 1 < (unsigned)geom.size.x) && (i < text.length()); j++, i++)
			printw("%c", text[i]);
	};
	bool handleEvent(int c)
	{
		if (CFramedWindow::handleEvent(c))
			return true;
		if ((c == KEY_DC) || (c == KEY_BACKSPACE)) {
			if (text.length() > 0) {
				text.erase(text.length() - 1);
				return true;
			};
		}
		if ((c > 255) || (c < 0))
			return false;
		if (!isalnum(c) && (c != ' '))
			return false;
		text.push_back(c);
		return true;
	}
};

class CGroup : public CView {
	list<CView*> children;

  public:
	CGroup(CRect g) : CView(g){};
	void paint()
	{
		for (list<CView*>::iterator i = children.begin(); i != children.end();
			 i++)
			(*i)->paint();
	};
	bool handleEvent(int key)
	{
		if (!children.empty() && children.back()->handleEvent(key))
			return true;
		if (key == '\t') {
			if (!children.empty()) {
				children.push_front(children.back());
				children.pop_back();
			};
			return true;
		}
		return false;
	};
	void insert(CView* v)
	{
		children.push_back(v);
	};
	~CGroup()
	{
		for (list<CView*>::iterator i = children.begin(); i != children.end();
			 i++)
			delete (*i);
	};
};

int wait = 1600;

class CDesktop : public CGroup {
  public:
	CDesktop() : CGroup(CRect())
	{
		int y, x;
		init_screen();
		getscreensize(y, x);
		geom.size.x = x;
		geom.size.y = y;
	};
	~CDesktop()
	{
		done_screen();
	};

	void paint()
	{
		for (int i = geom.topleft.y; i < geom.topleft.y + geom.size.y; i++) {
			gotoyx(i, geom.topleft.x);
			for (int j = 0; j < geom.size.x; j++)
				printw(".");
		};
		CGroup::paint();
	}

	int getEvent()
	{
		return ngetch();
	};

	void run()
	{
		int c;
		paint();
		refresh();
		napms(4000);
		paint();
		refresh();
		int last = 0;
		int count = 0;
		bool pause = false;
		while (1) {
			if (count == 0) {
				timeout(wait);
			}
			else {
				pause = true;
			}
			c = getEvent();
			if (c == ERR) {
				if (count == 0) {
					if (handleEvent(last)) {
						paint();
						refresh();
						continue;
					}
				}
				else {
					continue;
				}
			}
			if (c == 'p' || c == 'h') {
				count++;
				if (count == 2) {
					count = 0;
					pause = false;
				}
			}
			if(count == 1 && c == 'r') {
				count = 0;
				pause = false;
			}
			if (c != 'p' && c != 'h' && c != 'r' && pause == false) {
				last = c;
			}
			if (c == 27) // ESC
				break;
			if (handleEvent(c)) {
				paint();
				refresh();
			};
		};
	};
};

const int width = 43;
const int height = 18;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

class CSnake : public CFramedWindow {
  private:
	int x, y, fruitX, fruitY, score, level;
	Direction dir;
	bool tutorial;
	bool gameOver;
	bool pause;
	bool help;
	bool restart;
	int x_coords[height * width];
	int y_coords[height * width];
	bool move;

  public:
	CSnake(CRect r, char _c = ' ') : CFramedWindow(r, _c)
	{
		x = width / 2;
		y = height / 2;
		fruitX = rand() % width;
		fruitY = rand() % height;
		score = 0;
		level = 1;
		dir = STOP;
		tutorial = true;
		gameOver = false;
		pause = false;
		restart = false;
		help = false;
		x_coords[0] = x;
		y_coords[0] = y;
		move = false;
	}

	void printTutorial()
	{
		gotoyx(geom.topleft.y + 2, geom.topleft.x + 16);
		printw(">>> SNAKE <<<");
		gotoyx(geom.topleft.y + 4, geom.topleft.x + 3);
		printw("h - toggle help information");
		gotoyx(geom.topleft.y + 5, geom.topleft.x + 3);
		printw("p - toggle pause/play mode");
		gotoyx(geom.topleft.y + 6, geom.topleft.x + 3);
		printw("r - restart game");
		gotoyx(geom.topleft.y + 7, geom.topleft.x + 3);
		printw("arrows - move snake (in play mode) or");
		gotoyx(geom.topleft.y + 8, geom.topleft.x + 12);
		printw("move window (in pause mode)");
		tutorial = false;
	}

	void printHelp()
	{
		gotoyx(geom.topleft.y + 2, geom.topleft.x + 16);
		printw(">>> HELP <<<");
		gotoyx(geom.topleft.y + 4, geom.topleft.x + 3);
		printw("h - toggle help information");
		gotoyx(geom.topleft.y + 5, geom.topleft.x + 3);
		printw("p - toggle pause/play mode");
		gotoyx(geom.topleft.y + 6, geom.topleft.x + 3);
		printw("r - restart game");
		gotoyx(geom.topleft.y + 7, geom.topleft.x + 3);
		printw("arrows - move snake (in play mode) or");
		gotoyx(geom.topleft.y + 8, geom.topleft.x + 12);
		printw("move window (in pause mode)");
	}

	void setRestart()
	{
		x = width / 2;
		y = height / 2;
		fruitX = rand() % width;
		fruitY = rand() % height;
		score = 0;
		level = 1;
		dir = STOP;
		tutorial = false;
		gameOver = false;
		pause = false;
		restart = false;
		help = false;
		x_coords[0] = x;
		y_coords[0] = y;
		move = false;
		wait = 1600;
	}

	void printGmaeOver()
	{
		gotoyx(geom.topleft.y + 2, geom.topleft.x + 13);
		printw(">>> GAME OVER <<<");
		gotoyx(geom.topleft.y + 4, geom.topleft.x + 3);
		printw("Final score: %d", score);
		gameOver = true;
	}

	void printPause()
	{
		gotoyx(geom.topleft.y + 2, geom.topleft.x + 16);
		printw(">>> PAUSE <<<");
	}

	void generateNewFruit()
	{
		while (1) {
			fruitX = rand() % width;
			fruitY = rand() % height;
			int found = 0;
			for (int i = 0; i < score; i++) {
				if (fruitX == x_coords[i] && fruitY == y_coords[i]) {
					found++;
					break;
				}
			}
			if (found == 0) {
				break;
			}
			found = 0;
		}
	}

	void setScore()
	{
		score += 1;
		if (score > 0 && score % 3 == 0) {
			level++;
			if (wait > 100) {
				wait -= 100;
			}
		}
	}

	void makeMove()
	{
		for (int i = score; i > 0; i--) {
			x_coords[i] = x_coords[i - 1];
			y_coords[i] = y_coords[i - 1];
		}
		x_coords[0] = x;
		y_coords[0] = y;
	}

	void throughTheWall()
	{
		gotoyx(geom.topleft.y + 1, geom.topleft.x);
		if (y < 0) {
			y = height - 1;
		}
		if (y >= height) {
			y = 0;
		}
		if (x < 0) {
			x = width - 1;
		}
		if (x >= width) {
			x = 0;
		}
	}
	
	void printSnake() {
		int row = 0;
		for (int i = geom.topleft.y + 1; i < geom.topleft.y + 1 + geom.size.y - 2; i++) {
			gotoyx(i, geom.topleft.x + 1);
			for (int j = 0; j < geom.size.x - 2; j++) {
				int found = 0;
				for (int k = 1; k < score + 1; k++) {
					if (row == y_coords[k] && j == x_coords[k]) {
						found++;
						printw("#");
						break;
					}
					if (y == y_coords[k] && x == x_coords[k]) {
						CFramedWindow::paint();
						printGmaeOver();
						return;
					}
				}
				if (found != 0) {
					continue;
				}
				if (row == y && j == x) {
					printw("@");
				}
				else if (row == fruitY && j == fruitX) {
					printw("O");
				}
				else {
					printw(" ");
				}
			}
			row++;
		};
	}
	
	void printScore() {
		gotoyx(geom.topleft.y - 1, geom.topleft.x);
		printw("|Level: %d | Score: %d|", level, score);
	}

	void paint()
	{
		CFramedWindow::paint();
		if (tutorial == true) {
			printTutorial();
			return;
		}
		if (help == true) {
			printHelp();
			return;
		}
		if (pause == true && help == false) {
			printPause();
			if (restart != true) {
				return;
			}
		}
		if (restart == true) {
			setRestart();
		}
		throughTheWall();
		if (x == fruitX && y == fruitY) {
			setScore();
			generateNewFruit();
		}
		if (move == true) {
			makeMove();
		}
		printSnake();
		printScore();
	};

	bool handleGameOver(int c)
	{
		switch (c) {
		case 'r':
			setRestart();
			return true;
		default:
			return false;
		}
	}

	bool handlePause(int c)
	{
		if (CWindow::handleEvent(c) == true) {
			return true;
		}
		switch (c) {
		case 'p':
			pause = false;
			move = false;
			return true;
		case 'r':
			setRestart();
			return true;
		default:
			return false;
		}
	}

	bool handleHelp(int c)
	{
		switch (c) {
		case 'h':
			help = false;
			move = false;
			return true;
		case 'r':
			setRestart();
			return true;	
		default:
			return false;
		}
	}
	
	bool handleScore(int c)
	{
		switch (c) {
		case KEY_LEFT:
			dir = LEFT;
			x--;
			move = true;
			return true;
		case KEY_RIGHT:
			dir = RIGHT;
			x++;
			move = true;
			return true;
		case KEY_UP:
			dir = UP;
			y--;
			move = true;
			return true;
		case KEY_DOWN:
			dir = DOWN;
			y++;
			move = true;
			return true;
		case 'p':
			pause = true;
			return true;
		case 'r':
			setRestart();
			return true;
		case 'h':
			help = true;
			return true;
		default:
			return false;
		}
	}
	
	bool handleOther(int c) {
		if (c == 'p') {
			pause = true;
			move = false;
			return true;
		}
		else if (c == 'r') {
			setRestart();
			move = false;
			return true;
		}
		else if (c == 'h') {
			help = true;
			move = false;
			return true;
		}
		else {
			return false;
		}
	}
	
	bool handleDirections(int c) {
		if (c == KEY_LEFT && dir != RIGHT) {
			dir = LEFT;
			x--;
			move = true;
			return true;
		}
		else if (c == KEY_RIGHT && dir != LEFT) {
			dir = RIGHT;
			x++;
			move = true;
			return true;
		}
		else if (c == KEY_UP && dir != DOWN) {
			dir = UP;
			y--;
			move = true;
			return true;
		}
		else if (c == KEY_DOWN && dir != UP) {
			dir = DOWN;
			y++;
			move = true;
			return true;
		}
		else if (c == KEY_LEFT && dir == RIGHT) {
			dir = RIGHT;
			x++;
			move = true;
			return true;
		}
		else if (c == KEY_RIGHT && dir == LEFT) {
			dir = LEFT;
			x--;
			move = true;
			return true;
		}
		else if (c == KEY_UP && dir == DOWN) {
			dir = DOWN;
			y++;
			move = true;
			return true;
		}
		else if (c == KEY_DOWN && dir == UP) {
			dir = UP;
			y--;
			move = true;
			return true;
		}
		else {
			return handleOther(c);
		}
	}

	bool handleEvent(int c)
	{
		if (gameOver == true) {
			return handleGameOver(c);
		}
		if (pause == true) {
			return handlePause(c);
		}
		if (help == true) {
			return handleHelp(c);
		}
		if (score == 0) {
			return handleScore(c);
		}
		return handleDirections(c);
	}
};

int main()
{
	srand(static_cast<unsigned int>(time(0)));
	CDesktop d;
	d.insert(new CInputLine(CRect(CPoint(5, 7), CPoint(15, 15))));
	d.insert(new CWindow(CRect(CPoint(2, 3), CPoint(20, 10)), '#'));
	d.insert(new CSnake(CRect(CPoint(25, 3), CPoint(45, 20))));
	d.run();
	return 0;
}
