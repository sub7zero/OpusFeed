#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <string>
#include <inttypes.h>
#include <stdlib.h>
#include <cstring>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>

#include "Regex.h"
using namespace std;

namespace FileUtil{
	string validateFileName(const char *name);
	int64_t fileSize(const char *file);
	bool fileExists(const char *file);
	void touch(const char *file);
	string getFileFullName(const string &path);
	string getFileName(const string &path);
	string getFileExt(const string &path);
	string getUrlFileName(const string &url);
};

#endif // FILEUTIL_H
