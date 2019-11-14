-#ifdef _WIN32
-//Windows includes
-#include "curses.h"
-#include "panel.h"
-#include "curspriv.h"
-#else
-//Linux / MacOS includes
-#include <curses.h>
-#endif
-#include <string>
-#include <sstream>

#define PDC_DLL_BUILD 1
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <stack>
#include <queue>
#include "Trie.h"
#include "curses.h"
#include "panel.h"
#include "PairComparer.hpp"
using namespace std;

// takes a window, windows y- and x-cursor, the "page" to display (offset of height of window), vector of lines
void print_vect_page(WINDOW* win, int cursor_y, int cursor_x, int page_num, vector<vector<string>> pages_vect);
vector<char> push_buff_elements();
void make_new_page(vector<vector<string>>& current_pages, int& total);
void set_overflow(bool& overflow, int num_of_pages);
string chvect_to_str(vector<char> char_vector);
void flush_buffer(vector<char>& buffer_vect);
void copy_vector(vector<char>& vect1, vector<char>& vect2);
WINDOW* make_suggestions_win(int suggestions_win_height, int suggestions_win_width, int cursy, int cursx);
pair<vector<string>, int> show_word_suggestions(WINDOW* suggestions_win, int suggs_win_height, Trie& keyword_trie, const string& word, int highlight_flag);
void populate_trie(Trie& trie_tree);
void close_win(WINDOW* win_to_close);
void update_main_vector(char update_input, vector<char>& buffer, int& buffer_index, vector<vector<string>>& main_pages, int page_index, int lines_index);
string dec_to_bin(int convert_num);
unordered_map<string, int> getFreq(string file_name);
priority_queue< pair<string, int>, vector<pair<string, int>>, MaxHeapPairComparer > makeMaxHeap(unordered_map<string, int> UM);
unordered_map<string, string> getBinRep(priority_queue<pair<string, int>, vector<pair<string, int>>, MaxHeapPairComparer> aMaxPQ);
template<class T, class K>
void printUM(unordered_map<T, K> UM);

