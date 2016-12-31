## OpusFeed 1.4 ##

**opusfeed** is a cross-platform software for automatically generating a low-quality opus rss feed from another (audio or video) rss feed.

**The file format of the generated audio files  :**

- container format : ogg
- audio codec : opus
- cover codec : jpeg


### Dependencies : ###
- zlib
- libopus
- libav
- pcre
- sqlite
- tinyxml2
- libcurl
- openssl

### Building environment under Windows : ###
- msys
- mingw
- cmake (>=3.0)
- yasm

### Building environment under Linux : ###
- gcc,g++
- pkg-config
- cmake (>=3.0)
- yasm

### Building steps : ###
cd to the directory where you downloaded the source and run the following commands:

	$ make init
	$ make 3rdparty
	$ make opusfeed
the first command will extract and prepare the dependencies in `./tmp/` each to a separate folder and the project's makefiles in `./tmp/opusfeed/`.  
the second line will build the dependencies and install them to `./3rdparty/`, log files will be stored in `./logs`.  
the third line will build **opusfeed** and install the final binary to `./bin/`.

### Usage : ###
opusfeed [options]  
the software accepts options in these two forms :  

 -  option=value, eg :

		$ ./opusfeed option1=value1 option2=value2 ...
 -  option value, eg :

		$ ./opusfeed option1 value1 option2 value2 ...

the options (`--quiet`,`--verbose` & `--help`) don't take values.

at least the following options must be set :

- `--media-dir`
- `--feed-url`
- `--media-prefix`
- `--db-file`
- `--output-rss`

### Options : ###

<table>
	<tr>
		<th width="200">option</th>
		<th>descrription</th>
	</tr>
    <tr><td>--sample-rate</td><td>sample rate in kHz [8,12,16,24,48] (default:48)</td></tr>
    <tr><td>--bit-rate</td><td>bit rate in kb/s (default:16)</td></tr>
    <tr><td>--channels</td><td>number of channels [1,2] (default:2)</td></tr>
    <tr><td>--embed-cover</td><td>embed cover in output [yes,no] (default:yes)</td></tr>
    <tr><td>--use-video</td><td>use video to generate cover image if possible [yes,no] (default:yes)</td></tr>
    <tr><td>--cover-quality</td><td>jpeg cover quality [0-10] (default:6)</td></tr>
    <tr><td>--cover-width</td><td>cover width, 0 for same as input (default:0)</td></tr>
    <tr><td>--cover-height</td><td>cover height, 0 for same as input (default:0)</td></tr>
    <tr><td>--preserve-ar</td><td>preserve the aspect-ratio of the cover [yes,no] (default:yes)</td></tr>
    <tr><td>--speed-limit</td><td>download speed limit in KBytes/s, 0 for unlimited (default:0)</td></tr>
    <tr><td>--follow-location</td><td>follow http 3xx redirects [yes,no] (default:yes)</td></tr>
    <tr><td>--ignore-ssl</td><td>ignore ssl errors [yes,no] (default:yes)</td></tr>
    <tr><td>--user-agent</td><td>http user-agent string (default:Mozilla/5.0)</td></tr>
    <tr><td>--cookies</td><td>semicolon separated string of 'var=value' cookies</td></tr>
    <tr><td>--use-auth</td><td>use authentication [yes,no] (default:no)</td></tr>
    <tr><td>--auth-user</td><td>user</td></tr>
    <tr><td>--auth-pass</td><td>password</td></tr>
    <tr><td>--use-proxy</td><td>use proxy [yes,no] (default:no)</td></tr>
    <tr><td>--proxy-host</td><td>proxy host</td></tr>
    <tr><td>--proxy-port</td><td>proxy port</td></tr>
    <tr><td>--proxy-type</td><td>proxy type [http,socks4,socks4a,socks5,socks5h] (default:http)</td></tr>
    <tr><td>--use-proxy-auth</td><td>use authentication (for http proxies) [yes,no] (default:no)</td></tr>
    <tr><td>--proxy-auth-user</td><td>http proxy user</td></tr>
    <tr><td>--proxy-auth-pass</td><td>http proxy password</td></tr>
    <tr><td>--download-retries</td><td>number of retries in case of a failed download (default:5)</td></tr>
    <tr><td>--resume-retries</td><td>number of resume attempts before redownloading the file (default:5)</td></tr>
    <tr><td>--retry-delay</td><td>delay between retries in seconds (default:10)</td></tr>
    <tr><td>--ip-version</td><td>ip version to be used [auto,ipv4,ipv6] (default:auto)</td></tr>
    <tr><td>--preserve-failed</td><td>pass original items to the output rss feed when encountering a downloading/converting error [yes,no] (default:yes)</td></tr>
    <tr><td>--max-items</td><td>maximum number of items, 0 for unlimited (default:10)</td></tr>
    <tr><td>--max-age</td><td>maximum age of items in days, 0 for unlimited (default:0)</td></tr>
    <tr><td>--feed-url</td><td>url of an rss feed</td></tr>
    <tr><td>--media-dir</td><td>where to store the media files</td></tr>
    <tr><td>--media-prefix</td><td>url prefix to be used with in the generated rss feed</td></tr>
    <tr><td>--tmp-dir</td><td>temporary directory where partially downloaded files are kept (default:/tmp/)</td></tr>
    <tr><td>--db-file</td><td>database file for managing items</td></tr>
    <tr><td>--output-rss</td><td>path of the output rss file</td></tr>
    <tr><td>--update-interval</td><td>update interval in minutes, 0 for one shot (default:10)</td></tr>
    <tr><td>--exec-on-change</td><td>command to execute on data change</td></tr>
    <tr><td>--enable-colors</td><td>enable colored output [yes,no] (default:yes)</td></tr>
    <tr><td>--enable-progress</td><td>show downloading/converting progress [yes,no] (default:yes)</td></tr>
    <tr><td>--verbose</td><td>verbose output</td></tr>
    <tr><td>--quiet</td><td>suppress output</td></tr>
    <tr><td>--help</td><td>this message</td></tr>
