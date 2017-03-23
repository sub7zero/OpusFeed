//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef FEED_H
#define FEED_H

#include <dirent.h>
#include <zlib.h>

#include "Version.h"
#include "OptionsManager.h"
#include "OpusConverter.h"
#include "Downloader.h"
#include "RFC822Time.h"
#include "Tree.h"
#include "Log.h"
#include "Regex.h"
#include "FileUtil.h"
#include "SqliteStatement.h"
#include "StrUtil.h"
using namespace std;

class Feed{
	public:
		Feed(OptionsManager &options);
		bool execLoop();
	private:
		struct rssitem{
			rssitem():size(0),date(0),crc(0),downloaded(false),converted(false),lastmod(0),originalsize(0),updated(false),replaced(false){}
			string id;
			string title;
			string file;
			int64_t size;
			int64_t date;
			int64_t crc;
			string url;
			string attributes;
			bool downloaded;
			bool converted;
			int64_t lastmod;
			int64_t originalsize;
			string originalmimetype;
			//flags:
			bool updated;			//update media file if modified
			bool replaced;			//force replace media file
		};
		static bool datecmp(const rssitem &a,const rssitem &b);
		//-
		bool openDB();
		bool cleanupOrphanFiles();
		bool cleanupItemsWithMissingFiles();
		void setupComponents();
		bool cleanupExtraItems();
		bool cleanupOldItems();
		//-
		int downloadIfModified(const char *url,const char *file,int64_t time,bool attemptresume);	//1:success, 2:unmodified, 0:fail
		int downloadIfModified(const char *url,ByteArray *buffer,int64_t time,bool attemptresume);  //1:success, 2:unmodified, 0:fail
		int download(const char *url,const char *file,bool attemptresume); //1:success, 0:fail
		int download(const char *url,ByteArray *buffer,bool attemptresume);//1:success, 0:fail
		bool getFeedPropertyStr(const string &key,string &out);
		bool getFeedPropertyVal(const string &key,int64_t &out);
		bool getFeedPropertyBool(const string &key,bool &out);
		bool setFeedPropertyStr(const string &key,const string &val,bool invalidatedb=true);
		bool setFeedPropertyVal(const string &key,int64_t val,bool invalidatedb=true);
		bool setFeedPropertyBool(const string &key,bool val,bool invalidatedb=true);
		string getAvailableFileName(const string &dir,const string &prefix,const string &ext=string());
		bool readDBItems(list<rssitem> &rssitems);
		bool convert(const string &input,const string &output);
		bool submitDBItem(const rssitem &i);
		void xsleep(unsigned long ms);
	private:
		OptionsManager m_options;
		SqliteDB m_db;
		bool m_datachanged;
		Downloader m_downloader;
};

#endif // FEED_H
