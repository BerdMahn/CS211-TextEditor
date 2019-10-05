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
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include "curses.h"
#include "panel.h"
using namespace std;

// takes a window, windows y- and x-cursor, the "page" to display (offset of height of window), vector of lines
void print_vect_page(WINDOW* win, int cursor_y, int cursor_x, int page_num, vector<vector<string>> pages_vect);

vector<char> push_buff_elements();
void make_new_page(vector<vector<string>>& current_pages, int &total);
void set_overflow(bool& overflow, int num_of_pages);
string chvect_to_str(vector<char> char_vector);
void flush_buffer(vector<char>& buffer_vect);

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

	vector<char> buffer = push_buff_elements();			// has 25 positions
	int buffer_index = 0;
	vector<string> lines;			// each has 25 positions
	int lines_index = 0;
	vector<vector<string>> pages;	// will be added as needed
	int page_index = 0;
	int total_num_pages = 0;

	make_new_page(pages, total_num_pages);

	vector<string> main_win_lines_vect;
	int main_win_lines_vect_index = 0;

	string open_file;
	bool overFlow = false;

	if (argc == 2)
	{
		ifstream infile;
		infile.open(argv[1]);
		if (infile.good() == true)
		{
			string output;
			open_file = argv[1];

			main_win_cursx = 1;
			main_win_cursy = 1;

			page_index = 0;
			lines_index = 0;
			while (getline(infile, output))
			{
				pages[page_index][lines_index++] = output;				// add each line in text file to lines_vect
				if (lines_index == 25)
				{
					lines_index = 0;
					make_new_page(pages, total_num_pages);
					page_index++;
				}
			}

			set_overflow(overFlow, total_num_pages);
		}
		infile.close();

		main_win_cursy = 1;
		main_win_cursx = 1;
		page_index = 0;
		lines_index = 0;
		print_vect_page(main_window, main_win_cursy, main_win_cursx, page_index, pages);
		wrefresh(main_window);
	}

	bool stillRunning = true;
	while (stillRunning)
	{
		WINDOW* page_indicator_win = newwin(3, 9, LINES - 3, COLS - 9);
		mvwprintw(page_indicator_win, 1, 2, "pg. %d", page_index + 1);
		box(page_indicator_win, '|', '-');
		wrefresh(page_indicator_win);

		int user_input = wgetch(main_window);

		switch (user_input)
		{
		case '0':
		{
			// sentinel: breaks out of program
			stillRunning = false;
			break;
		}
		case KEY_SDOWN:		// displays "open file" box in bottom right
		{
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

			temp_win_cursx += 11;
			int c = mvwgetch(temp_window, temp_win_cursy, temp_win_cursx);

			vector<char> file_name;
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
			open_file = sfile_name;

			werase(temp_window);
			wrefresh(temp_window);
			delwin(temp_window);

			ifstream infile;
			infile.open(sfile_name);
			if (infile.good() == true)
			{
				string output;

				main_win_cursx = 1;
				main_win_cursy = 1;
				wmove(main_window, main_win_cursy, main_win_cursx);
				wclrtobot(main_window);
				box(main_window, '|', '-');
				wrefresh(main_window);

				page_index = 0;
				lines_index = 0;
				while (getline(infile, output))
				{
					pages[page_index][lines_index++] = output;				// add each line in text file to lines_vect
					if (lines_index == 25)
					{
						lines_index = 0;
						make_new_page(pages, total_num_pages);
						page_index++;
					}
				}

				page_index = 0;
				lines_index = 0;
				print_vect_page(main_window, main_win_cursy, main_win_cursx, page_index, pages);

				wrefresh(main_window);

				set_overflow(overFlow, total_num_pages);

				main_win_cursy = 1;
				wmove(main_window, main_win_cursy, main_win_cursx);
				wrefresh(main_window);
			}
			infile.close();
			break;
		}
		case KEY_SUP:
		{
			make_new_page(pages, total_num_pages);
			set_overflow(overFlow, total_num_pages);
			break;
		}
		case ALT_9:
		{
			ofstream outfile;
			outfile.open(open_file);
			if (outfile.good() == true)
			{
				outfile.clear();
				page_index = 0;
				lines_index = 0;

				for (int i = 0; i < 25; i++)
				{
					outfile << pages[page_index][i] << endl;
					if (i == 24)
					{
						lines_index = 0;
						page_index++;
					}
				}
			}
			outfile.close();
			break;
		}
		case KEY_DC:
		{
			if (main_win_cursx > 1)
			{
				mvwdelch(main_window, main_win_cursy, --main_win_cursx);
				mvwdelch(main_window, main_win_cursy, main_win_width - 2);
				box(main_window, '|', '-');
				wmove(main_window, main_win_cursy, --main_win_cursx);
				wrefresh(main_window);
				pages[page_index][lines_index].erase(pages[page_index][lines_index].begin() + buffer_index);
				buffer_index--;
			}
		}
		case KEY_RIGHT:
		{
			// moves cursor right if it's not already all the way to the right of the window
			if (main_win_cursx < COLS - 2)
			{
				wmove(main_window, main_win_cursy, ++main_win_cursx);
				wrefresh(main_window);
				buffer_index++;
				break;
			}
		}
		case KEY_LEFT:
		{
			// moves cursor left if it's not already all the way to the left
			if (main_win_cursx > 1)
			{
				wmove(main_window, main_win_cursy, --main_win_cursx);
				wrefresh(main_window);
				buffer_index--;
			}
			break;
		}
		case KEY_DOWN:
		{
			// moves cursor down if it's not already all the way at the bottom, else if it is and overflow is true, then scroll
			if (main_win_cursy < main_win_height)
			{
				wmove(main_window, ++main_win_cursy, main_win_cursx);
				main_win_lines_vect_index++;
				wrefresh(main_window);
				lines_index++;
				flush_buffer(buffer);
			}
			else if (main_win_cursy == (LINES - 5))
			{
				if (overFlow)
				{


					main_win_cursy = 1;
					main_win_cursx = 1;
					wmove(main_window, main_win_cursy, main_win_cursx);
					wclrtobot(main_window);
					box(main_window, '|', '-');

					print_vect_page(main_window, main_win_cursy, main_win_cursx, ++page_index, pages);

					main_win_cursy = 1;
					main_win_cursx = 1;
					lines_index = 0;
					flush_buffer(buffer);
					buffer_index = 0;
					wmove(main_window, main_win_cursy, main_win_cursx);

					main_win_lines_vect_index++;
				}
			}
			break;
		}
		case KEY_UP:
		{
			// moves cursor up if it's not already all the way at the top
			if (main_win_cursy > 1)
			{
				wmove(main_window, --main_win_cursy, main_win_cursx);
				wrefresh(main_window);
				lines_index--;
				flush_buffer(buffer);
			}
			else if (main_win_cursy == 1)
			{
				if (overFlow and (page_index != 0))
				{
					main_win_cursy = 1;
					main_win_cursx = 1;
					wmove(main_window, main_win_cursy, main_win_cursx);
					wclrtobot(main_window);
					box(main_window, '|', '-');
					page_index--;

					print_vect_page(main_window, main_win_cursy, main_win_cursx, page_index, pages);

					main_win_cursy = main_win_height;
					main_win_cursx = 1;
					lines_index = 24;
					flush_buffer(buffer);
					buffer_index = 0;
					wmove(main_window, main_win_cursy, main_win_cursx);
				}
			}
			break;
		}
		}
		

		if ((user_input != '0') and (user_input != KEY_RIGHT) and (user_input != KEY_LEFT) and
			(user_input != KEY_UP) and (user_input != KEY_DOWN) and (user_input != KEY_SDOWN) and 
			(user_input != KEY_SUP) and (user_input != ALT_9) and (user_input != KEY_DC))
		{
			mvwaddch(main_window, main_win_cursy, main_win_cursx++, user_input);
			buffer[buffer_index++] = user_input;
			string this_line = chvect_to_str(buffer);
			pages[page_index][lines_index] += this_line;
		}
	}

	endwin();
	return 0;
}

void print_vect_page(WINDOW* win, int cursor_y, int cursor_x, int page_num, vector<vector<string>> pages_vect)
{
	for (int i = 0; i < 25; i++)	// print each string in source_vect to win according to page number
	{
		mvwprintw(win, cursor_y++, cursor_x, pages_vect[page_num][i].c_str());
	}
}

vector<char> push_buff_elements()
{
	vector<char> line_vect;
	for (int i = 0; i < 118; i++)
	{
		line_vect.push_back(' ');
	}
	return line_vect;
}

void make_new_page(vector<vector<string>>& current_pages, int &total)
{
	vector<string> new_lines;
	for (int i = 0; i < 25; i++)
	{
		new_lines.push_back("");
	}
	current_pages.push_back(new_lines);
	total++;
}

void set_overflow(bool &overflow, int num_of_pages)
{
	if (num_of_pages > 0)
		overflow = true;
	else
		overflow = false;
}

string chvect_to_str(vector<char> char_vector)
{
	string string_line(char_vector.begin(), char_vector.end());
	return string_line;
}

void flush_buffer(vector<char>& buffer_vect)
{
	buffer_vect.clear();
	buffer_vect = push_buff_elements();
}
