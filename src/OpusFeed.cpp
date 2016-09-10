//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <algorithm>
#include <time.h>
#include <utime.h>
#include <sys/stat.h>

#include "OptionsManager.h"
#include "OpusConverter.h"
#include "Downloader.h"
#include "RFC822Time.h"
#include "Tree.h"
#include "Log.h"
#include "Regex.h"
#include "sqlite3.h"
using namespace std;
using namespace Log;

#define OPUS_FEED_VERSION "1.1"

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __unix__
#include <unistd.h>
#endif
//---
void xsleep(unsigned long ms){
	#ifdef _WIN32
	Sleep(ms);
	#else
	usleep(ms*1000);
	#endif
}
//---
string validatefilename(const char *name){
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
int64_t filesize(const char *file){
	FILE *f=fopen(file,"r");
	if (f==NULL)
		return -1;
	fseek(f,0,SEEK_END);
	int64_t ret=ftello64(f);
	fclose(f);
	return ret;
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
struct rssitem{
	string title;
	string url;
	int64_t date;
	string attributes;
};
bool datecmp(const rssitem &a,const rssitem &b){
	return a.date>b.date;
}
//---
inline string str(int64_t i){ // lltoa replacement
	string t;
	t.reserve(35);
	int64_t u=i;
	do{
		t+="0123456789abcdef"[abs(u%10)];
		u/=10;
	} while(u);
	if (i<0)
		t+='-';
	reverse(t.begin(),t.end());
	return t;
}
//---
int main(int argc,char **argv){
	OptionsManager options;
	//-
	SetParam<long> type_samplerates;
	type_samplerates.addValue(48);
	type_samplerates.addValue(24);
	type_samplerates.addValue(16);
	type_samplerates.addValue(12);
	type_samplerates.addValue(8);
	IntParam<long> type_long;
	SetParam<int> type_channels;
	type_channels.addValue(1);
	type_channels.addValue(2);
	BoolParam type_yesno;
	type_yesno.setStrings("yes","no");
	RangeParam<int> type_quality;
	type_quality.setAcceptableRange(0,10);
	StrParam type_string;
	MapParam<Downloader::ProxyType> type_proxytype;
	type_proxytype.addValue("http",Downloader::proxy_http);
	type_proxytype.addValue("socks4",Downloader::proxy_socks4);
	type_proxytype.addValue("socks4a",Downloader::proxy_socks4a);
	type_proxytype.addValue("socks5",Downloader::proxy_socks5);
	type_proxytype.addValue("socks5h",Downloader::proxy_socks5h);
	MapParam<Downloader::HTTPAuthType> type_authtype;
	type_authtype.addValue("basic",Downloader::auth_basic);
	type_authtype.addValue("ntlm",Downloader::auth_ntlm);
	MapParam<Downloader::IPVersion> type_ipversion;
	type_ipversion.addValue("auto",Downloader::ip_auto);
	type_ipversion.addValue("ipv4",Downloader::ip_v4);
	type_ipversion.addValue("ipv6",Downloader::ip_v6);
	//-
	#ifdef _WIN32
	char tmpc[MAX_PATH]={0};
	GetTempPath(MAX_PATH,tmpc);
	string tmpdir=tmpc;
	#else
	string tmpdir="/tmp/";
	#endif
	//-converter options
	options.reg("--sample-rate",&type_samplerates,"sample rate in kHz",48L,true);
	options.reg("--bit-rate",&type_long,"bit rate in kb/s",16L,true);
	options.reg("--channels",&type_channels,"number of channels",2,true);
	options.reg("--embed-cover",&type_yesno,"embed cover in output",true,true);
	options.reg("--use-video",&type_yesno,"use video to generate cover image if possible",true,true);
	options.reg("--cover-quality",&type_quality,"jpeg cover quality",6,true);
	options.reg("--cover-width",&type_long,"cover width, 0 for same as input",0L,true);
	options.reg("--cover-height",&type_long,"cover height, 0 for same as input",0L,true);
	options.reg("--preserve-ar",&type_yesno,"preserve the aspect-ratio of the cover",true,true);
	//-downloader options
	options.reg("--speed-limit",&type_long,"download speed limit in KBytes/s, 0 for unlimited",0L,true);
	options.reg("--follow-location",&type_yesno,"follow http 3xx redirects",true,true);
	options.reg("--ignore-ssl",&type_yesno,"ignore ssl errors",true,true);
	options.reg("--user-agent",&type_string,"http user-agent string",string("Mozilla/5.0"),true);
	options.reg("--cookies",&type_string,"semicolon separated string of 'var=value' cookies",Variant(),false);
	options.reg("--use-proxy",&type_yesno,"use proxy",false,true);
	options.reg("--proxy-host",&type_string,"proxy host",Variant(),false);
	options.reg("--proxy-port",&type_long,"proxy port",Variant(),false);
	options.reg("--proxy-type",&type_proxytype,"proxy type",Downloader::proxy_http,false);
	options.reg("--auth-user",&type_string,"http authentication user",Variant(),false);
	options.reg("--auth-pass",&type_string,"http authentication password",Variant(),false);
	options.reg("--auth-type",&type_authtype,"http authentication type",Downloader::auth_basic,false);
	options.reg("--download-retries",&type_long,"number of retries in case of a failed download",5L,true);
	options.reg("--resume-retries",&type_long,"number of resume attempts before redownloading the file",5L,true);
	options.reg("--retry-wait",&type_long,"wait between retries in seconds",10L,true);
	options.reg("--ip-version",&type_ipversion,"ip version to be used",Downloader::ip_auto,true);
	options.reg("--fetch-if-modified",&type_yesno,"use the 'If-Modified-Header' header when fetching the feed",true,true);
	//-
	options.reg("--max-items",&type_long,"maximum number of items, 0 for unlimited",10L,false);
	options.reg("--max-age",&type_long,"maximum age of items in days, 0 for unlimited",0L,false);
	options.reg("--feed-url",&type_string,"url of an rss feed",Variant(),true);
	options.reg("--media-dir",&type_string,"where to store the media files",Variant(),true);
	options.reg("--media-prefix",&type_string,"url prefix to be used with in the generated rss feed",Variant(),true);
	options.reg("--tmp-dir",&type_string,"temporary directory where partially downloaded files are kept",tmpdir,true);
	options.reg("--db-file",&type_string,"database file for managing items",Variant(),true);
	options.reg("--output-rss",&type_string,"path of the output rss file",Variant(),true);
	options.reg("--update-interval",&type_long,"update interval in minutes, 0 for one shot",10L,true);
	options.reg("--exec-on-change",&type_string,"command to execute on data change",Variant(),false);
	options.reg("--verbose","verbose output");
	options.reg("--quiet","suppress output");
	options.reg("--enable-colors",&type_yesno,"enable colored output",true,true);
	options.reg("--help","this message");
	//-
	for (int i=1;i<argc;i++){
		if (!options.append(argv[i]))
			return -1;
	}
	if (argc==1 || options["--help"].value<bool>()){
		puts("OpusFeed "OPUS_FEED_VERSION" by Alex Izeld, Email : sub7zero@hotmail.com");
		puts("Usage : opusfeed [options]");
		puts("");
		puts("Options :");
		options.print();
		puts("");
		return 0;
	}
	if (!options.validate())
		return -1;
	if (options["--verbose"].value<bool>())
		setLogLevel(verbose);
	if (options["--quiet"].value<bool>())
		setLogLevel(quiet);
    setColorsEnabled(options["--enable-colors"].value<bool>());
	//-open db
	log(verbose,true,true,"+ opening database");
	sqlite3 *db;
	if (sqlite3_open(options["--db-file"].value<string>().c_str(),&db)!=SQLITE_OK){
		log(normal,true,true,"! unable to open the database file");
		return -1;
	}
	if (sqlite3_exec(db,"create table if not exists items (title text primary key,file text,size integer,date integer,url text,attributes text);",NULL,NULL,NULL)!=SQLITE_OK){
		log(normal,true,true,"! unable to write to the database file");
		return -1;
	}
	if (sqlite3_exec(db,"create table if not exists feedinfo (key text primary key,value text);",NULL,NULL,NULL)!=SQLITE_OK){
		log(normal,true,true,"! unable to write to the database file");
		return -1;
	}
	//-cleanup orphan files
	log(verbose,true,true,"+ cleaning up orphan files");
	DIR *dp;
	if((dp=opendir(options["--media-dir"].value<string>().c_str()))!=NULL){
		dirent *dirp;
		while ((dirp=readdir(dp))!=NULL){
			if (strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)
				continue;
			sqlite3_stmt *stmt;
			if (sqlite3_prepare_v2(db,"select * from items where file=?;",-1,&stmt,NULL)!=SQLITE_OK ||
				sqlite3_bind_text(stmt,1,dirp->d_name,-1,NULL)!=SQLITE_OK){
				sqlite3_finalize(stmt);
				log(normal,true,true,"! db error");
				continue;
			}
			if (sqlite3_step(stmt)==SQLITE_DONE){
				string file=options["--media-dir"].value<string>()+"/"+dirp->d_name;
				remove(file.c_str());
			}
			sqlite3_finalize(stmt);
				continue;
		}
		closedir(dp);
	}
	//-setup downloader & converter
	Downloader downloader;
	downloader.setSpeedLimit(options["--speed-limit"].value<long>()*1024);
	downloader.setFollowLocation(options["--follow-location"].value<bool>());
	downloader.setIgnoreSslErrors(options["--ignore-ssl"].value<bool>());
	downloader.setUserAgent(options["--user-agent"].value<string>().c_str());
	if (!options["--cookies"].empty())
		downloader.setCookies(options["--cookies"].value<string>().c_str());
	downloader.setEnableProxy(options["--use-proxy"].value<bool>());
	if (!options["--proxy-host"].empty())
		downloader.setProxyHost(options["--proxy-host"].value<string>().c_str());
	if (!options["--proxy-port"].empty())
		downloader.setProxyPort(options["--proxy-port"].value<long>());
	downloader.setProxyType(options["--proxy-type"].value<Downloader::ProxyType>());
	if (!options["--auth-user"].empty())
		downloader.setAuthUser(options["--auth-user"].value<string>().c_str());
	if (!options["--auth-pass"].empty())
		downloader.setAuthPass(options["--auth-pass"].value<string>().c_str());
	downloader.setIPVersion(options["--ip-version"].value<Downloader::IPVersion>());
	downloader.setAuthType(options["--auth-type"].value<Downloader::HTTPAuthType>());
	OpusConverter::init();
	//-main loop
	while(1){
		{
			bool dbchanged=false;
			log(normal,true,true,"+ fetching feed");
			ByteArray rssbuff;
			bool rssfetched=false;
			//-
			sqlite3_stmt *stmt;
			if (sqlite3_prepare_v2(db,"select value from feedinfo where key=\"lastfetch\"",-1,&stmt,NULL)!=SQLITE_OK){
				sqlite3_finalize(stmt);
				log(normal,true,true,"! db error");
				goto out;
			}
			sqlite3_step(stmt);
			int64_t lastfetch=sqlite3_column_int64(stmt,0);
			sqlite3_finalize(stmt);
			//-
			int64_t now=RFC822Time::nowgmt(); //for updating lastfetch in the db
			for (int i=0;i<options["--download-retries"].value<long>();i++){
				if (!options["--fetch-if-modified"].value<bool>() || lastfetch==0){
					if (downloader.download(options["--feed-url"].value<string>().c_str(),&rssbuff) && downloader.getStatusCode()==200){
						rssfetched=true;
						break;
					}
				}else{
					if (downloader.downloadIfModified(options["--feed-url"].value<string>().c_str(),&rssbuff,lastfetch)){
						long statuscode=downloader.getStatusCode();
						if (statuscode==200){
							rssfetched=true;
							break;
						}else if (statuscode==304){
                            log(normal,true,true,"+ unmodified feed, skipping");
                            //touch the output rss file
                            touch(options["--output-rss"].value<string>().c_str());
                            goto out;
						}
					}
				}
				if (i+1==options["--download-retries"].value<long>())
					break;
				log(normal,true,true,"+ retrying in (%d) seconds...",options["--retry-wait"].value<long>());
				xsleep(options["--retry-wait"].value<long>()*1000);
			}
			int64_t lastmod=downloader.getLastModified();
			if (lastmod==-1)
				lastmod=now; //if no 'Last-Modified' header is present, use server time and hope it's set correctly
			if (!rssfetched){
				log(normal,true,true,"! fetching feed failed");
				goto out;
			}
			//-parse rss
			Tree xml(NULL);
			if (!xml.fromXML(rssbuff) || xml.getName()!="rss"){
				log(normal,true,true,"! invalid rss data");
				goto out;
			}
			Tree *channel=xml.firstChild("channel");
			if (!channel){
				log(normal,true,true,"! feed doesn't contain a channel");
				goto out;
			}
			//-
			if (sqlite3_prepare_v2(db,"select max(date),count(*) from items;",-1,&stmt,NULL)!=SQLITE_OK){
				sqlite3_finalize(stmt);
				log(normal,true,true,"! db error");
				goto out;
			}
			sqlite3_step(stmt);
			int64_t latest=sqlite3_column_int64(stmt,0);
			long count=sqlite3_column_int(stmt,1);
			sqlite3_finalize(stmt);
			//-
			list<rssitem> newitems;
			Tree *item=channel->firstChild("item");
			if (item){
				do{
					Tree *t_date=item->firstChild("pubDate");
					int64_t date;
					if (t_date){
						date=RFC822Time::parse(t_date->getText().c_str());
						if (date==-1)
							date=RFC822Time::nowgmt();
						else{
							long maxage=options["--max-age"].value<long>();
							long maxitems=options["--max-items"].value<long>();
							if ((latest!=0 && date<latest && count>=maxitems) || (maxage!=0 && date<RFC822Time::nowgmt()-maxage*24*60*60))
								continue;
						}
					}else
						date=RFC822Time::nowgmt();
					//-
					Tree *t_title=item->firstChild("title");
					string title;
					if (t_title)
						title=t_title->getText();
					else{
						log(normal,true,true,"! item with no title");
						continue;
					}
					//-
					Tree *t_url=item->firstChild("enclosure");
					if (t_url==NULL){
						log(normal,true,true,"! item has no media file");
						continue;
					}
					string url=t_url->getAttribute("url");
					//-
					sqlite3_stmt *stmt;
					if (sqlite3_prepare_v2(db,"select * from items where title=?;",-1,&stmt,NULL)!=SQLITE_OK ||
						sqlite3_bind_text(stmt,1,title.c_str(),-1,NULL)!=SQLITE_OK){
						sqlite3_finalize(stmt);
						log(normal,true,true,"! db error");
						goto out;
					}
					int rc=sqlite3_step(stmt);
					sqlite3_finalize(stmt);
					if (rc==SQLITE_ROW)
						continue;
					else if (rc==SQLITE_DONE){
						if (t_date)
							item->deleteChild(t_date);
						if (t_title)
							item->deleteChild(t_title);
						if (t_url)
							item->deleteChild(t_url);
						ByteArray attributes_bytearray=item->toXML();
						string attributes;
						if (attributes_bytearray.size())
							attributes=string(attributes_bytearray.buffer(),attributes_bytearray.size());
						rssitem newitem={title,url,date,attributes};
						newitems.push_back(newitem);
					}else{
						log(normal,true,true,"! db error");
						goto out;
					}
				}while((item=item->nextSibling("item")));
			}
			//-delete all items from the tree
			item=channel->firstChild("item");
			while(item){
				channel->deleteChild(item);
				item=channel->firstChild("item");
			}
			//-process new items
			if (newitems.size())
				dbchanged=true;
			log(normal,true,true,"+ (%d) new items",newitems.size());
			newitems.sort(datecmp);
			list<rssitem>::iterator i=newitems.begin();
			long maxitems=options["--max-items"].value<long>();
			while(i!=newitems.end() && (maxitems==0 || count+distance(newitems.begin(),i)<maxitems)){
				log(normal,true,true,"+ downloading (%s)",i->title.c_str());
				string tname="opusfeed-"+str(RFC822Time::nowgmt());
				string tfile=options["--tmp-dir"].value<string>()+"/"+tname;
				bool downloaded=false;
				for (int d=0;d<options["--download-retries"].value<long>();d++){
					if (downloader.download(i->url.c_str(),tfile.c_str()) && downloader.getStatusCode()==200){
						downloaded=true;
						break;
					}else{
						for (int r=0;r<options["--resume-retries"].value<long>();r++){
							log(normal,true,true,"+ attempting to resume in (%d) seconds...",options["--retry-wait"].value<long>());
							xsleep(options["--retry-wait"].value<long>()*1000);
							if (downloader.resume(i->url.c_str(),tfile.c_str()) && downloader.getStatusCode()==200){
								downloaded=true;
								break;
							}
						}
						if (downloaded)
							break;
					}
					if (d+1==options["--download-retries"].value<long>())
						break;
					log(normal,true,true,"+ retrying in (%d) seconds...",options["--retry-wait"].value<long>());
					xsleep(options["--retry-wait"].value<long>()*1000);
				}
				if (downloaded){
					log(normal,true,true,"+ converting (%s)",i->title.c_str());
					string cname=downloader.getRemoteFileName();
					if (cname.empty()){
						Regex::match m=Regex::extract("/([^/\\\\]+)$",i->url.c_str(),0);
						if (m.size())
							cname=m[0][1];
						else
							cname=tname;
					}
					Regex::match m=Regex::extract("(.+?)(\\.[^.]*$|$)",cname.c_str(),0);
					cname=m[0][1]+".opus";
					cname=validatefilename(cname.c_str());
					string cfile=options["--media-dir"].value<string>()+"/"+cname;
					if (OpusConverter::convert(tfile.c_str(),
											   cfile.c_str(),
											   options["--sample-rate"].value<long>()*1000,
											   options["--channels"].value<int>(),
											   options["--bit-rate"].value<long>()*1000,
											   options["--embed-cover"].value<bool>(),
											   options["--use-video"].value<bool>(),
											   options["--cover-width"].value<long>(),
											   options["--cover-height"].value<long>(),
											   options["--preserve-ar"].value<bool>(),
											   options["--cover-quality"].value<int>())){
						sqlite3_stmt *stmt;
						if (sqlite3_prepare_v2(db,"insert into items values (?,?,?,?,?,?)",-1,&stmt,NULL)!=SQLITE_OK){
							sqlite3_finalize(stmt);
							log(normal,true,true,"! db error");
							goto out;
						}
						sqlite3_bind_text(stmt,1,i->title.c_str(),-1,NULL);
						sqlite3_bind_text(stmt,2,cname.c_str(),-1,NULL);
						sqlite3_bind_int64(stmt,3,filesize(cfile.c_str()));
						sqlite3_bind_int64(stmt,4,i->date);
						sqlite3_bind_text(stmt,5,i->url.c_str(),-1,NULL);
						sqlite3_bind_text(stmt,6,i->attributes.c_str(),-1,NULL);
						sqlite3_step(stmt);
						sqlite3_finalize(stmt);
					}
				}
				remove(tfile.c_str());
				i++;
			}
			//-cleanup extra items
			log(verbose,true,true,"+ cleaning up outdated items");
			if (!options["--max-items"].empty()){
				sqlite3_stmt *stmt;
				string statement="select file from items where title not in (select title from items order by date desc limit "+str(options["--max-items"].value<long>())+")";
				if (sqlite3_prepare_v2(db,statement.c_str(),-1,&stmt,NULL)!=SQLITE_OK){
					sqlite3_finalize(stmt);
					log(normal,true,true,"! db error");
					goto out;
				}
				while (sqlite3_step(stmt)==SQLITE_ROW){
					dbchanged=true;
					string file=options["--media-dir"].value<string>()+"/"+(const char *)sqlite3_column_text(stmt,0);
					remove(file.c_str());
				}
				sqlite3_finalize(stmt);
				statement="delete from items where title not in (select title from items order by date desc limit "+str(options["--max-items"].value<long>())+")";
				sqlite3_exec(db,statement.c_str(),NULL,NULL,NULL);
			}
			//-cleanup old items
			if (!options["--max-age"].empty()){
				sqlite3_stmt *stmt;
				long maxage=options["--max-age"].value<long>();
				string statement="select file from items where date<"+str(RFC822Time::nowgmt()-maxage*24*60*60);
				if (sqlite3_prepare_v2(db,statement.c_str(),-1,&stmt,NULL)!=SQLITE_OK){
					sqlite3_finalize(stmt);
					log(normal,true,true,"! db error");
					goto out;
				}
				while (sqlite3_step(stmt)==SQLITE_ROW){
					dbchanged=true;
					string file=options["--media-dir"].value<string>()+"/"+(const char *)sqlite3_column_text(stmt,0);
					remove(file.c_str());
				}
				sqlite3_finalize(stmt);
				statement="delete file from items where date<"+str(RFC822Time::nowgmt()-maxage*24*60*60);
				sqlite3_exec(db,statement.c_str(),NULL,NULL,NULL);
			}
			//-generate rss items
			log(verbose,true,true,"+ generating rss feed");
			if (sqlite3_prepare_v2(db,"select * from items",-1,&stmt,NULL)!=SQLITE_OK){
				sqlite3_finalize(stmt);
				log(normal,true,true,"! db error");
				goto out;
			}
			while (sqlite3_step(stmt)==SQLITE_ROW){
				Tree *e=channel->insertChild("item");
				const char *attributes=(const char*)sqlite3_column_text(stmt,5);
				ByteArray attributes_bytearray;
				attributes_bytearray.append(attributes,strlen(attributes)+1);
				e->fromXML(attributes_bytearray);
				Tree *t_title=e->insertChild("title");
				t_title->setText((const char*)sqlite3_column_text(stmt,0));
				Tree *t_url=e->insertChild("enclosure");
				t_url->setAttribute("url",options["--media-prefix"].value<string>()+"/"+(const char*)sqlite3_column_text(stmt,1));
				t_url->setAttribute("length",(const char *)sqlite3_column_text(stmt,2));
				t_url->setAttribute("type","audio/opus");
				Tree *t_date=e->insertChild("pubDate");
				t_date->setText(RFC822Time::format(sqlite3_column_int64(stmt,3),"GMT"));
				//-
				Tree *t_description=e->firstChild("description");
				if (!t_description)
					t_description=e->insertChild("description");
				const char *url=(const char*)sqlite3_column_text(stmt,4);
				string description=(string)"<b>Automatically generated using OpusFeed "OPUS_FEED_VERSION" </b><br>Original file : <a href=\""+url+"\">"+url+"</a><br><br>"+t_description->getText();
				t_description->setText(description);
				//-
				Tree *t_itunessum=e->firstChild("itunes:summary");
				if (t_itunessum){
					string idescription=(string)"<b>Automatically generated using OpusFeed "OPUS_FEED_VERSION" </b><br>Original file : <a href=\""+url+"\">"+url+"</a><br><br>"+t_itunessum->getText();
					t_itunessum->setText(idescription);
				}
			}
			sqlite3_finalize(stmt);
			//-setting up channel
			Tree *t_generator=channel->firstChild("generator");
			if (!t_generator)
				t_generator=channel->insertChild("generator");
			t_generator->setText("OpusFeed "OPUS_FEED_VERSION);
			string date=RFC822Time::format(RFC822Time::nowgmt(),"GMT");
			Tree *t_date=channel->firstChild("pubDate");
			if (!t_date)
				t_date=channel->insertChild("pubDate");
			t_date->setText(date);
			Tree *t_builddate=channel->firstChild("lastBuildDate");
			if (!t_builddate)
				t_builddate=channel->insertChild("lastBuildDate");
			t_builddate->setText(date);
			Tree *t_description=channel->firstChild("description");
			if (!t_description)
				t_description=channel->insertChild("description");
			string description=(string)"This feed was automatically generated using OpusFeed "OPUS_FEED_VERSION" \n"
							   "By Alex Izeld, Email : sub7zero@hotmail.com \n"+
							   "original feed url : "+options["--feed-url"].value<string>()+" \n"+
							   t_description->getText();
			t_description->setText(description);
			Tree *t_idescription=channel->firstChild("itunes:summary");
			if (t_idescription){
				string idescription=(string)"This feed was automatically generated using OpusFeed "OPUS_FEED_VERSION" \n"
									"By Alex Izeld, Email : sub7zero@hotmail.com \n"+
									"original feed url : "+options["--feed-url"].value<string>()+" \n"+
									t_idescription->getText();
				t_idescription->setText(idescription);
			}
			//-
			ByteArray output=xml.toXML();
			FILE *fout=fopen(options["--output-rss"].value<string>().c_str(),"wb");
			if (fout==NULL){
				log(normal,true,true,"! couldn't open the output rss file");
				goto out;
			}
			static const string xmldec="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
			if (fwrite(xmldec.c_str(),xmldec.size(),1,fout)==0 || fwrite(output.buffer(),output.size(),1,fout)==0){
				log(normal,true,true,"! couldn't write data to the output file");
				fclose(fout);
				goto out;
			}
			fclose(fout);
			//-update "lastfetch" in the db
			if (sqlite3_prepare_v2(db,"insert or ignore into feedinfo values(?,?)",-1,&stmt,NULL)!=SQLITE_OK){
				sqlite3_finalize(stmt);
				log(normal,true,true,"! db error");
				goto out;
			}
			sqlite3_bind_text(stmt,1,"lastfetch",-1,NULL);
			sqlite3_bind_int64(stmt,2,lastmod);
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
			string statement="update feedinfo set value="+str(lastmod)+" where key=\"lastfetch\"";
			sqlite3_exec(db,statement.c_str(),NULL,NULL,NULL);
			//-execute user command if necessary
			if (dbchanged && !options["--exec-on-change"].empty()){
				log(verbose,true,true,"+ executing user command");
				system(options["--exec-on-change"].value<string>().c_str());
			}
		}
		out:
		long ui=options["--update-interval"].value<long>();
		if (ui==0)
			break;
		log(normal,true,true,"+ updating in (%d) minutes...",ui);
		xsleep(ui*60*1000);
	}
	return 0;
}
