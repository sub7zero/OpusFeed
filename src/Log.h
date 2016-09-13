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

class Log{
	public:
		enum LogLevel{
			verbose,    //print everything
			normal,     //print errors & status (default)
			quiet       //print nothing
		};
		static void setLogLevel(LogLevel l) {p_loglevel=l;}
		static void setColorsEnabled(bool b) {p_colorsenabled=b;} //true by default
		static void setProgressEnabled(bool b) {p_progressenabled=b;} //true by default (lines starting with '\s*>' won't be logged when false)
		static LogLevel getLogLevel() {return p_loglevel;}
		static bool getColorsEnabled() {return p_colorsenabled;}
		static bool getProgressEnabled() {return p_progressenabled;}
		static void log(LogLevel l,bool newline,bool linebreak,const char *fmt,...);
	private:
		static LogLevel p_loglevel;
		static bool p_colorsenabled;
		static bool p_progressenabled;
};
#endif //LOG_H
