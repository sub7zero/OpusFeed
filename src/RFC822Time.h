//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef RFC822TIME_H
#define RFC822TIME_H

#include <string.h>
#include <time.h>
#include "Regex.h"
#include <iostream>
#include <inttypes.h>
using namespace std;

namespace RFC822Time{
	struct xtime{
		int y;
		int m; //0=Jan,...
		int d; //1-31
		int h;
		int i;
		int s;
	};
	int64_t parse(const char *str); // returns -1 when invalid
	int64_t xtime2sec(xtime t);
	xtime sec2xtime(int64_t t);
	int64_t nowgmt();
	string format(int64_t gmt,const char *timez=NULL,int offset=0); //offset is in minutes
};

#endif // RFC822TIME_H
