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

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif
//---
void exitgracefully(int ecode=0){
	//-restore console color
	if (Log::getColorsEnabled())
		ConsoleColor::restoreColors();
	//-
	exit(ecode);
}
//---
#ifdef _WIN32
	bool win_ctrlhandler(DWORD type){
		if (type==CTRL_C_EVENT)
			exitgracefully(0);
		return false;
	}
#else
	void linux_ctrlhandler(int i){
		exitgracefully(0);
	}
#endif
//---
int main(int argc,char **argv){
	#ifdef _WIN32
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)win_ctrlhandler,TRUE);
	#else
		struct sigaction exithandler;
		exithandler.sa_handler=linux_ctrlhandler;
		sigemptyset(&exithandler.sa_mask);
		exithandler.sa_flags=0;
		sigaction(SIGINT,&exithandler,NULL);
	#endif
	//-
	OptionsManager options;
	//-
	SmartPtr<SetParam<long> >param_samplerates(new SetParam<long>());
	param_samplerates->addValue(48);
	param_samplerates->addValue(24);
	param_samplerates->addValue(16);
	param_samplerates->addValue(12);
	param_samplerates->addValue(8);
	SmartPtr<IntParam<long> >param_long(new IntParam<long>());
	SmartPtr<SetParam<int> >param_channels(new SetParam<int>());
	param_channels->addValue(1);
	param_channels->addValue(2);
	SmartPtr<BoolParam> param_bool(new BoolParam);
	param_bool->setStrings("yes","no");
	SmartPtr<RangeParam<int> > param_quality(new RangeParam<int>());
	param_quality->setAcceptableRange(0,10);
	SmartPtr<StrParam> param_string(new StrParam);
	SmartPtr<MapParam<Downloader::ProxyType> >param_proxytype(new MapParam<Downloader::ProxyType>());
	param_proxytype->addValue("http",Downloader::proxy_http);
	param_proxytype->addValue("socks4",Downloader::proxy_socks4);
	param_proxytype->addValue("socks4a",Downloader::proxy_socks4a);
	param_proxytype->addValue("socks5",Downloader::proxy_socks5);
	param_proxytype->addValue("socks5h",Downloader::proxy_socks5h);
	SmartPtr<MapParam<Downloader::IPVersion> >param_ipversion(new MapParam<Downloader::IPVersion>());
	param_ipversion->addValue("auto",Downloader::ip_auto);
	param_ipversion->addValue("ipv4",Downloader::ip_v4);
	param_ipversion->addValue("ipv6",Downloader::ip_v6);
	//-
	#ifdef _WIN32
		char tmpc[MAX_PATH]={0};
		GetTempPath(MAX_PATH,tmpc);
		string tmpdir=tmpc;
	#else
		string tmpdir="/tmp/";
	#endif
	//-converting options
	options.registerHeader("Converting Options");
	options.registerOption("--sample-rate",param_samplerates,48L,"sample rate in kHz");
	options.registerOption("--bit-rate",param_long,16L,"bit rate in kb/s");
	options.registerOption("--channels",param_channels,2,"number of channels");
	options.registerOption("--embed-cover",param_bool,true,"embed cover in output");
	options.registerOption("--use-video",param_bool,true,"use video to generate cover image if possible");
	options.registerOption("--cover-quality",param_quality,6,"jpeg cover quality");
	options.registerOption("--cover-width",param_long,0L,"cover width, 0 for same as input");
	options.registerOption("--cover-height",param_long,0L,"cover height, 0 for same as input");
	options.registerOption("--preserve-ar",param_bool,true,"preserve the aspect-ratio of the cover");
	//-downloading options
	options.registerHeader("Downloading Options");
	options.registerOption("--speed-limit",param_long,0L,"download speed limit in KBytes/s, 0 for unlimited");
	options.registerOption("--follow-location",param_bool,true,"follow http 3xx redirects");
	options.registerOption("--ignore-ssl",param_bool,true,"ignore ssl errors");
	options.registerOption("--user-agent",param_string,string("Mozilla/5.0"),"http user-agent string");
	options.registerOption(false,"--cookies",param_string,"semicolon separated string of 'var=value' cookies");
	options.registerOption("--use-auth",param_bool,false,"use authentication");
	options.registerOption(false,"--auth-user",param_string,"http authentication user");
	options.registerOption(false,"--auth-pass",param_string,"http authentication password");
	options.registerOption("--use-proxy",param_bool,false,"use proxy");
	options.registerOption(false,"--proxy-host",param_string,"proxy host");
	options.registerOption(false,"--proxy-port",param_long,"proxy port");
	options.registerOption("--proxy-type",param_proxytype,Downloader::proxy_http,"proxy type");
	options.registerOption("--use-proxy-auth",param_bool,false,"use authentication (for http proxies)");
	options.registerOption(false,"--proxy-auth-user",param_string,"http proxy user");
	options.registerOption(false,"--proxy-auth-pass",param_string,"http proxy password");
	options.registerOption("--download-retries",param_long,5L,"number of retries in case of a failed download");
	options.registerOption("--resume-retries",param_long,5L,"number of resume attempts before redownloading the file");
	options.registerOption("--retry-delay",param_long,10L,"delay between retries in seconds");
	options.registerOption("--ip-version",param_ipversion,Downloader::ip_auto,"ip version to be used");
	//-feed options
	options.registerHeader("Feed Options");
	options.registerOption("--preserve-failed",param_bool,true,"pass original items to the output rss feed when encountering a downloading/converting error");
	options.registerOption("--max-items",param_long,10L,"maximum number of items, 0 for unlimited");
	options.registerOption("--max-age",param_long,0L,"maximum age of items in days, 0 for unlimited");
	options.registerOption(true,"--feed-url",param_string,"url of an rss feed");
	options.registerOption(true,"--media-dir",param_string,"where to store the media files");
	options.registerOption(true,"--media-prefix",param_string,"url prefix to be used with in the generated rss feed");
	options.registerOption("--tmp-dir",param_string,tmpdir,"temporary directory where partially downloaded files are kept");
	options.registerOption(true,"--db-file",param_string,"database file for managing items");
	options.registerOption(true,"--output-rss",param_string,"path of the output rss file");
	options.registerOption("--update-interval",param_long,10L,"update interval in minutes, 0 for one shot");
	options.registerOption(false,"--exec-on-change",param_string,"command to execute on data change");
	//-logging options
	options.registerHeader("Logging Options");
	options.registerOption("--enable-colors",param_bool,true,"enable colored output");
	options.registerOption("--enable-progress",param_bool,true,"show downloading/converting progress");
	options.registerOption("--verbose","verbose output");
	options.registerOption("--quiet","suppress output");
	options.registerOption("--help","this message");
	//-
    options.setDependency("--use-proxy","--proxy-host");
    options.setDependency("--use-proxy","--proxy-port");
    options.setDependency("--use-auth","--auth-user");
    options.setDependency("--use-auth","--auth-pass");
    options.setDependency("--use-proxy-auth","--proxy-auth-user");
    options.setDependency("--use-proxy-auth","--proxy-auth-pass");
	//-
	for (int i=1;i<argc;i++){
		if (!options.append(argv[i]))
			exitgracefully(-1);
	}
	if (argc==1 || options["--help"]){
		puts("OpusFeed "OPUS_FEED_VERSION" by Alex Izeld, Email : sub7zero@hotmail.com");
		puts("Usage : opusfeed [options]");
		puts("");
		puts("Options :");
		options.print();
		puts("");
		exitgracefully();
	}
	if (!options.validate())
		exitgracefully(-1);
	if (options["--verbose"])
		Log::setLogLevel(Log::verbose);
	if (options["--quiet"])
		Log::setLogLevel(Log::quiet);
    Log::setColorsEnabled(options["--enable-colors"].value<bool>());
    Log::setProgressEnabled(options["--enable-progress"].value<bool>());
	//-
    Feed feed(options);
	exitgracefully(feed.execLoop()?0:-1);
}
