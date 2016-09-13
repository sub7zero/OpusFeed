//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "Regex.h"
#include "ByteArray.h"
#include "RFC822Time.h"
#include "Log.h"
using namespace std;
using namespace RFC822Time;

typedef vector<vector<string> > regexmatch;
class Downloader{
	public:
		enum ProxyType{
			proxy_http=CURLPROXY_HTTP,
			proxy_socks4=CURLPROXY_SOCKS4,
			proxy_socks4a=CURLPROXY_SOCKS4A,
			proxy_socks5=CURLPROXY_SOCKS5,
			proxy_socks5h=CURLPROXY_SOCKS5_HOSTNAME
		};
		enum HTTPAuthType{
			auth_basic=CURLAUTH_BASIC,
			auth_ntlm=CURLAUTH_NTLM
		};
		enum IPVersion{
			ip_auto=CURL_IPRESOLVE_WHATEVER,
			ip_v4=CURL_IPRESOLVE_V4,
			ip_v6=CURL_IPRESOLVE_V6
		};
	public:
		Downloader();
		//-
		bool download(const char *url,const char *file);
		bool resume(const char *url,const char *file);
		bool download(const char *url,ByteArray *buffer);
		bool resume(const char *url,ByteArray *buffer);
		//-
		bool downloadIfModified(const char *url,ByteArray *buffer,int64_t time);
		bool downloadIfModified(const char *url,const char *file,int64_t time);
		//-
		void setSpeedLimit(uint64_t i){m_speedlimit=i;}                                 // 0 for unlimited
		void setFollowLocation(bool b){m_followlocation=b;}                             // default true
		void setIgnoreSslErrors(bool b){m_ignoresslerrors=b;}                           // default true
		void setUserAgent(const char *useragent){if (useragent) m_useragent=useragent;}
		void setCookies(const char *cookies){if (cookies) m_cookies=cookies;}
		void setEnableProxy(bool b){m_enableproxy=b;}                                   // default false
		void setProxyHost(const char *host){if (host) m_proxyhost=host;}
		void setProxyPort(int port){m_proxyport=port;}
		void setProxyType(ProxyType proxytype){m_proxytype=proxytype;}                  // default http
		void setAuthType(HTTPAuthType authtype){m_authtype=authtype;}                   // default basic
		void setAuthUser(const char *user){if (user) m_authuser=user;}
		void setAuthPass(const char *pass){if (pass) m_authpass=pass;}
		void setIPVersion(IPVersion ver){m_ipversion=ver;}
		//-
		string getRemoteFileName(){return m_remotefilename;}
		int64_t getDownloadedBytes(){return m_offset;}
		int64_t getContentLength(){return m_contentlen;}
		long getStatusCode(){return m_statuscode;}
		int64_t getLastModified(){return m_lastmodified;}
	private:
		bool init();
		static size_t headerfnc(char *buff,size_t size,size_t nitems,void *userdata);
		static size_t writefnc(char *buff,size_t size,size_t nitems,void *userdata);
		static int progressfnc(void *userdata,curl_off_t dltotal,curl_off_t dlnow,curl_off_t ultotal,curl_off_t ulnow);
		static string size_format(uint64_t size);
		static string time_format(uint64_t time);
		#define curl_setopt(type,curl,opt,value) curl_setopt_internal<type>(curl,opt,#opt,(type)value)
		template<typename T> static bool curl_setopt_internal(CURL *curl,CURLoption opt,const char *optstr,T value);
	private:
		string m_remotefilename;
		int64_t m_contentlen;
		int64_t m_offset;
		int64_t m_startoffset;
		FILE *m_fileh;
		ByteArray *m_buffer;
		CURL *m_curl;
		long m_statuscode;
		int64_t m_lastmodified;
		//-
		bool m_followlocation;
		int64_t m_speedlimit;
		bool m_ignoresslerrors;
		string m_useragent;
		string m_cookies;
		bool m_enableproxy;
		string m_proxyhost;
		int m_proxyport;
		ProxyType m_proxytype;
		HTTPAuthType m_authtype;
		string m_authuser;
		string m_authpass;
		IPVersion m_ipversion;
};

#endif // DOWNLOADER_H
