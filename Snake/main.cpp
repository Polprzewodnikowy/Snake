/*
 * File: main.cpp
 * Author: Polprzewodnikowy
 * Date: 09.01.2014
 */

#include <list>
#include <ctime>
#include <fstream>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define DIR_LEFT	1
#define DIR_UP		2
#define DIR_RIGHT	3
#define DIR_DOWN	4

using namespace std;
using namespace sf;

typedef struct {
	float x, y;
} pos;

void game_pause(int sc);
void game_over(int sc);
void game_reset(void);
int random(int min, int max);
pos roll_fruit(list<pos> l);
bool collision(list<pos> l, pos h);

RenderWindow window(VideoMode(750, 600, 32), "Snake", Style::Close);
Texture head_t, tail_t, fruit_t, background_t;
Sprite head, tail, fruit, background;
Font score_f;
Text score_t;
Event ev;
Clock timer;
Music bgmusic;

list<pos> snake;
int snake_dir = DIR_RIGHT;
int snake_cur_dir = DIR_RIGHT;
pos head_pos = {10, 5};
pos fruit_pos = {0, 0};
int score = 0;
string score_s;
ifstream config;
int level_ms = 150;

int main(void)
{
#ifdef _DEBUG
	cout << "Game Started" << endl;
#endif

	srand(time(NULL));

	window.setFramerateLimit(60);

	config.open("../config.ini");
	if(config.good())
		config >> level_ms;
	config.close();

	head_pos.x = 9;
	snake.push_back(head_pos);
	head_pos.x = 8;
	snake.push_back(head_pos);
	head_pos.x = 10;
	snake.push_front(head_pos);

	head_t.loadFromFile("../data/head.png");
	tail_t.loadFromFile("../data/tail.png");
	fruit_t.loadFromFile("../data/fruit.png");
	background_t.loadFromFile("../data/background.png");
	score_f.loadFromFile("../data/verdanab.ttf");
	bgmusic.openFromFile("../data/music.ogg");

	head.setTexture(head_t);
	tail.setTexture(tail_t);
	fruit.setTexture(fruit_t);
	background.setTexture(background_t);
	score_t.setFont(score_f);
	score_t.setCharacterSize(20);
	score_t.setStyle(Text::Regular);
	score_t.setColor(Color(255, 255, 255, 200));
	bgmusic.setLoop(true);
	bgmusic.play();

	fruit_pos = roll_fruit(snake);
	score_t.setString("Score: 0");

	while(window.isOpen())
	{
		while(window.pollEvent(ev))
		{
			switch(ev.type)
			{
				case Event::Closed:
					window.close();
					break;
				case Event::KeyPressed:
					switch(ev.key.code)
					{
						case Keyboard::Up:
							if(snake_cur_dir != DIR_DOWN)
								snake_dir = DIR_UP;
							break;
						case Keyboard::Down:
							if(snake_cur_dir != DIR_UP)
								snake_dir = DIR_DOWN;
							break;
						case Keyboard::Left:
							if(snake_cur_dir != DIR_RIGHT)
								snake_dir = DIR_LEFT;
							break;
						case Keyboard::Right:
							if(snake_cur_dir != DIR_LEFT)
								snake_dir = DIR_RIGHT;
							break;
						case Keyboard::Escape:
							game_pause(score);
#ifdef _DEBUG
							cout << "Game Resumed" << endl;
#endif
							break;
					}
					break;
			}
		}

		if(timer.getElapsedTime().asMilliseconds() >= level_ms)
		{
			timer.restart();
			snake.pop_back();
			switch(snake_dir)
			{
				case DIR_UP:
					--head_pos.y;
					if(head_pos.y < 0)
						head_pos.y = 39;
					break;
				case DIR_DOWN:
					++head_pos.y;
					if(head_pos.y > 39)
						head_pos.y = 0;
					break;
				case DIR_LEFT:
					--head_pos.x;
					if(head_pos.x < 0)
						head_pos.x = 49;
					break;
				case DIR_RIGHT:
					++head_pos.x;
					if(head_pos.x > 49)
						head_pos.x = 0;
					break;
			}
			snake_cur_dir = snake_dir;
			if(collision(snake, head_pos))
				game_over(score);
			snake.push_front(head_pos);
		}

		if(head_pos.x == fruit_pos.x && head_pos.y == fruit_pos.y)
		{
#ifdef _DEBUG
			cout << "Fruit Consumed at: X: " << head_pos.x << " Y: " << head_pos.y << endl;
#endif
			fruit_pos = roll_fruit(snake);
			score_s = "Score: " + to_string(++score);
			score_t.setString(score_s);
			snake.push_back(head_pos);
		}

		window.draw(background);

		for(list<pos>::iterator i = snake.begin(); i != snake.end(); i++)
		{
			tail.setPosition((*i).x * 15, (*i).y * 15);
			window.draw(tail);
		}

		head.setPosition(head_pos.x * 15, head_pos.y * 15);
		window.draw(head);

		fruit.setPosition(fruit_pos.x * 15, fruit_pos.y * 15);
		window.draw(fruit);

		score_t.setPosition(10, 10);
		window.draw(score_t);

		window.display();
	}

	return 0;
}

