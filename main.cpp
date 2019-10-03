#ifdef _WIN32
//Windows includes
#include "curses.h"
#include "panel.h"
#include "curspriv.h"
#else
//Linux / MacOS includes
#include <curses.h>
#endif
#include <string>
#include <sstream>

#define PDC_DLL_BUILD 1
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
using namespace std;

//void print_vect_from_pos(WINDOW* win, int page_num, int win_cursy, int win_cursx, vector<string> source_vect);

int main(int argc, char* argv[])
{
	initscr();
	start_color();
	noecho();
	keypad(stdscr, TRUE);

	WINDOW* main_window = newwin(LINES - 3, COLS, 0, 0);
	keypad(main_window, TRUE);
	box(main_window, '|', '-');
	wrefresh(main_window);
	int main_win_cursx = 1;
	int main_win_cursy = 1;
	wmove(main_window, main_win_cursy, main_win_cursx);
	int main_win_height;
	int main_win_width;
	getmaxyx(main_window, main_win_height, main_win_width);
	main_win_height -= 2;

	WINDOW* option_bar_windows[5];
	string options[5] = { "File", "Edit", "View", "Insert", "Help" };
	for (int i = 0; i < 5; i++)
	{
		option_bar_windows[i] = newwin(3, 10, LINES - 3, (10 * i));
		keypad(option_bar_windows[i], TRUE);
		box(option_bar_windows[i], '|', '-');

		mvwprintw(option_bar_windows[i], 1, 2, options[i].c_str());
		wrefresh(option_bar_windows[i]);
	}

	vector<string> lines_vect;
	bool overFlow = false;
	int page = 0;

	if (argc == 2)
	{
		ifstream infile;
		infile.open(argv[1]);
		if (infile.is_open())
		{
			string output;

			main_win_cursx = 1;
			main_win_cursy = 1;
			while (getline(infile, output))
				lines_vect.push_back(output);				// add each line in text file to lines_vect

			for (int i = (25 * page); i < lines_vect.size(); i++)		// print each string in source_vect to win according to page number
				mvwprintw(main_window, main_win_cursy++, main_win_cursx, lines_vect[i].c_str());
			/*print_vect_from_pos(main_window, page, main_win_cursy, main_win_cursx, lines_vect);*/

			if (lines_vect.size() >= ((LINES - 5) * (page + 1)))			// if text is larger than window size, overflow = true
				overFlow = true;

			main_win_cursy = 1;
			wmove(main_window, main_win_cursy, main_win_cursx);
		}
		infile.close();
	}

	WINDOW* page_indicator_win = newwin(3, 9, LINES - 3, COLS - 9);
	mvwprintw(page_indicator_win, 1, 2, "pg. %d", page + 1);
	box(page_indicator_win, '|', '-');
	wrefresh(page_indicator_win);
	
	vector<char> main_win_lines_vect;
	bool stillRunning = true;
	while (stillRunning)
	{
		int user_input = wgetch(main_window);

		switch (user_input)
		{
		case '0':
			stillRunning = false;				// sentinel: breaks out of program
			break;
		case KEY_RIGHT:
			if (main_win_cursx < COLS - 2)		// moves cursor right if it's not already all the way to the right of the window
			{
				wmove(main_window, main_win_cursy, ++main_win_cursx);
				wrefresh(main_window);
			}
			break;
		case KEY_LEFT:
			if (main_win_cursx > 1)			// moves cursor left if it's not already all the way to the left
			{
				wmove(main_window, main_win_cursy, --main_win_cursx);
				wrefresh(main_window);
			}
			break;
		case KEY_DOWN:
			if (main_win_cursy < main_win_height)		// moves cursor down if it's not already all the way at the bottom
			{
				wmove(main_window, ++main_win_cursy, main_win_cursx);
				wrefresh(main_window);
			}
			else if (main_win_cursy == (LINES - 5))
			{
				if (overFlow == true)
				{
					main_win_cursy = 1;
					main_win_cursx = 1;
					wmove(main_window, main_win_cursy, main_win_cursx);
					wclrtobot(main_window);
					box(main_window, '|', '-');
					page++;

					/*print_vect_from_pos(main_window, ++page, main_win_cursy, main_win_cursx, lines_vect);*/
					for (int i = (25 * page); i < lines_vect.size(); i++)
					{
						mvwprintw(main_window, main_win_cursy++, main_win_cursx, lines_vect[i].c_str());

						/*if (i == lines_vect.size() - 3)*/
						if (i == (LINES - 6) + (25 * page))
							break;
					}
					main_win_cursy = 1;
					main_win_cursx = 1;
					wmove(main_window, main_win_cursy, main_win_cursx);
				}
			}
			break;
		case KEY_UP:
			if (main_win_cursy > 1)			// moves cursor up if it's not already all the way at the top
			{
				wmove(main_window, --main_win_cursy, main_win_cursx);
				wrefresh(main_window);
			}
			else if (main_win_cursy == 1)
			{
				if ((overFlow == true) and (page != 0))
				{
					main_win_cursy = 1;
					main_win_cursx = 1;
					wmove(main_window, main_win_cursy, main_win_cursx);
					wclrtobot(main_window);
					box(main_window, '|', '-');
					page--;

					for (int i = (25 * page); i < lines_vect.size(); i++)
					{
						mvwprintw(main_window, main_win_cursy++, main_win_cursx, lines_vect[i].c_str());

						/*if (i == lines_vect.size() - 3)*/
						if (i == (LINES - 6) + (25 * page))
							break;
					}
					main_win_cursy = main_win_height;
					main_win_cursx = 1;
					wmove(main_window, main_win_cursy, main_win_cursx);
				}
			}
			break;
		case KEY_SDOWN:		// displays "open file" box in bottom right
			WINDOW* temp_window = newwin(3, 30, 27, 50);
			keypad(temp_window, TRUE);
			box(temp_window, '|', '-');
			wrefresh(temp_window);

			int temp_win_height;
			int temp_win_width;
			getmaxyx(temp_window, temp_win_height, temp_win_width);
			int temp_win_cursx = 1;
			int temp_win_cursy = 1;
			mvwprintw(temp_window, temp_win_cursy, temp_win_cursx, "Open file: ");
			int temp_win_vect_index = 0;

			vector<char> file_name;

			temp_win_cursx += 11;
			int c = mvwgetch(temp_window, temp_win_cursy, temp_win_cursx);
			while (c != KEY_SUP)
			{
				if ((c == KEY_DC) and (temp_win_cursx > 12))
				{
					mvwdelch(temp_window, temp_win_cursy, --temp_win_cursx);
					mvwdelch(temp_window, temp_win_cursy, temp_win_width - 2);
					box(temp_window, '|', '-');
					wmove(temp_window, temp_win_cursy, temp_win_cursx);
					wrefresh(temp_window);
					temp_win_vect_index--;
					file_name.erase(file_name.begin() + temp_win_vect_index);
				}
				else if (c == KEY_LEFT)
				{
					if (temp_win_cursx > 12)			// moves cursor left if it's not already all the way to the left
					{
						wmove(temp_window, temp_win_cursy, --temp_win_cursx);
						wrefresh(temp_window);
						temp_win_vect_index--;
					}
				}
				else if (c == KEY_RIGHT)
				{
					if (temp_win_cursx < (temp_win_width - 1))		// moves cursor right if it's not already all the way to the right of the window
					{
						wmove(temp_window, temp_win_cursy, ++temp_win_cursx);
						wrefresh(temp_window);
						temp_win_vect_index++;
					}
				}
				else
				{
					mvwaddch(temp_window, temp_win_cursy, temp_win_cursx++, c);
					file_name.push_back(c);
					temp_win_vect_index++;
				}
				c = wgetch(temp_window);
			}

			string sfile_name(file_name.begin(), file_name.end());		// make file_name vector into string sfile_name

			werase(temp_window);
			wrefresh(temp_window);
			delwin(temp_window);

			ifstream infile;
			infile.open(sfile_name);
			if (infile.is_open())
			{
				string output;

				main_win_cursx = 1;
				main_win_cursy = 1;
				wmove(main_window, main_win_cursy, main_win_cursx);
				wclrtobot(main_window);
				box(main_window, '|', '-');
				wrefresh(main_window);
				while (getline(infile, output))
					lines_vect.push_back(output);				// add each line in text file to lines_vect

				for (int i = (25 * page); i < (25 * page) + 25; i++)	// print each string in source_vect to win according to page number
				{
					if (lines_vect.size() == i)
						break;
					mvwprintw(main_window, main_win_cursy++, main_win_cursx, lines_vect[i].c_str());
				}
				
				wrefresh(main_window);
				/*print_vect_from_pos(main_window, page, main_win_cursy, main_win_cursx, lines_vect);*/

				if (lines_vect.size() >= ((LINES - 5) * (page + 1)))
					overFlow = true;

				main_win_cursy = 1;
				wmove(main_window, main_win_cursy, main_win_cursx);
				wrefresh(main_window);
			}
			infile.close();
			break;
		}
		/*main_win_lines_vect.push_back(user_input);
		for (int i = 0; i < main_win_lines_vect.size(); i++)
		{
			mvwprintw(main_window, main_win_cursy, main_win_cursx++, "%d", user_input);
		}*/
		if ((user_input != '0') and (user_input != KEY_RIGHT) and (user_input != KEY_LEFT) and 
			(user_input != KEY_UP) and (user_input != KEY_DOWN) and (user_input != KEY_SDOWN))
			mvwaddch(main_window, main_win_cursy, main_win_cursx++, user_input);
	}
	wgetch(main_window);
	endwin();
	return 0;
}

//void print_vect_from_pos(WINDOW* win, int page_num, int win_cursy, int win_cursx, vector<string> source_vect)
//{
//	for (int i = (25 * page_num); i < ((LINES - 5) * (page_num + 1)); i++)	// print each string in source_vect to win according to page number
//		mvwprintw(win, win_cursy++, win_cursx, source_vect[i].c_str());
//}