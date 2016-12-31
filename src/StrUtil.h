#ifndef STRUTIL_H
#define STRUTIL_H

#include <string>
#include <inttypes.h>
#include <math.h>
#include <algorithm>
using namespace std;

namespace StrUtil{
	string str(int64_t i); // lltoa replacement
	int64_t val(const string &s);
};

#endif // STRUTIL_H
