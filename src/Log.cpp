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
Log::LogLevel Log::p_loglevel=normal;
bool Log::p_colorsenabled=true;
bool Log::p_progressenabled=true;
//---
void Log::log(LogLevel l,bool newline,bool linebreak,const char *fmt,...){
	static char buff[256];
	static bool plinebreak=true;
	//-
	if (l<p_loglevel)
		return;
	va_list args;
	va_start(args,fmt);
	vsnprintf(buff,256,fmt,args);
	va_end(args);
	const char *c=fmt;
	while(*c){
		if (!isspace((int)*c))
			break;
		c++;
	}
	if (!p_progressenabled && *c=='>')
		return;
	if (!plinebreak && newline){
		if (p_colorsenabled)
			setFGColor(WHITE);
		cout<<endl;
	}
	if (p_colorsenabled && (plinebreak || newline)){
		switch (*c){
			case '+':
				setFGColor(GREEN);
				break;
			case '-':
				setFGColor(YELLOW);
				break;
			case '>':
				setFGColor(CYAN);
				break;
			case '!':
				setFGColor(RED);
				break;
		}
	}
	cout<<buff;
	if (linebreak){
		if (p_colorsenabled)
			restoreColors();
		cout<<endl;
	}
	plinebreak=linebreak;
}
