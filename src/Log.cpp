//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "Log.h"
//---
namespace Log{
	LogLevel p_loglevel=normal;
	bool p_colorsenabled=true;
	//---
	void setLogLevel(LogLevel l){
		p_loglevel=l;
	}
	//---
	void setColorsEnabled(bool b){
		p_colorsenabled=b;
	}
	//---
	void log(LogLevel l,bool newline,bool linebreak,const char *fmt,...){
		static bool plinebreak=true;
		if (l<p_loglevel)
			return;
		va_list args;
		va_start(args,fmt);
		if (!plinebreak && newline){
			if (p_colorsenabled)
				setFGColor(WHITE);
			cout<<'\n';
		}
		if (p_colorsenabled && (plinebreak || newline)){
			const char *c=fmt;
			while(*c){
				if (!isspace((int)*c))
					break;
				c++;
			}
			switch (*c){
				case '+':
					setFGColor(GREEN);
					break;
				case '-':
					setFGColor(YELLOW);
					break;
				case '!':
					setFGColor(RED);
					break;
			}
		}
		vprintf(fmt,args);
		if (linebreak){
			if (p_colorsenabled)
				setFGColor(WHITE);
			cout<<'\n';
		}
		va_end(args);
		plinebreak=linebreak;
	}
}