</table>

### Notes : ###

- **Migrating from version 1.3 :**
	
	if you don't wish to rebuild your feed from scratch, run `sqlite3 your-v1.3-db-file.db` and then execute the following statements :

	```sql
	alter table items add column downloaded text;
	alter table items add column converted text;
	alter table items add column moddate integer;
	alter table items add column originalsize integer;
	alter table items add column originalmimetype text;
	update items set downloaded="true",converted="true",originalmimetype="";
	```

- **The media directory `--media-dir` :**

	The media directory is managed completely by the software, make sure it's not set to a shared folder containing other files.

- **Force updating a specific item :**

	delete the corresponding file in `--media-dir`.
	
- **Force the regeneration of the output rss file :**
	
	The output rss file is regenerated automatically whenever the local data changes or when a related parameter is updated (--preserve-failed & --media-prefix), or when the output file doesn't exist...so if for some reason you wanted to force the software to rewrite the output rss file, just delete it.

- **Other make commands :**

	-  init dependencies only :

			$ make init-3rdparty  
	-  init opusfeed only :

			$ make init-opusfeed  
	-  clean the dependencies' build files & logs :

			$ make clean-3rdparty  
	-  clean opusfeed's build files & log :

			$ make clean-opusfeed 
	-  clean all build files & logs :

			$ make clean  
	-  wipe everything except the opusfeed binary (this will delete built dependencies) :

			$ make wipe  
	-  build sqlite only :

			$ make 3rdparty-sqlite 
	-  build pcre only :

			$ make 3rdparty-pcre 
	-  build openssl only :

			$ make 3rdparty-openssl
	-  build curl only (requires openssl) :

			$ make 3rdparty-curl
	-  build tinyxml2 only :

			$ make 3rdparty-tinyxml2
	-  build opus only :

			$ make 3rdparty-opus
	-  build libav only (requires opus) :

			$ make 3rdparty-libav

### About : ###
By : *Alex Izeld*  
Email : *sub7zero@hotmail.com*