int main(int argc, char* argv[])
{
	Trie kywds_trie;
	populate_trie(kywds_trie);	// populate from "keywords.txt"

	initscr();
	start_color();
	noecho();
	keypad(stdscr, TRUE);

	// create the main interactive window
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

	// create options bar window
	const int NUM_OF_OPTS = 5;
	WINDOW* option_bar_windows[NUM_OF_OPTS];
	string options[NUM_OF_OPTS] = { "File", "Edit", "View", "Insert", "Help" };
	for (int i = 0; i < NUM_OF_OPTS; i++)
	{
		option_bar_windows[i] = newwin(3, 10, LINES - 3, (10 * i));
		keypad(option_bar_windows[i], TRUE);
		box(option_bar_windows[i], '|', '-');

		mvwprintw(option_bar_windows[i], 1, 2, options[i].c_str());
		wrefresh(option_bar_windows[i]);
	}


	vector<char> buffer = push_buff_elements();			// vector of 25 char spaces, keeps track of current line being typed
	int buffer_index = 0;
	int lines_index = 0;				// keeps track of position in a line of a page
	vector<vector<string>> pages;		// a vector of 25 lines, will be added as needed / requested
	int page_index = 0;					// which page currently being shown
	int total_num_pages = 0;
	string latest_typed;				// keeps track of the current word being typed

	make_new_page(pages, total_num_pages);

	bool overFlow = false;
	if (argc == 2)
	{
		ifstream infile;
		infile.open(argv[1]);
		if (infile.good())
		{
			page_index = 0;
			lines_index = 0;

			string output;
			while (getline(infile, output))
			{
				pages[page_index][lines_index++] = output;				// add each line in text file to each line in current page
				if (lines_index == main_win_height)
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

	string open_file;
	bool stillRunning = true;
	const char sentinel = '0';
	while (stillRunning)
	{
		WINDOW* page_indicator_win = newwin(3, 9, LINES - 3, COLS - 9);
		mvwprintw(page_indicator_win, 1, 2, "pg. %d", page_index + 1);
		box(page_indicator_win, '|', '-');
		wrefresh(page_indicator_win);

		int user_input = wgetch(main_window);

		switch (user_input)
		{
		case sentinel:		// sentinel: breaks out of program
		{
			stillRunning = false;
			break;
		}
		case KEY_SDOWN:		// opens a file
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

			close_win(temp_window);

			ifstream infile;
			infile.open(open_file);
			if (infile.good())
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
		case KEY_SUP:		// make a new page
		{
			make_new_page(pages, total_num_pages);
			set_overflow(overFlow, total_num_pages);
			break;
		}
		case ALT_9:		// save changes to file
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
		case ALT_8:
		{
			ofstream outfile;
			outfile.open(open_file);
			if (outfile.good())
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

			unordered_map<string, int> freqMap = getFreq(open_file);
			priority_queue<pair<string, int>, vector<pair<string, int>>, MaxHeapPairComparer> maxPQ = makeMaxHeap(freqMap);
			unordered_map<string, string> BinRepUM = getBinRep(maxPQ);

			ifstream infile;
			string line;
			vector<string> the_lines;
			infile.open(open_file);
			if (infile.good())
			{
				while (getline(infile, line))
					the_lines.push_back(line);
			}
			infile.close();

			string curr_word = "";
			int i = 0;
			for (auto &str : the_lines)
			{
				while (i < str.length())
				{
					if ((str[i] == ' ') || (str[i] == '\n'))
					{
						int str_len_diff = curr_word.length() - BinRepUM[curr_word].length();
						str.replace(str.find(curr_word), curr_word.length(), BinRepUM[curr_word]);
						curr_word = "";
						i -= str_len_diff;
						i++;
					}
					else
					{
						curr_word.push_back(str[i]);
						i++;
					}
				}
				str.replace(str.find(curr_word), curr_word.length(), BinRepUM[curr_word]);
			}

			ofstream BinRepOutfile;
			ofstream BinMappingOutfile;
			BinRepOutfile.open("BinRep.txt");
			BinMappingOutfile.open("BinMapping.txt");
			BinRepOutfile.clear();
			BinMappingOutfile.clear();

			for (auto line : the_lines)
				BinRepOutfile << line << endl;
			for (auto row : BinRepUM)
				BinMappingOutfile << row.first << ": " << row.second << endl;

			BinRepOutfile.close();
			BinMappingOutfile.close();
			break;
		}
		case ALT_1:		// create new file
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
			mvwprintw(temp_window, temp_win_cursy, temp_win_cursx, "Name file: ");
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

			string sfile_name(file_name.begin(), file_name.end());		// make file_name vestor into string sfile_name
			open_file = sfile_name;

			close_win(temp_window);
			break;
		}
		case KEY_DC:	// delete character
		{
			latest_typed = "";
			if (main_win_cursx > 1)
			{
				mvwdelch(main_window, main_win_cursy, --main_win_cursx);
				mvwdelch(main_window, main_win_cursy, main_win_width - 2);
				box(main_window, '|', '-');
				wmove(main_window, main_win_cursy, main_win_cursx);
				wrefresh(main_window);
				pages[page_index][lines_index].erase(pages[page_index][lines_index].begin() + buffer_index);
				buffer_index--;
			}
			break;
		}
		case KEY_RIGHT:
		{
			latest_typed = "";
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
			latest_typed = "";
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
			latest_typed = "";
			// moves cursor down if it's not already all the way at the bottom, else if it is and overflow is true, then scroll
			if (main_win_cursy < main_win_height)
			{
				wmove(main_window, ++main_win_cursy, main_win_cursx);
				wrefresh(main_window);
				lines_index++;
				flush_buffer(buffer);
				string next_line = pages[page_index][lines_index];
				vector<char> new_chars(next_line.begin(), next_line.end());
				copy_vector(new_chars, buffer);
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

					print_vect_page(main_window, main_win_cursy, main_win_cursx, ++page_index, pages);

					main_win_cursy = 1;
					main_win_cursx = 1;
					lines_index = 0;
					flush_buffer(buffer);
					buffer_index = 0;
					string next_line = pages[page_index][lines_index];
					vector<char> new_chars(next_line.begin(), next_line.end());
					copy_vector(new_chars, buffer);
					wmove(main_window, main_win_cursy, main_win_cursx);
				}
			}
			break;
		}
		case KEY_UP:
		{
			latest_typed = "";
			// moves cursor up if it's not already all the way at the top
			if (main_win_cursy > 1)
			{
				wmove(main_window, --main_win_cursy, main_win_cursx);
				wrefresh(main_window);
				lines_index--;
				flush_buffer(buffer);
				string next_line = pages[page_index][lines_index];
				vector<char> new_chars(next_line.begin(), next_line.end());
				copy_vector(new_chars, buffer);
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
					string next_line = pages[page_index][lines_index];
					vector<char> new_chars(next_line.begin(), next_line.end());
					copy_vector(new_chars, buffer);
					wmove(main_window, main_win_cursy, main_win_cursx);
				}
			}
			break;
		}
		case ALT_4:		// opens keyword auto-suggestions
		{
			curs_set(0);
			int suggestions_win_width = 20;
			int suggestions_win_height = 7;
			WINDOW* sugg_win = make_suggestions_win(suggestions_win_height, suggestions_win_width, main_win_cursy, main_win_cursx);
			keypad(sugg_win, TRUE);

			int sugg_win_cursy = 1;
			int sugg_win_cursx = 1;
			bool keepGoing = true;
			while (keepGoing)
			{
				pair<vector<string>, int> selected_option = show_word_suggestions(sugg_win, suggestions_win_height, kywds_trie, latest_typed, sugg_win_cursy - 1);
				int c = mvwgetch(sugg_win, sugg_win_cursy, sugg_win_cursx);

				if (c == ALT_4)
				{
					close_win(sugg_win);
					keepGoing = false;
				}
				else if (c == KEY_DOWN)
				{
					wmove(sugg_win, ++sugg_win_cursy, sugg_win_cursx);
					wrefresh(sugg_win);
				}
				else if (c == KEY_UP)
				{
					wmove(sugg_win, --sugg_win_cursy, sugg_win_cursx);
					wrefresh(sugg_win);
				}
				else if (c == KEY_SUP)
				{
					close_win(sugg_win);
					string chosen_word = selected_option.first[selected_option.second];

					for (int i = latest_typed.size(); i < chosen_word.size(); i++)
					{
						mvwaddch(main_window, main_win_cursy, main_win_cursx++, chosen_word[i]);
						update_main_vector(chosen_word[i], buffer, buffer_index, pages, page_index, lines_index);
						wrefresh(main_window);
					}
					keepGoing = false;
				}
			}
			curs_set(1);
			break;
		}
		}


		if ((user_input != '0') and (user_input != KEY_RIGHT) and (user_input != KEY_LEFT) and
			(user_input != KEY_UP) and (user_input != KEY_DOWN) and (user_input != KEY_SDOWN) and
			(user_input != KEY_SUP) and (user_input != KEY_DC) and (user_input != ALT_1) and
			(user_input != ALT_9) and (user_input != ALT_8) and (user_input != ALT_4))
		{
			mvwaddch(main_window, main_win_cursy, main_win_cursx++, user_input);
			update_main_vector(user_input, buffer, buffer_index, pages, page_index, lines_index);
			wrefresh(main_window);

			if (user_input == ' ')
				latest_typed = "";
			else
				latest_typed.push_back(user_input);
		}
	}

	endwin();



	return 0;
}

