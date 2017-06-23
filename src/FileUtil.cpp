#include "FileUtil.h"
//---
namespace FileUtil{
	string validateFileName(const char *name){
		char *str=(char *)malloc(strlen(name)+1);
		const char *r=name;
		char *w=str;
		while(*r){
			#ifdef _WIN32
			if (*r=='/' ||
				*r=='\\'||
				*r==':' ||
				*r=='*' ||
				*r=='?' ||
				*r=='"' ||
				*r=='<' ||
				*r=='>' ||
				*r=='|'){
				r++;
				continue;
			}
			#else
			if (*r=='/'){
				r++;
				continue;
			}
			#endif
			*(w++)=*(r++);
		}
		*w=0;
		string ret(str);
		free(str);
		return ret;
	}
	//---
	int64_t fileSize(const char *file){
		FILE *f=fopen(file,"r");
		if (f==NULL)
			return -1;
		fseek(f,0,SEEK_END);
		int64_t ret=ftello64(f);
		fclose(f);
		return ret;
	}
	//---
	bool fileExists(const char *file){
		FILE *f=fopen(file,"r");
		if (f==NULL)
			return false;
		fclose(f);
		return true;
	}
	//---
	void touch(const char *file){
	  struct stat s;
	  utimbuf newtimes;
	  stat(file,&s);
	  newtimes.actime=s.st_atime;
	  newtimes.modtime=time(NULL);
	  utime(file,&newtimes);
	}
	//---
	string getFileFullName(const string &path){
		Regex::match m=Regex::extract("[/\\\\]?([^/\\\\]+)$",path.c_str(),0);
		if (m.size())
			return validateFileName(m[0][1].c_str());
		return validateFileName(path.c_str());
	}
	//---
	string getFileName(const string &path){
		string fullname=getFileFullName(path);
		Regex::match m=Regex::extract("(.+?)(\\.[^.]*$|$)",fullname.c_str(),0);
		if (m.size())
			return validateFileName(m[0][1].c_str());
		return path;
	}
	//---
	string getFileExt(const string &path){
		string name=getFileFullName(path);
		Regex::match m=Regex::extract("(.+?)\\.([^.]*$|$)",name.c_str(),0);
		if (m.size())
			return validateFileName(m[0][2].c_str());
		return "";
	}
	//---
	string getUrlFileName(const string &url){
		Regex::match m=Regex::extract("([^/\\?#]+)([\\?#].+)?$",url.c_str(),PCRE_MULTILINE);
		if (m.size())
			return validateFileName(m[0][1].c_str());
		return "";
	}
}
