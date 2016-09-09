//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef LOG_H
#define LOG_H

#include <cstdarg>
#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include "ConsoleColor.h"
using namespace ConsoleColor;
using namespace std;

namespace Log{
	enum LogLevel{
		verbose,    //print everything
		normal,     //print errors & status (default)
		quiet       //print nothing
	};
	void setLogLevel(LogLevel l);
	void setColorsEnabled(bool b); //true by default
	void log(LogLevel l,bool newline,bool linebreak,const char *fmt,...);
};
#endif //LOG_H
