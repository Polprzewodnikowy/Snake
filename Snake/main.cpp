/*
 * File: main.cpp
 * Author: Polprzewodnikowy
 * Date: 09.01.2014
 */

#include <list>
#include <map>
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
void read_config(string fname);
int random(int min, int max);
pos roll_fruit(list<pos> l);
bool collision(list<pos> l, pos h);

RenderWindow window(VideoMode(750, 600, 32), "Snake", Style::Close);
Texture head_t, tail_t, fruit_t, background_t;
Sprite head, tail, fruit, background;
Font score_f;
Text score_t;
Event ev;
Clock timer, score_timer;
Music bgmusic;

list<pos> snake;
int snake_dir = DIR_RIGHT;
int snake_cur_dir = DIR_RIGHT;
pos head_pos = {10, 5};
pos fruit_pos = {0, 0};

int score = 0;
int level = 1;
int prev_level = 1;
map<string, float> level_cfg;

int main(void)
{
	srand(time(NULL));

	read_config("../config.ini");

	window.setFramerateLimit(60);

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
	score_t.setString("Score: " + to_string(score) + "\nLevel: " + to_string(level));
	
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
							break;
					}
					break;
			}
		}

		if(timer.getElapsedTime().asMilliseconds() >= level_cfg.at("ms_tim"))
		{
			timer.restart();
			snake.pop_back();
			switch(snake_dir)
			{
				case DIR_UP:
					--head_pos.y;
					break;
				case DIR_DOWN:
					++head_pos.y;
					break;
				case DIR_LEFT:
					--head_pos.x;
					break;
				case DIR_RIGHT:
					++head_pos.x;
					break;
			}
			snake_cur_dir = snake_dir;
			if(collision(snake, head_pos))
				game_over(score);
			else
				snake.push_front(head_pos);
		}

		if(head_pos.x == fruit_pos.x && head_pos.y == fruit_pos.y)
		{
			score += (int)(level_cfg.at("sc_div") / score_timer.getElapsedTime().asSeconds());
			score_timer.restart();

			level = score / level_cfg.at("lv_edge") + 1;
			if(level != prev_level)
			{
				if(level_cfg.at("ms_tim") > level_cfg.at("ms_min"))
					level_cfg.at("ms_tim") -= level_cfg.at("ms_dec");
				if(level_cfg.at("sc_div") > level_cfg.at("sc_min"))
					level_cfg.at("sc_div") -= level_cfg.at("sc_dec");
				prev_level = level;
			}

			fruit_pos = roll_fruit(snake);
			snake.push_back(head_pos);

			score_t.setString("Score: " + to_string(score) + "\nLevel: " + to_string(level));
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

		score_t.setPosition(15, 10);
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
	read_config("../config.ini");
	score = 0;
	level = prev_level = 1;
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
	timer.restart();
	score_timer.restart();	
	score_t.setString("Score: " + to_string(score) + "\nLevel: " + to_string(level));
}

void read_config(string fname)
{
	ifstream config(fname.c_str());
	string id, eq, val;
	if(config.good())
	{
		while(config >> id >> eq >> val)
		{
			if(id[0] == '#')
				continue;
			if(eq != "=")
				throw runtime_error("Parse error");
			level_cfg[id] = atof(val.c_str());
		}
	}
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

	do {
		col = false;
		p.x = random(0, 49);
		p.y = random(0, 39);
		for(list<pos>::iterator i = l.begin(); i != l.end(); i++)
			if(((*i).x == p.x) && ((*i).y == p.y))
				col = true;
	} while(col);

	return p;
}

bool collision(list<pos> l, pos h)
{
	if(h.x < 0 || h.x > 49)
		return true;
	if(h.y < 0 || h.y > 39)
		return true;
		
	for(list<pos>::iterator i = l.begin(); i != l.end(); i++)
		if(((*i).x == h.x) && ((*i).y == h.y))
			return true;

	return false;
}