template<class T, class K>
void printUM(unordered_map<T, K> UM)
{
	for (auto row : UM)
		cout << row.first << ": " << row.second << endl;
}

unordered_map<string, int> getFreq(string file_name)
{
	ifstream infile;
	infile.open(file_name);

	unordered_map<string, int> freqMap;
	string line;
	string curr_word;
	if (infile.good())
	{
		while (getline(infile, line))
		{
			for (auto ch : line)
			{
				if ((ch == ' ') || (ch == '\n'))
				{
					freqMap[curr_word]++;
					curr_word = "";
				}
				else
					curr_word.push_back(ch);
			}
			freqMap[curr_word]++;
		}
	}
	infile.close();

	freqMap.erase("");

	return freqMap;
}

priority_queue< pair<string, int>, vector<pair<string, int>>, MaxHeapPairComparer > makeMaxHeap(unordered_map<string, int> UM)
{
	priority_queue< pair<string, int>, vector<pair<string, int>>, MaxHeapPairComparer > maxPQ{};
	for (auto row : UM)
	{
		maxPQ.push(make_pair(row.first, row.second));
	}
	return maxPQ;
}

string dec_to_bin(int convert_num)
{
	stack<char> binary_stack;
	if (convert_num == 0)
	{
		binary_stack.push('0');
		goto here;
	}

	while (convert_num > 0)
	{
		if ((convert_num & 1) == 1)
			binary_stack.push('1');
		else
			binary_stack.push('0');

		convert_num = convert_num >> 1;
	}

here:
	string final_bitstr;
	while (!binary_stack.empty())
	{
		final_bitstr.push_back(binary_stack.top());
		binary_stack.pop();
	}

	return final_bitstr;
}

