//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "Downloader.h"
//---
Downloader::Downloader(){
	m_contentlen=0;
	m_offset=0;
	m_startoffset=0;
	m_fileh=NULL;
	m_followlocation=true;
	m_speedlimit=0;
	m_ignoresslerrors=true;
	m_useauth=false;
	m_useproxy=false;
	m_proxytype=proxy_http;
	m_useproxyauth=false;
	m_ipversion=ip_auto;
	m_buffer=NULL;
}
//---
size_t Downloader::writefnc(char *buff,size_t size,size_t nitems,void *userdata){
	Downloader *obj=static_cast<Downloader *>(userdata);
	if (obj->m_statuscode!=200 && obj->m_statuscode!=206)
		return size*nitems; //don't write the body of error messages
	if (obj->m_fileh){
		size_t written=fwrite(buff,size,nitems,obj->m_fileh);
		if (written)
			obj->m_offset+=written;
		return written;
	}else{
		size_t asize=size*nitems;
		if (!obj->m_buffer->append(buff,asize)){
			Log::log(Log::normal,true,true,"! couldn't allocate enough memory");
			return 0;
		}
		return asize;
	}
}
//---
size_t Downloader::headerfnc(char *buff,size_t size,size_t nitems,void *userdata){
	Downloader *obj=static_cast<Downloader *>(userdata);
	size_t asize=nitems*size;
	Regex::match matches;
	matches=Regex::extract("Content-Length: (\\d+)",string(buff,asize).c_str(),PCRE_MULTILINE); //buff may not be null terminated
	if (matches.size())
		obj->m_contentlen=obj->m_startoffset+atoll(matches[0][1].c_str());
	matches=Regex::extract("Content-Disposition: .*filename\\s*=\\s*\"([^;]+)\\s*\"",string(buff,asize).c_str(),PCRE_MULTILINE); //buff may not be null terminated
	if (matches.size())
		obj->m_remotefilename=matches[0][1].c_str();
	matches=Regex::extract("Content-Type: ([^;]+)",string(buff,asize).c_str(),PCRE_MULTILINE); //buff may not be null terminated
	if (matches.size())
		obj->m_contenttype=matches[0][1].c_str();
	matches=Regex::extract("^HTTP/\\d\\.\\d\\s(\\d+)",string(buff,asize).c_str(),PCRE_MULTILINE);
	if (matches.size())
		obj->m_statuscode=atol(matches[0][1].c_str());
	matches=Regex::extract("Last-Modified: (.+)",string(buff,asize).c_str(),PCRE_MULTILINE);
	if (matches.size())
		obj->m_lastmodified=parse(matches[0][1].c_str());
	return asize;
}
//---
int Downloader::progressfnc(void *userdata,curl_off_t dltotal,curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow){
	static double ptime=0;
	Downloader *obj=static_cast<Downloader *>(userdata);
	double time;
	curl_easy_getinfo(obj->m_curl,CURLINFO_TOTAL_TIME,&time);
	if (ptime>time || time-ptime>=0.5 || dlnow==dltotal){
		double speed;
		curl_easy_getinfo(obj->m_curl,CURLINFO_SPEED_DOWNLOAD,&speed);
		//-
		char eta_str[32];
		if (dltotal==0 || speed==0)
			sprintf(eta_str,"--:--:--");
		else{
			int64_t remaining=dltotal-dlnow;
			double eta=remaining/speed;
			if (eta>60*60*1000)
				sprintf(eta_str,"inf");
			else
				sprintf(eta_str,time_format(eta).c_str());
		}
		//-
		int64_t rdltotal=obj->m_startoffset+dltotal;
		int64_t rdlnow=obj->m_startoffset+dlnow;
		int percentage=(dltotal==0)?-1:rdlnow*100/rdltotal;
		char percentage_str[8];
		if (percentage==-1)
			sprintf(percentage_str,"--%%");
		else
			sprintf(percentage_str,"%02d%%",percentage);
		Log::log(Log::normal,false,false,"\r>%*c\r> (%s-%s) %s @ %s/s - %s , ETA : %s",
							   78,
							   ' ',
							   size_format(rdlnow).c_str(),
							   dltotal==0?size_format(0).c_str():size_format(rdltotal).c_str(),
							   percentage_str,
							   size_format(speed).c_str(),
							   time_format(time).c_str(),
							   eta_str);
		fflush(stdout);
		ptime=time;
	}
	return 0;
}
//---
template<typename T> bool Downloader::curl_setopt_internal(CURL *curl,CURLoption opt,const char *optstr,T value){
	Log::log(Log::verbose,true,true,"- setting libcurl option : %s",optstr);
	CURLcode err=curl_easy_setopt(curl,opt,value);
	if (err){
		Log::log(Log::normal,true,true,"! error while setting curl option (%s) : %d",optstr,err);
		return false;
	}
	return true;
}
//---
bool Downloader::download(const char *url,ByteArray *buffer){
	if (!init())
		return false;
	if (!curl_setopt(const char *,m_curl,CURLOPT_URL,url)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	m_remotefilename.clear();
	m_contenttype.clear();
	m_contentlen=0;
	m_startoffset=0;
	m_offset=0;
	m_fileh=NULL;
	m_buffer=buffer;
	m_buffer->cleanup();
	//-
	bool ret=true;
	Log::log(Log::verbose,true,true,"- downloading...");
	CURLcode err;
	if ((err=curl_easy_perform(m_curl))!=CURLE_OK){
		Log::log(Log::normal,true,true,"! downloading error : %s",curl_easy_strerror(err));
		ret=false;
	}else{
		if (m_statuscode!=200)
			Log::log(Log::normal,true,true,"! http code (%d) returned",m_statuscode);
	}
	curl_easy_cleanup(m_curl);
	return ret;
}
//---
bool Downloader::resume(const char *url,ByteArray *buffer){
	if (!init())
		return false;
	if (!curl_setopt(const char *,m_curl,CURLOPT_URL,url)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	m_remotefilename.clear();
	m_contenttype.clear();
	m_contentlen=0;
	m_buffer=buffer;
	m_startoffset=m_buffer->size();
	m_offset=m_startoffset;
	if (m_startoffset){
		if (!curl_setopt(curl_off_t,m_curl,CURLOPT_RESUME_FROM_LARGE,m_startoffset)){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	//-
	bool ret=true;
	Log::log(Log::verbose,true,true,"- downloading...");
	CURLcode err;
	if ((err=curl_easy_perform(m_curl))!=CURLE_OK){
		Log::log(Log::normal,true,true,"! downloading error : %s",curl_easy_strerror(err));
		ret=false;
	}else{
		if (m_statuscode!=200 && m_statuscode!=206)
			Log::log(Log::normal,true,true,"! http code (%d) returned",m_statuscode);
	}
	curl_easy_cleanup(m_curl);
	return ret;
}
//---
bool Downloader::download(const char *url,const char *file){
	if (!init())
		return false;
	if (!curl_setopt(const char *,m_curl,CURLOPT_URL,url)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	m_remotefilename.clear();
	m_contenttype.clear();
	m_contentlen=0;
	m_startoffset=0;
	m_offset=0;
	if ((m_fileh=fopen(file,"wb"))==0){
		Log::log(Log::normal,true,true,"! unable open output file for writing");
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	bool ret=true;
	Log::log(Log::verbose,true,true,"- downloading...");
	CURLcode err;
	if ((err=curl_easy_perform(m_curl))!=CURLE_OK){
		Log::log(Log::normal,true,true,"! downloading error : %s",curl_easy_strerror(err));
		ret=false;
	}else{
		if (m_statuscode!=200)
			Log::log(Log::normal,true,true,"! http code (%d) returned",m_statuscode);
	}
	fclose(m_fileh);
	m_fileh=NULL;
	curl_easy_cleanup(m_curl);
	return ret;
}
//---
bool Downloader::resume(const char *url,const char *file){
	if (!init())
		return false;
	if (!curl_setopt(const char *,m_curl,CURLOPT_URL,url)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	m_remotefilename.clear();
	m_contenttype.clear();
	m_contentlen=0;
	if((m_fileh=fopen(file,"r"))){
		fseek(m_fileh,0,SEEK_END);
		m_startoffset=ftello64(m_fileh);
		fclose(m_fileh);
	}else
		m_startoffset=0;
	m_offset=m_startoffset;
	if (m_startoffset){
		if (!curl_setopt(curl_off_t,m_curl,CURLOPT_RESUME_FROM_LARGE,m_startoffset)){
			curl_easy_cleanup(m_curl);
			return false;
		}
		if ((m_fileh=fopen(file,"ab"))==0){
			Log::log(Log::normal,true,true,"! unable open output file for writing");
			curl_easy_cleanup(m_curl);
			return false;
		}
	}else{
		if ((m_fileh=fopen(file,"wb"))==0){
			Log::log(Log::normal,true,true,"! unable open output file for writing");
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	//-
	bool ret=true;
	Log::log(Log::verbose,true,true,"- downloading...");
	CURLcode err;
	if ((err=curl_easy_perform(m_curl))!=CURLE_OK){
		Log::log(Log::normal,true,true,"! downloading error : %s",curl_easy_strerror(err));
		ret=false;
	}else{
		if (m_statuscode!=200 && m_statuscode!=206)
			Log::log(Log::normal,true,true,"! http code (%d) returned",m_statuscode);
	}
	fclose(m_fileh);
	m_fileh=NULL;
	curl_easy_cleanup(m_curl);
	return ret;
}
//---
bool Downloader::downloadIfModified(const char *url,ByteArray *buffer,int64_t time){
	if (!init())
		return false;
	if (!curl_setopt(const char *,m_curl,CURLOPT_URL,url)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	xtime r={1970,0,1,0,0,0};
    int64_t t1970=time-RFC822Time::xtime2sec(r);
	if (!curl_setopt(long,m_curl,CURLOPT_TIMEVALUE,t1970)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	if (!curl_setopt(long,m_curl,CURLOPT_TIMECONDITION,CURL_TIMECOND_IFMODSINCE)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	m_remotefilename.clear();
	m_contenttype.clear();
	m_contentlen=0;
	m_startoffset=0;
	m_offset=0;
	m_fileh=NULL;
	m_buffer=buffer;
	m_buffer->cleanup();
	//-
	bool ret=true;
	Log::log(Log::verbose,true,true,"- downloading...");
	CURLcode err;
	if ((err=curl_easy_perform(m_curl))!=CURLE_OK){
		Log::log(Log::normal,true,true,"! downloading error : %s",curl_easy_strerror(err));
		ret=false;
	}else{
		if (m_statuscode!=200 && m_statuscode!=304)
			Log::log(Log::normal,true,true,"! http code (%d) returned",m_statuscode);
	}
	curl_easy_cleanup(m_curl);
	return ret;
}
//---
bool Downloader::downloadIfModified(const char *url,const char *file,int64_t time){
	if (!init())
		return false;
	if (!curl_setopt(const char *,m_curl,CURLOPT_URL,url)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	xtime r={1970,0,1,0,0,0};
    int64_t t1970=time-RFC822Time::xtime2sec(r);
	if (!curl_setopt(long,m_curl,CURLOPT_TIMEVALUE,t1970)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	if (!curl_setopt(long,m_curl,CURLOPT_TIMECONDITION,CURL_TIMECOND_IFMODSINCE)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	m_remotefilename.clear();
	m_contenttype.clear();
	m_contentlen=0;
	m_startoffset=0;
	m_offset=0;
	if ((m_fileh=fopen(file,"wb"))==0){
		Log::log(Log::normal,true,true,"! unable open output file for writing");
		curl_easy_cleanup(m_curl);
		return false;
	}
	//-
	bool ret=true;
	Log::log(Log::verbose,true,true,"- downloading...");
	CURLcode err;
	if ((err=curl_easy_perform(m_curl))!=CURLE_OK){
		Log::log(Log::normal,true,true,"! downloading error : %s",curl_easy_strerror(err));
		ret=false;
	}else{
		if (m_statuscode!=200 && m_statuscode!=304)
			Log::log(Log::normal,true,true,"! http code (%d) returned",m_statuscode);
	}
	fclose(m_fileh);
	m_fileh=NULL;
	curl_easy_cleanup(m_curl);
	return ret;
}
//---
bool Downloader::init(){
	m_lastmodified=-1;
	m_statuscode=-1;
	//-
	Log::log(Log::verbose,true,true,"- initializing libcurl");
	m_curl=curl_easy_init();
	if (!m_curl){
		Log::log(Log::normal,true,true,"! unable to initialize libcurl");
		curl_easy_cleanup(m_curl);
		return false;
	}
	if (
		#ifdef DEBUG
		!curl_setopt(long,m_curl,CURLOPT_VERBOSE,1) ||
		#endif
		!curl_setopt(void *,m_curl,CURLOPT_HEADERFUNCTION,Downloader::headerfnc) ||
		!curl_setopt(void *,m_curl,CURLOPT_HEADERDATA,this) ||
		!curl_setopt(void *,m_curl,CURLOPT_WRITEFUNCTION,Downloader::writefnc) ||
		!curl_setopt(void *,m_curl,CURLOPT_WRITEDATA,this) ||
		!curl_setopt(void *,m_curl,CURLOPT_XFERINFOFUNCTION,Downloader::progressfnc) ||
		!curl_setopt(void *,m_curl,CURLOPT_XFERINFODATA,this) ||
		!curl_setopt(long,m_curl,CURLOPT_NOPROGRESS,0)
		){
		curl_easy_cleanup(m_curl);
		return false;
	}
	if (m_followlocation){
		if (!curl_setopt(long,m_curl,CURLOPT_FOLLOWLOCATION,1)){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	if (m_speedlimit){
		if (!curl_setopt(int64_t,m_curl,CURLOPT_MAX_RECV_SPEED_LARGE,m_speedlimit)){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	if (m_ignoresslerrors){
		if (!curl_setopt(long,m_curl,CURLOPT_SSL_VERIFYHOST,0) ||
			!curl_setopt(long,m_curl,CURLOPT_SSL_VERIFYPEER,0)){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	if (!m_useragent.empty()){
		if (!curl_setopt(char *,m_curl,CURLOPT_USERAGENT,m_useragent.c_str())){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	if (!m_cookies.empty()){
		if (!curl_setopt(char *,m_curl,CURLOPT_COOKIE,m_cookies.c_str())){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	if (m_useauth){
		if (!curl_setopt(long,m_curl,CURLOPT_HTTPAUTH,CURLAUTH_BASIC|CURLAUTH_DIGEST|CURLAUTH_DIGEST_IE|CURLAUTH_NEGOTIATE|CURLAUTH_NTLM) ||
			!curl_setopt(const char *,m_curl,CURLOPT_USERNAME,m_user.c_str()) ||
			!curl_setopt(const char *,m_curl,CURLOPT_PASSWORD,m_pass.c_str())){
			curl_easy_cleanup(m_curl);
			return false;
		}
	}
	if (m_useproxy){
		if (!curl_setopt(long,m_curl,CURLOPT_PROXYTYPE,m_proxytype) ||
			!curl_setopt(const char *,m_curl,CURLOPT_PROXY,m_proxyhost.c_str()) ||
			!curl_setopt(long,m_curl,CURLOPT_PROXYPORT,m_proxyport)){
			curl_easy_cleanup(m_curl);
			return false;
		}
		if (m_useproxyauth){
			if (!curl_setopt(long,m_curl,CURLOPT_PROXYAUTH,CURLAUTH_BASIC|CURLAUTH_DIGEST|CURLAUTH_DIGEST_IE|CURLAUTH_NEGOTIATE|CURLAUTH_NTLM) ||
				!curl_setopt(const char *,m_curl,CURLOPT_PROXYUSERNAME,m_proxyauthuser.c_str()) ||
				!curl_setopt(const char *,m_curl,CURLOPT_PROXYPASSWORD,m_proxyauthpass.c_str())){
				curl_easy_cleanup(m_curl);
				return false;
			}
		}
	}
	if (!curl_setopt(long,m_curl,CURLOPT_IPRESOLVE,m_ipversion)){
		curl_easy_cleanup(m_curl);
		return false;
	}
	return true;
}
//---
string Downloader::size_format(uint64_t size){
	char tmp[64];
	if (size>=1024*1024*1024)
		sprintf(tmp,"%0.1f GB",(float)size/(1024*1024*1024));
	else if (size>=1024*1024)
		sprintf(tmp,"%0.1f MB",(float)size/(1024*1024));
	else if (size>=1024)
		sprintf(tmp,"%0.1f KB",(float)size/1024);
	else
		sprintf(tmp,"%d B",(int)size);
	return string(tmp);
}
//---
string Downloader::time_format(uint64_t time){
	char tmp[64];
	sprintf(tmp,"%02d:%02d:%02d",(int)time/3600,(int)(time%3600)/60,(int)time%60);
	return string(tmp);
}
//---