void game_pause(int sc)
{
	Text ps_t;
	Text sl_t;

	ps_t.setFont(score_f);
	ps_t.setCharacterSize(60);
	ps_t.setStyle(Text::Regular);
	ps_t.setColor(Color(255, 255, 255, 255));

	sl_t.setFont(score_f);
	sl_t.setCharacterSize(40);
	sl_t.setStyle(Text::Regular);
	sl_t.setColor(Color(255, 255, 255, 255));

	ps_t.setString("Game Paused");
	sl_t.setString("Any Key -> Resume\nBackspace -> End Game");

#ifdef _DEBUG
	cout << "Game Paused" << endl;
#endif

	while(window.isOpen())
	{
		while(window.pollEvent(ev))
		{
			switch(ev.type)
			{
				case Event::Closed:
					window.close();
					break;
				case Event::KeyPressed:
					if(ev.key.code == Keyboard::BackSpace)
						game_over(sc);
					return;
					break;
			}
		}

		window.draw(background);
		
		ps_t.setPosition(40, 20 + 360);
		window.draw(ps_t);

		sl_t.setPosition(40, 100 + 360);
		window.draw(sl_t);

		window.display();
	}
}

void game_over(int sc)
{
	Text ov_t;
	Text sc_t;
	Text sl_t;

	ov_t.setFont(score_f);
	ov_t.setCharacterSize(60);
	ov_t.setStyle(Text::Regular);
	ov_t.setColor(Color(255, 255, 255, 255));

	sc_t.setFont(score_f);
	sc_t.setCharacterSize(50);
	sc_t.setStyle(Text::Regular);
	sc_t.setColor(Color(255, 255, 255, 255));

	sl_t.setFont(score_f);
	sl_t.setCharacterSize(40);
	sl_t.setStyle(Text::Regular);
	sl_t.setColor(Color(255, 255, 255, 255));

	ov_t.setString("Game Over");
	sc_t.setString("Score: " + to_string(sc));
	sl_t.setString("Enter -> New Game\nEscape -> Quit");

#ifdef _DEBUG
	cout << "Game Over" << endl;
#endif

	while(window.isOpen())
	{
		while(window.pollEvent(ev))
		{
			switch(ev.type)
			{
				case Event::Closed:
					window.close();
					break;
				case Event::KeyPressed:
					if(ev.key.code == Keyboard::Escape)
						window.close();
					if(ev.key.code == Keyboard::Return)
					{
						game_reset();
						return;
					}
					break;
			}
		}

		window.draw(background);
		
		ov_t.setPosition(40, 20 + 290);
		window.draw(ov_t);

		sc_t.setPosition(40, 100 + 290);
		window.draw(sc_t);

		sl_t.setPosition(40, 170 + 290);
		window.draw(sl_t);

		window.display();
	}
}

void game_reset(void)
{
	score = 0;
	snake_dir = DIR_RIGHT;
	snake_cur_dir = DIR_RIGHT;
	snake.clear();
	head_pos.y = 5;
	head_pos.x = 9;
	snake.push_back(head_pos);
	head_pos.x = 8;
	snake.push_back(head_pos);
	head_pos.x = 10;
	snake.push_front(head_pos);
	fruit_pos = roll_fruit(snake);
	score_t.setString("Score: 0");
	timer.restart();

#ifdef _DEBUG
	cout << "Game Restarted" << endl;
#endif
}

int random(int min, int max)
{
	int tmp;

	if(max >= min)
	{
		max -= min;
	}else{
		tmp = min - max;
		min = max;
		max = tmp;
	}

	return max ? (rand() % max + min) : min;
}

pos roll_fruit(list<pos> l)
{
	bool col;
	pos p;

	do{
		col = false;
		p.x = random(0, 49);
		p.y = random(0, 39);
		for(list<pos>::iterator i = l.begin(); i != l.end(); i++)
		{
			if(((*i).x == p.x) && ((*i).y == p.y))
				col = true;
		}
	}while(col);

#ifdef _DEBUG
	cout << "Fruit rolled at: X: " << p.x << " Y: " << p.y << endl;
#endif

	return p;
}

bool collision(list<pos> l, pos h)
{
	for(list<pos>::iterator i = l.begin(); i != l.end(); i++)
	{
		if(((*i).x == h.x) && ((*i).y == h.y))
		{

#ifdef _DEBUG
			cout << "Collision at: X: " << h.x << " Y: " << h.y << endl;
#endif

			return true;
		}
	}

	return false;
}