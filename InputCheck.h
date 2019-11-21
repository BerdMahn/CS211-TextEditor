#define PDC_DLL_BUILD 1
#include <cstdlib>
#include <string>
#include <iostream>
#include "curses.h"
using namespace std;

//when key is pressed, this object returns a bool depending on what the char is
class InputCheck
{
public:
	InputCheck(WINDOW* window_to_track)
	{
		tracking_window = window_to_track;
	}
	bool isSpecialKey(int user_input)
	{
		for (int i = 0; i < 12; i++)
		{
			if (user_input == possible_inputs[i])
				return true;
		}
		return false;
	}
	bool isSpace(int user_input)
	{
		return (user_input == ' ');
	}
private:
	int possible_inputs[12] = { '0', KEY_RIGHT, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_SDOWN, KEY_SUP, KEY_DC, ALT_1, ALT_4, ALT_8, ALT_9 };
	WINDOW* tracking_window;
};