unordered_map<string, string> getBinRep(priority_queue<pair<string, int>, vector<pair<string, int>>, MaxHeapPairComparer> aMaxPQ)
{
	unordered_map<string, string> BinRepUM;
	int counter = 0;
	while (!aMaxPQ.empty())
	{
		BinRepUM[aMaxPQ.top().first] = dec_to_bin(counter);
		aMaxPQ.pop();
		counter++;
	}

	return BinRepUM;
}

void update_main_vector(char update_input, vector<char>& buffer, int& buffer_index, vector<vector<string>>& main_pages, int page_index, int lines_index)
{
	buffer[buffer_index++] = update_input;
	string this_line = chvect_to_str(buffer);
	main_pages[page_index][lines_index] = this_line;
}

void print_vect_page(WINDOW* win, int cursor_y, int cursor_x, int page_num, vector<vector<string>> pages_vect)
{
	for (int i = 0; i < 25; i++)	// print each string in source_vect to win according to page number
		mvwprintw(win, cursor_y++, cursor_x, pages_vect[page_num][i].c_str());
}

vector<char> push_buff_elements()
{
	vector<char> line_vect;
	for (int i = 0; i < 118; i++)
		line_vect.push_back(' ');
	return line_vect;
}

void make_new_page(vector<vector<string>>& current_pages, int& total)
{
	vector<string> new_lines;
	for (int i = 0; i < 25; i++)
		new_lines.push_back("");
	current_pages.push_back(new_lines);
	total++;
}

void set_overflow(bool& overflow, int num_of_pages)
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

void copy_vector(vector<char>& copied_vect, vector<char>& paste_to_vect)
{
	for (int i = 0; i < copied_vect.size(); i++)
		paste_to_vect[i] = copied_vect[i];
}

WINDOW* make_suggestions_win(int suggestions_win_height, int suggestions_win_width, int cursy, int cursx)
{
	WINDOW* suggestions_win;

	if (cursx >= COLS - 22)
		suggestions_win = newwin(suggestions_win_height, suggestions_win_width, cursy + 2, cursx - 20);
	else
		suggestions_win = newwin(suggestions_win_height, suggestions_win_width, cursy + 2, cursx + 1);

	box(suggestions_win, 0, 0);
	return suggestions_win;
}

pair<vector<string>, int> show_word_suggestions(WINDOW* suggestions_win, int suggs_win_height, Trie& keyword_trie, const string& word, int highlight_flag)
{
	init_pair(1, COLOR_BLACK, COLOR_CYAN);
	vector<string> auto_suggestions = keyword_trie.search(word);
	for (int i = 0; i < auto_suggestions.size(); i++)
	{
		if (i == suggs_win_height - 2)
			break;
		else if (i == highlight_flag)
		{
			wattron(suggestions_win, COLOR_PAIR(1));
			mvwprintw(suggestions_win, i + 1, 1, auto_suggestions[i].c_str());
			wattroff(suggestions_win, COLOR_PAIR(1));
		}
		else
			mvwaddstr(suggestions_win, i + 1, 1, auto_suggestions[i].c_str());
	}

	wrefresh(suggestions_win);
	return make_pair(auto_suggestions, highlight_flag);
}

void populate_trie(Trie& trie_tree)
{
	ifstream infile;
	infile.open("keywords.txt");

	string read_in;
	if (infile.good())
	{
		while (getline(infile, read_in))
			trie_tree.addWord(read_in);
	}
	infile.close();
}

void close_win(WINDOW* win_to_close)
{
	werase(win_to_close);
	wrefresh(win_to_close);
	delwin(win_to_close);
}