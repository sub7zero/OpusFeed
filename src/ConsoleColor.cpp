//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "ConsoleColor.h"
//---
namespace ConsoleColor{
	//---
	#ifdef _WIN32
	struct wincolor{
		color c;
		int code;
	};
	const struct wincolor wincolors[]={
		{BLACK,0},
		{RED,12},
		{GREEN,10},
		{YELLOW,14},
		{BLUE,9},
		{MAGENTA,13},
		{CYAN,11},
		{WHITE,7}
	};
	WORD defaultwincolor;
	#else
	struct ansicolor{
		color c;
		char code[7];
	};
	const struct ansicolor ansifgcolors[]={
		{BLACK,		"\033[30m"},
		{RED,		"\033[31m"},
		{GREEN,		"\033[32m"},
		{YELLOW,	"\033[33m"},
		{BLUE,		"\033[34m"},
		{MAGENTA,	"\033[35m"},
		{CYAN,		"\033[36m"},
		{WHITE,		"\033[37m"}
	};
	const struct ansicolor ansibgcolors[]={
		{BLACK,		"\033[40m"},
		{RED,		"\033[41m"},
		{GREEN,		"\033[42m"},
		{YELLOW,	"\033[43m"},
		{BLUE,		"\033[44m"},
		{MAGENTA,	"\033[45m"},
		{CYAN,		"\033[46m"},
		{WHITE,		"\033[47m"}
	};
	#endif
	//---
	void setFGColor(color c){
		#ifdef _WIN32
			for (unsigned int i=0;i<sizeof(wincolors)/sizeof(wincolor);i++){
				if (wincolors[i].c==c){
					HANDLE hstdout=GetStdHandle(STD_OUTPUT_HANDLE);
					CONSOLE_SCREEN_BUFFER_INFO csbi;
					GetConsoleScreenBufferInfo(hstdout,&csbi);
					SetConsoleTextAttribute(hstdout,(csbi.wAttributes & 0xFFF0)|(WORD)wincolors[i].code);
					return;
				}
			}
		#else
			for (unsigned int i=0;i<sizeof(ansifgcolors)/sizeof(ansicolor);i++){
				if (ansifgcolors[i].c==c){
					cout<<ansifgcolors[i].code;
					return;
				}
			}
		#endif
	}
	//---
	void setBGColor(color c){
		#ifdef _WIN32
			for (unsigned int i=0;i<sizeof(wincolors)/sizeof(wincolor);i++){
				if (wincolors[i].c==c){
					HANDLE hstdout=GetStdHandle(STD_OUTPUT_HANDLE);
					CONSOLE_SCREEN_BUFFER_INFO csbi;
					GetConsoleScreenBufferInfo(hstdout,&csbi);
					SetConsoleTextAttribute(hstdout,(csbi.wAttributes & 0xFF0F)|(((WORD)wincolors[i].code)<<4));
					return;
				}
			}
		#else
			for (unsigned int i=0;i<sizeof(ansibgcolors)/sizeof(ansicolor);i++){
				if (ansibgcolors[i].c==c){
					cout<<ansibgcolors[i].code;
					return;
				}
			}
		#endif
	}
	//---
}
