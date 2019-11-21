#define PDC_DLL_BUILD 1
#include <cstdlib>
#include <string>
#include <iostream>
#include "curses.h"
using namespace std;

class CurrentWordTracker
{
public:
	CurrentWordTracker(WINDOW* window_to_track)
	{
		tracking_window = window_to_track;
	}
	string get_current_word()
	{
		string string_line(current_word.begin(), current_word.end());
		return string_line; 
	}
	void append_char(char char_to_add)
	{
		current_word.push_back(char_to_add);
	}
	void insert_char_at(int index, char char_to_add)
	{
		auto iterPos = current_word.begin() + index;
		current_word.insert(iterPos, char_to_add);
	}
	void delete_last_char()
	{
		current_word.pop_back();
	}
	void delete_char_at(int index)
	{
		auto iterPos = current_word.begin() + index;
		current_word.erase(iterPos);
	}
	void clear()
	{
		current_word.clear();
	}
	int word_length()
	{
		return current_word.size();
	}
private:
	WINDOW* tracking_window;
	vector<char> current_word = {};
};