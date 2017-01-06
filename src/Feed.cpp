//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "Feed.h"
//---
Feed::Feed(OptionsManager &options){
	m_options=options;
	m_datachanged=false;
}
//---
bool Feed::execLoop(){
	if (!openDB() ||
		!cleanupOrphanFiles() ||
		!cleanupItemsWithMissingFiles())
	return false;
	setupComponents();
	//-
	bool firstloop=true;
	while (1){
		if (!firstloop){
			long ui=m_options["--update-interval"].value<long>();
			if (ui==0)
				return true;;
			Log::log(Log::normal,true,true,"+ updating in [%d] minute(s)...",ui);
			xsleep(ui*60*1000);
		}else
			firstloop=false;
		//-read db items
		list<rssitem> rssitems;
		if (!readDBItems(rssitems))
			return false;
		//-fetch feed
		Log::log(Log::normal,true,true,"+ fetching feed");
		int64_t lastfetch=0;
		getFeedPropertyVal("lastfetch",lastfetch);
		int64_t now=RFC822Time::nowgmt(); //for updating lastfetch in the db
		ByteArray rssbuff;
		bool fetched=false;
		if (lastfetch==0)
			fetched=download(m_options["--feed-url"].value<string>().c_str(),&rssbuff,false)==1;
		else{
			int rssfetched=downloadIfModified(m_options["--feed-url"].value<string>().c_str(),&rssbuff,lastfetch,false);
			if (rssfetched==2)
				Log::log(Log::normal,true,true,"+ unmodified feed");
			else if (rssfetched==0)
				Log::log(Log::normal,true,true,"! fetching feed failed");
			else
				fetched=true;
		}
		//-parse rss
		if (fetched){
			lastfetch=m_downloader.getLastModified();
			if (lastfetch==-1)
				lastfetch=now; //if no 'Last-Modified' header is present, use server time and hope it's set correctly
			Tree xml(NULL);
			if (!xml.fromXML(rssbuff) || xml.getName()!="rss")
				Log::log(Log::normal,true,true,"! invalid rss data");
			else{
				Tree *channel=xml.firstChild("channel");
				if (!channel)
					Log::log(Log::normal,true,true,"! feed doesn't contain a channel");
				else{
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
									long maxage=m_options["--max-age"].value<long>();
									if (maxage!=0 && date<RFC822Time::nowgmt()-maxage*24*60*60)
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
								Log::log(Log::normal,true,true,"! item with no title");
								continue;
							}
							//-
							Tree *t_guid=item->firstChild("guid");
							string id;
							if (t_guid)
								id=t_guid->getText();
							else
								id=title;
							//-
							Tree *t_url=item->firstChild("enclosure");
							if (t_url==NULL){
								Log::log(Log::normal,true,true,"! item has no media file");
								continue;
							}
							string url=t_url->getAttribute("url");
							string originalmimetype=t_url->getAttribute("type");
							int64_t originalsize=StrUtil::val(t_url->getAttribute("length"));
							//-
							ByteArray itemxml=item->toXML();
							int64_t crc=crc32(0L,(const Bytef *)itemxml.buffer(),itemxml.size());
							//-
							bool found=false;
							list<rssitem>::iterator i=rssitems.begin();
							while(i!=rssitems.end()){
								if (i->id==id){ //item exists
									if (i->crc!=crc || date>i->date){ //modified item
										if (i->url!=url || i->originalsize!=originalsize) //force redownloading of items with changed urls
											i->replaced=true;
										else
											i->updated=true;
										//-
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
										//-
										i->title=title;
										i->date=date;
										i->crc=crc;
										i->url=url;
										i->attributes=attributes;
										i->originalsize=originalsize;
										i->originalmimetype=originalmimetype;
									}
									found=true;
									break;
								}
								i++;
							}
							if (found)
								continue;
							//-
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
							//-
							rssitem newitem;
							newitem.id=id;
							newitem.title=title;
							newitem.date=date;
							newitem.crc=crc;
							newitem.url=url;
							newitem.attributes=attributes;
							newitem.downloaded=false;
							newitem.converted=false;
							newitem.originalsize=originalsize;
							newitem.originalmimetype=originalmimetype;
							//-
							rssitems.push_back(newitem);
						}while((item=item->nextSibling("item")));
					}
					//-delete all items from the tree
					item=channel->firstChild("item");
					while(item){
						channel->deleteChild(item);
						item=channel->firstChild("item");
					}
					ByteArray rssdata=xml.toXML();
					if (rssdata.size()){
						int64_t rsscrc_db=0;
						getFeedPropertyVal("rsscrc",rsscrc_db);
						unsigned long rsscrc_xml=crc32(0L,(const Bytef *)rssdata.buffer(),rssdata.size());
						if (rsscrc_db!=rsscrc_xml){
							setFeedPropertyStr("rssdata",string(rssdata.buffer(),rssdata.size()));
							setFeedPropertyVal("rsscrc",rsscrc_xml);
						}
					}
				}
			}
		}
		//-print stats
		long maxitems=m_options["--max-items"].value<long>();
		rssitems.sort(Feed::datecmp);
		int n=0;
		int u=0;
		int r=0;
		list<rssitem>::iterator i=rssitems.begin();
		while(i!=rssitems.end() && (maxitems==0 || distance(rssitems.begin(),i)<maxitems)){
			if (i->lastmod==0) //set to x>0 or -1 in db items
				n++;
			else if (i->updated || i->replaced)
				u++;
			else if (!i->converted || !i->downloaded)
				r++;
			i++;
		}
		Log::log(Log::normal,true,true,"+ [%d] new item(s), [%d] updated item(s), [%d] item(s) will be rechecked",n,u,r);
		//-process items
		i=rssitems.begin();
		while(i!=rssitems.end() && (maxitems==0 || distance(rssitems.begin(),i)<maxitems)){
			if (i->converted && !i->replaced && !i->updated){ //nothing to do
				i++;
				continue;
			}
			//-
			if (i->lastmod==0) //set to x>0 or -1 in db items
				Log::log(Log::normal,true,true,"+ downloading (%s)",i->title.c_str());
			else if (i->updated || i->replaced)
				Log::log(Log::normal,true,true,"+ updating (%s)",i->title.c_str());
			else
				Log::log(Log::normal,true,true,"+ rechecking (%s)",i->title.c_str());
			//-
			string tname=getAvailableFileName(m_options["--tmp-dir"].value<string>(),"opusfeed");
			string tfile=m_options["--tmp-dir"].value<string>()+"/"+tname;
			if (i->replaced){
				if (i->converted){
					string file=m_options["--media-dir"].value<string>()+"/"+i->file;
					remove(file.c_str());
					i->converted=false;
				}
				i->file="-";
				i->lastmod=-1;
				i->size=-1;
				i->downloaded=download(i->url.c_str(),tfile.c_str(),true)==1;
			}else if (!i->downloaded){
				if (download(i->url.c_str(),tfile.c_str(),true)!=1){
					if (i->lastmod==0){
						i->file="-";
						i->lastmod=-1;
						i->size=-1;
						submitDBItem(*i);
					}
					remove(tfile.c_str());
					i++;
					continue;
				}else
					i->downloaded=true;
			}else if (!i->converted || i->updated){
				int state=downloadIfModified(i->url.c_str(),tfile.c_str(),i->lastmod,true);
				if (state==0 || state==2){
					if (i->updated)
						submitDBItem(*i);
					i++;
					continue;
				}else if (i->updated && i->converted){
					string file=m_options["--media-dir"].value<string>()+"/"+i->file;
					remove(file.c_str());
				}
			}
			//-
			if (i->downloaded){
				i->lastmod=m_downloader.getLastModified();
				string remotename=m_downloader.getRemoteFileName();
				string ext=FileUtil::getFileExt(remotename);
				if (!ext.empty()){
					string newtname=getAvailableFileName(m_options["--tmp-dir"].value<string>(),"opusfeed",ext);
					string newtfile=m_options["--tmp-dir"].value<string>()+"/"+newtname;
					if (rename(tfile.c_str(),newtfile.c_str())==0){
						tname=newtname;
						tfile=newtfile;
					}
				}
				Log::log(Log::normal,true,true,"+ converting (%s)",i->title.c_str());
				string cname=FileUtil::getFileName(remotename);
				if (cname.empty()){
					cname=FileUtil::getFileName(i->url);
					if (cname.empty())
						cname=tname;
				}
				cname=getAvailableFileName(m_options["--media-dir"].value<string>(),cname,"opus");
				string cfile=m_options["--media-dir"].value<string>()+"/"+cname;
				i->converted=convert(tfile.c_str(),cfile.c_str());
				remove(tfile.c_str());
				if (i->converted){
					i->file=cname;
					i->size=FileUtil::fileSize(cfile.c_str());
				}else{
					remove(cfile.c_str());
					i->file="-";
					i->size=-1;
				}
			}else{
				i->file="-";
				i->lastmod=-1;
				i->size=-1;
			}
			submitDBItem(*i);
			i++;
		}
		//-update 'lastfetch' (after processing the items)
		if (fetched)
			setFeedPropertyVal("lastfetch",lastfetch);
		//-cleanup
		Log::log(Log::normal,true,true,"+ cleaning up outdated items");
		if (!cleanupExtraItems())
			return false;
		if (!cleanupOldItems())
			return false;
		//- force update the rss file when --preserve-failed or --media-prefix change
		string mediaprefix;
		if (!getFeedPropertyStr("mediaprefix",mediaprefix) || mediaprefix!=m_options["--media-prefix"].value<string>())
            setFeedPropertyStr("mediaprefix",m_options["--media-prefix"].value<string>());
		bool preservefailed=true;
		if (!getFeedPropertyBool("preservefailed",preservefailed) || preservefailed!=m_options["--preserve-failed"].value<bool>())
			setFeedPropertyBool("preservefailed",m_options["--preserve-failed"].value<bool>());
		//-
		if (m_datachanged || !FileUtil::fileExists(m_options["--output-rss"].value<string>().c_str())){
			Log::log(Log::normal,true,true,"+ generating rss feed");
			//-generate rss
			string rssdata;
			if (getFeedPropertyStr("rssdata",rssdata)){
				ByteArray rssbuff;
				rssbuff.append(rssdata.c_str(),rssdata.size());
				Tree xml(NULL);
				if (xml.fromXML(rssbuff)){
					Tree *channel=xml.firstChild("channel");
					if (channel){
						//-adding rss items
						rssitems.clear();
						if (!readDBItems(rssitems))
							return false;
						rssitems.sort(Feed::datecmp);
						i=rssitems.begin();
						while(i!=rssitems.end()){
							if ((!i->converted || !i->downloaded) && !m_options["--preserve-failed"].value<bool>()){
								i++;
								continue;
							}
							Tree *e=channel->insertChild("item");
							ByteArray attributes_bytearray;
							attributes_bytearray.append(i->attributes.c_str(),i->attributes.size());
							e->fromXML(attributes_bytearray);
							//-
							Tree *t_title=e->insertChild("title");
							Tree *t_url=e->insertChild("enclosure");
							if (!i->downloaded){
								t_title->setText(i->title+" [Error Downloading]");
								t_url->setAttribute("url",i->url);
								t_url->setAttribute("length",StrUtil::str(i->originalsize));
								t_url->setAttribute("type",i->originalmimetype);
							}else if (!i->converted){
								t_title->setText(i->title+" [Error Converting]");
								t_url->setAttribute("url",i->url);
								t_url->setAttribute("length",StrUtil::str(i->originalsize));
								t_url->setAttribute("type",i->originalmimetype);
							}else{
								t_title->setText(i->title);
								t_url->setAttribute("url",m_options["--media-prefix"].value<string>()+"/"+i->file);
								t_url->setAttribute("length",StrUtil::str(i->size));
								t_url->setAttribute("type","audio/opus");
							}
							Tree *t_date=e->insertChild("pubDate");
							t_date->setText(RFC822Time::format(i->date,"GMT"));
							Tree *t_description=e->firstChild("description");
							if (!t_description)
								t_description=e->insertChild("description");
							string description=(string)"<b>Automatically generated using OpusFeed "OPUS_FEED_VERSION" </b><br>Original file : <a href=\""+i->url+"\">"+i->url+"</a><br><br>"+t_description->getText();
							t_description->setText(description);
							Tree *t_itunessum=e->firstChild("itunes:summary");
							if (t_itunessum){
								string idescription=(string)"<b>Automatically generated using OpusFeed "OPUS_FEED_VERSION" </b><br>Original file : <a href=\""+i->url+"\">"+i->url+"</a><br><br>"+t_itunessum->getText();
								t_itunessum->setText(idescription);
							}
							i++;
						}
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
										   "original feed url : "+m_options["--feed-url"].value<string>()+" \n"+
										   t_description->getText();
						t_description->setText(description);
						Tree *t_idescription=channel->firstChild("itunes:summary");
						if (t_idescription){
							string idescription=(string)"This feed was automatically generated using OpusFeed "OPUS_FEED_VERSION" \n"
												"By Alex Izeld, Email : sub7zero@hotmail.com \n"+
												"original feed url : "+m_options["--feed-url"].value<string>()+" \n"+
												t_idescription->getText();
							t_idescription->setText(idescription);
						}
						Tree *t_title=channel->firstChild("title");
						if (t_title){
							string title=t_title->getText()+" - OpusFeed";
							t_title->setText(title);
						}
						//-writing output rss file
						ByteArray output=xml.toXML();
						FILE *fout=fopen(m_options["--output-rss"].value<string>().c_str(),"wb");
						if (fout==NULL){
							Log::log(Log::normal,true,true,"! couldn't open the output rss file");
							continue; //m_datachanged remains unmodified
						}
						static const string xmldec="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
						if (fwrite(xmldec.c_str(),xmldec.size(),1,fout)==0 || fwrite(output.buffer(),output.size(),1,fout)==0){
							Log::log(Log::normal,true,true,"! couldn't write data to the output file");
							fclose(fout);
							remove(m_options["--output-rss"].value<string>().c_str());
							continue; //m_datachanged remains unmodified
						}
						fclose(fout);
					}
				}
			}
			//-
			if (m_datachanged){
				m_datachanged=false;
				//-execute user command if necessary
				if (m_options["--exec-on-change"]){
					Log::log(Log::normal,true,true,"+ executing user command");
					system(m_options["--exec-on-change"].value<string>().c_str());
				}
			}
		}
	}
}
//---
bool Feed::openDB(){
	Log::log(Log::normal,true,true,"+ opening database");
	if (!m_db.open(m_options["--db-file"].value<string>().c_str())){
		Log::log(Log::normal,true,true,"! unable to open the database file");
		return false;
	}
	if (!m_db.exec("create table if not exists items (id text primary key,title text,file text,size integer,date integer,crc integer,url text,attributes text,downloaded text,converted text,moddate integer,originalsize integer,originalmimetype text)")){
		Log::log(Log::normal,true,true,"! unable to write to the database file");
		return false;
	}
	if (!m_db.exec("create table if not exists feedinfo (key text primary key,value text)")){
		Log::log(Log::normal,true,true,"! unable to write to the database file");
		return false;
	}
	return true;
}
//---
bool Feed::cleanupOrphanFiles(){
	Log::log(Log::normal,true,true,"+ cleaning up orphan files");
	DIR *dp;
	if((dp=opendir(m_options["--media-dir"].value<string>().c_str()))!=NULL){
		SqliteStatement s(m_db);
		if (!s.prepare("select * from items where file=?")){
			Log::log(Log::normal,true,true,"! db error");
			return false;
		}
		dirent *dirp;
		while ((dirp=readdir(dp))!=NULL){
			if (strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)
				continue;
			if (!s.bindStr(1,dirp->d_name)){
				Log::log(Log::normal,true,true,"! db error");
				s.reset();
				continue;
			}
			if (s.step()==0){
				string file=m_options["--media-dir"].value<string>()+"/"+dirp->d_name;
				remove(file.c_str());
				m_datachanged=true;
			}
			s.reset();
		}
		closedir(dp);
		s.free();
	}
	return true;
}
//---
bool Feed::cleanupItemsWithMissingFiles(){
	Log::log(Log::normal,true,true,"+ cleaning up items with missing files");
	SqliteStatement s(m_db);
	if (!s.prepare("select id,file from items where converted='true'")){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	while (s.step()==1){
		string id=s.getStr(0);
		string fname=s.getStr(1);
		string file=m_options["--media-dir"].value<string>()+"/"+fname;
		if (!FileUtil::fileExists(file.c_str())){
				SqliteStatement d(m_db);
				if (!d.prepare("delete from items where id=?") ||
					!d.bindStr(1,id.c_str()) ||
					d.step()==-1){
					Log::log(Log::normal,true,true,"! db error");
					return false;
				}
				m_datachanged=true;
		}
	}
	return true;
}
//---
void Feed::setupComponents(){
	m_downloader.setSpeedLimit(m_options["--speed-limit"].value<long>()*1024);
	m_downloader.setFollowLocation(m_options["--follow-location"].value<bool>());
	m_downloader.setIgnoreSslErrors(m_options["--ignore-ssl"].value<bool>());
	m_downloader.setUserAgent(m_options["--user-agent"].value<string>().c_str());
	m_downloader.setUseAuth(m_options["--use-auth"].value<bool>());
	if (m_options["--auth-user"])
		m_downloader.setUser(m_options["--auth-user"].value<string>().c_str());
	if (m_options["--auth-pass"])
		m_downloader.setPass(m_options["--auth-pass"].value<string>().c_str());
	if (m_options["--cookies"])
		m_downloader.setCookies(m_options["--cookies"].value<string>().c_str());
	m_downloader.setUseProxy(m_options["--use-proxy"].value<bool>());
	if (m_options["--proxy-host"])
		m_downloader.setProxyHost(m_options["--proxy-host"].value<string>().c_str());
	if (m_options["--proxy-port"])
		m_downloader.setProxyPort(m_options["--proxy-port"].value<long>());
	m_downloader.setProxyType(m_options["--proxy-type"].value<Downloader::ProxyType>());
	m_downloader.setUseProxyAuth(m_options["--use-proxy-auth"].value<bool>());
	if (m_options["--proxy-auth-user"])
		m_downloader.setProxyAuthUser(m_options["--proxy-auth-user"].value<string>().c_str());
	if (m_options["--proxy-auth-pass"])
		m_downloader.setProxyAuthPass(m_options["--proxy-auth-pass"].value<string>().c_str());
	m_downloader.setIPVersion(m_options["--ip-version"].value<Downloader::IPVersion>());
	OpusConverter::init();
}
//---
bool Feed::cleanupExtraItems(){
	if (m_options["--max-items"]){
		SqliteStatement s(m_db);
		string statement="select file,converted from items where id not in (select id from items order by date desc limit "+StrUtil::str(m_options["--max-items"].value<long>())+")";
		if (!s.prepare(statement.c_str())){
			Log::log(Log::normal,true,true,"! db error");
			return false;
		}
		while (s.step()==1){
			if (s.getBool(1)){
				string file=m_options["--media-dir"].value<string>()+"/"+s.getStr(0);
				remove(file.c_str());
				m_datachanged=true;
			}
		}
		s.free();
		statement="delete from items where id not in (select id from items order by date desc limit "+StrUtil::str(m_options["--max-items"].value<long>())+")";
		if (!m_db.exec(statement.c_str())){
			Log::log(Log::normal,true,true,"! db error");
			return false;
		}
		if (m_db.changes()>0)
			m_datachanged=true;
	}
	return true;
}
//---
bool Feed::cleanupOldItems(){
	long maxage=m_options["--max-age"].value<long>();
	if (maxage!=0){
		SqliteStatement s(m_db);
		string maxdate=StrUtil::str(RFC822Time::nowgmt()-maxage*24*60*60);
		string statement="select file,converted from items where date<"+maxdate;
		if (!s.prepare(statement.c_str())){
			Log::log(Log::normal,true,true,"! db error");
			return false;
		}
		while (s.step()==1){
			if (s.getBool(1)){
				string file=m_options["--media-dir"].value<string>()+"/"+s.getStr(0);
				remove(file.c_str());
				m_datachanged=true;
			}
		}
		s.free();
		statement="delete from items where date<"+maxdate;
		if (!m_db.exec(statement.c_str())){
			Log::log(Log::normal,true,true,"! db error");
			return false;
		}
		if (m_db.changes()>0)
			m_datachanged=true;
	}
	return true;
}
//------
bool Feed::datecmp(const rssitem &a,const rssitem &b){
	return a.date>b.date;
}
//------
int Feed::downloadIfModified(const char *url,const char *file,int64_t time,bool attemptresume){
	for (int d=0;d<m_options["--download-retries"].value<long>();d++){
		if (m_downloader.downloadIfModified(url,file,time)){
			long statuscode=m_downloader.getStatusCode();
			if (statuscode==200)
				return 1;
			else if (statuscode==304)
				return 2;
		}else if (attemptresume){
			for (int r=0;r<m_options["--resume-retries"].value<long>();r++){
				Log::log(Log::normal,true,true,"+ attempting to resume in [%d] seconds...",m_options["--retry-delay"].value<long>());
				xsleep(m_options["--retry-delay"].value<long>()*1000);
				if (m_downloader.resume(url,file)){
					long statuscode=m_downloader.getStatusCode();
					if (statuscode==200 || statuscode==206)
						return 1;
				}
			}
		}
		if (d+1==m_options["--download-retries"].value<long>())
			break;
		Log::log(Log::normal,true,true,"+ retrying in [%d] seconds...",m_options["--retry-delay"].value<long>());
		xsleep(m_options["--retry-delay"].value<long>()*1000);
	}
	remove(file);
	return 0;
}
//---
int Feed::downloadIfModified(const char *url,ByteArray *buffer,int64_t time,bool attemptresume){
	for (int d=0;d<m_options["--download-retries"].value<long>();d++){
		if (m_downloader.downloadIfModified(url,buffer,time)){
			long statuscode=m_downloader.getStatusCode();
			if (statuscode==200)
				return 1;
			else if (statuscode==304)
				return 2;
		}else if (attemptresume){
			for (int r=0;r<m_options["--resume-retries"].value<long>();r++){
				Log::log(Log::normal,true,true,"+ attempting to resume in [%d] seconds...",m_options["--retry-delay"].value<long>());
				xsleep(m_options["--retry-delay"].value<long>()*1000);
				if (m_downloader.resume(url,buffer)){
					long statuscode=m_downloader.getStatusCode();
					if (statuscode==200 || statuscode==206)
						return 1;
				}
			}
		}
		if (d+1==m_options["--download-retries"].value<long>())
			break;
		Log::log(Log::normal,true,true,"+ retrying in [%d] seconds...",m_options["--retry-delay"].value<long>());
		xsleep(m_options["--retry-delay"].value<long>()*1000);
	}
	return 0;
}
//---
int Feed::download(const char *url,const char *file,bool attemptresume){
	for (int d=0;d<m_options["--download-retries"].value<long>();d++){
		if (m_downloader.download(url,file)){
			long statuscode=m_downloader.getStatusCode();
			if (statuscode==200)
				return 1;
		}else if (attemptresume){
			for (int r=0;r<m_options["--resume-retries"].value<long>();r++){
				Log::log(Log::normal,true,true,"+ attempting to resume in [%d] seconds...",m_options["--retry-delay"].value<long>());
				xsleep(m_options["--retry-delay"].value<long>()*1000);
				if (m_downloader.resume(url,file)){
					long statuscode=m_downloader.getStatusCode();
					if (statuscode==200 || statuscode==206)
						return 1;
				}
			}
		}
		if (d+1==m_options["--download-retries"].value<long>())
			break;
		Log::log(Log::normal,true,true,"+ retrying in [%d] seconds...",m_options["--retry-delay"].value<long>());
		xsleep(m_options["--retry-delay"].value<long>()*1000);
	}
	remove(file);
	return 0;
}
//---
int Feed::download(const char *url,ByteArray *buffer,bool attemptresume){
	for (int d=0;d<m_options["--download-retries"].value<long>();d++){
		if (m_downloader.download(url,buffer)){
			long statuscode=m_downloader.getStatusCode();
			if (statuscode==200)
				return 1;
			else
				return 0;
		}else if (attemptresume){
			for (int r=0;r<m_options["--resume-retries"].value<long>();r++){
				Log::log(Log::normal,true,true,"+ attempting to resume in [%d] seconds...",m_options["--retry-delay"].value<long>());
				xsleep(m_options["--retry-delay"].value<long>()*1000);
				if (m_downloader.resume(url,buffer)){
					long statuscode=m_downloader.getStatusCode();
					if (statuscode==200 || statuscode==206)
						return 1;
				}
			}
		}
		if (d+1==m_options["--download-retries"].value<long>())
			break;
		Log::log(Log::normal,true,true,"+ retrying in [%d] seconds...",m_options["--retry-delay"].value<long>());
		xsleep(m_options["--retry-delay"].value<long>()*1000);
	}
	return 0;
}
//---
bool Feed::setFeedPropertyStr(const string &key,const string &val){
	SqliteStatement s(m_db);
	if (!s.prepare("insert or replace into feedinfo values(?,?)") ||
		!s.bindStr(1,key.c_str()) ||
		!s.bindStr(2,val.c_str()) ||
		s.step()==-1){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	m_datachanged=true;
	return true;
}
//---
bool Feed::setFeedPropertyVal(const string &key,int64_t val){
	SqliteStatement s(m_db);
	if (!s.prepare("insert or replace into feedinfo values(?,?)") ||
		!s.bindStr(1,key.c_str()) ||
		!s.bindInt64(2,val) ||
		s.step()==-1){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	m_datachanged=true;
	return true;
}
//---
bool Feed::setFeedPropertyBool(const string &key,bool val){
	SqliteStatement s(m_db);
	if (!s.prepare("insert or replace into feedinfo values(?,?)") ||
		!s.bindStr(1,key.c_str()) ||
		!s.bindBool(2,val) ||
		s.step()==-1){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	m_datachanged=true;
	return true;
}
//---
bool Feed::getFeedPropertyStr(const string &key,string &out){
	SqliteStatement s(m_db);
	if (!s.prepare("select value from feedinfo where key=?") || !s.bindStr(1,key.c_str())){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	int step=s.step();
	if (step==-1){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}else if (step==1){
		out=string(s.getStr(0));
		return true;
	}else
		return false;
}
//---
bool Feed::getFeedPropertyVal(const string &key,int64_t &out){
	SqliteStatement s(m_db);
	if (!s.prepare("select value from feedinfo where key=?") || !s.bindStr(1,key.c_str())){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	int step=s.step();
	if (step==-1){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}else if (step==1){
		out=s.getInt64(0);
		return true;
	}else
		return false;
}
//---
bool Feed::getFeedPropertyBool(const string &key,bool &out){
	SqliteStatement s(m_db);
	if (!s.prepare("select value from feedinfo where key=?") || !s.bindStr(1,key.c_str())){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	int step=s.step();
	if (step==-1){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}else if (step==1){
		out=s.getBool(0);
		return true;
	}else
		return false;
}
//---
string Feed::getAvailableFileName(const string &dir,const string &prefix,const string &ext){
	int64_t padding=0;
	string name=prefix;
	if (!ext.empty())
		name+="."+ext;
	string file=dir+"/"+name;
	while(FileUtil::fileExists(file.c_str())){
		name=prefix+"-"+StrUtil::str(padding);
		if (!ext.empty())
			name+="."+ext;
		file=dir+"/"+name;
		padding++;
	}
	return name;
}
//---
bool Feed::convert(const string &input,const string &output){
	return OpusConverter::convert(input.c_str(),
								  output.c_str(),
								  m_options["--sample-rate"].value<long>()*1000,
								  m_options["--channels"].value<int>(),
								  m_options["--bit-rate"].value<long>()*1000,
								  m_options["--embed-cover"].value<bool>(),
								  m_options["--use-video"].value<bool>(),
								  m_options["--cover-width"].value<long>(),
								  m_options["--cover-height"].value<long>(),
								  m_options["--preserve-ar"].value<bool>(),
								  m_options["--cover-quality"].value<int>());
}
//---
bool Feed::readDBItems(list<rssitem> &rssitems){
	SqliteStatement s(m_db);
	if (!s.prepare("select * from items")){
		Log::log(Log::normal,true,true,"! db error");
		return false;
	}
	while (s.step()==1){
		rssitem i;
		i.id=s.getStr(0);
		i.title=s.getStr(1);
		i.file=s.getStr(2);
		i.size=s.getInt64(3);
		i.date=s.getInt64(4);
		i.crc=s.getInt64(5);
		i.url=s.getStr(6);
		i.attributes=s.getStr(7);
		i.downloaded=s.getBool(8);
		i.converted=s.getBool(9);
		i.lastmod=s.getInt64(10);
		i.originalsize=s.getInt64(11);
		i.originalmimetype=s.getStr(12);
		//-
		i.updated=false;
		i.replaced=false;
		rssitems.push_back(i);
	}
	return true;
}
//---
bool Feed::submitDBItem(const rssitem &i){
	//id,title,file,size,date,crc,url,attributes,downloaded,converted,moddate,originalsize,originalmimetype
	SqliteStatement s(m_db);
	if (!s.prepare("insert or replace into items values (?,?,?,?,?,?,?,?,?,?,?,?,?)") ||
		!s.bindStr(1,i.id.c_str()) ||
		!s.bindStr(2,i.title.c_str()) ||
		!s.bindStr(3,i.file.c_str()) ||
		!s.bindInt64(4,i.size) ||
		!s.bindInt64(5,i.date) ||
		!s.bindInt64(6,i.crc) ||
		!s.bindStr(7,i.url.c_str()) ||
		!s.bindStr(8,i.attributes.c_str()) ||
		!s.bindBool(9,i.downloaded) ||
		!s.bindBool(10,i.converted) ||
		!s.bindInt64(11,i.lastmod) ||
		!s.bindInt64(12,i.originalsize) ||
		!s.bindStr(13,i.originalmimetype.c_str()) ||
		s.step()==-1){
		return false;
		Log::log(Log::normal,true,true,"! db error");
	}
	m_datachanged=true;
	return true;
}
//---
void Feed::xsleep(unsigned long ms){
	#ifdef _WIN32
	Sleep(ms);
	#else
	usleep(ms*1000);
	#endif
}
