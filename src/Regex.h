//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef REGEX_H
#define REGEX_H

#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "pcre.h"
#include "Log.h"
using namespace std;

namespace Regex{
	typedef vector<vector<string> > match;
	match extract(const char *regex,const char *str,int flags=0);
};

#endif // REGEX_H
