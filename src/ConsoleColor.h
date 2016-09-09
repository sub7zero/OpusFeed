//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef CONSOLECOLOR_H
#define CONSOLECOLOR_H

#include <iostream>
#include <stdlib.h>
#include <stack>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

namespace ConsoleColor{
	enum color{
		BLACK,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE
	};
	//-
	void setFGColor(color c);
	void setBGColor(color c);
};

#endif // CONSOLECOLOR_H
