DIR_ROOT=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DIR_LOG=$(DIR_ROOT)/logs
DIR_3RDPARTY=$(DIR_ROOT)/3rdparty
DIR_ARCHIVES=$(DIR_ROOT)/archives
DIR_TMP=$(DIR_ROOT)/tmp
#---
export PATH:=$(DIR_3RDPARTY)/bin:$(value PATH)
export CPLUS_INCLUDE_PATH:=$(DIR_3RDPARTY)/include:$(value CPLUS_INCLUDE_PATH)
export C_INCLUDE_PATH:=$(DIR_3RDPARTY)/include:$(value C_INCLUDE_PATH)
export LIBRARY_PATH:=$(DIR_3RDPARTY)/lib:$(value LIBRARY_PATH)
export PKG_CONFIG_PATH:=$(DIR_3RDPARTY)/lib/pkgconfig
#---
opusfeed:
	rm -f $(DIR_LOG)/opusfeed.txt >/dev/null 2>&1
	cd $(DIR_TMP)/opusfeed && make install 2>&1 | tee $(DIR_LOG)/opusfeed.txt
#---
clean-3rdparty:
	- rm -rf $(DIR_TMP)/sqlite*
	- rm -rf $(DIR_TMP)/openssl*
	- rm -rf $(DIR_TMP)/curl*
	- rm -rf $(DIR_TMP)/libav*
	- rm -rf $(DIR_TMP)/pcre*
	- rm -rf $(DIR_TMP)/tinyxml2*
	- rm -rf $(DIR_TMP)/opus*
	- rm -f $(DIR_LOG)/sqlite.txt
	- rm -f $(DIR_LOG)/openssl.txt
	- rm -f $(DIR_LOG)/curl.txt
	- rm -f $(DIR_LOG)/libav.txt
	- rm -f $(DIR_LOG)/pcre.txt
	- rm -f $(DIR_LOG)/tinyxml2.txt
	- rm -f $(DIR_LOG)/opus.txt
#---
clean-opusfeed:
	- rm -rf $(DIR_TMP)/opusfeed
	- rm -f $(DIR_LOG)/opusfeed.txt
#---
clean:
	- cd $(DIR_TMP) && rm -rf *
	- cd $(DIR_LOG) && rm -rf *
#---
wipe:
	- rm -rf $(DIR_TMP)
	- rm -rf $(DIR_LOG)
	- rm -rf $(DIR_3RDPARTY)
#---
init-3rdparty: clean-3rdparty
	if [ ! -e $(DIR_TMP) ]; then mkdir $(DIR_TMP); fi;
	if [ ! -e $(DIR_LOG) ]; then mkdir $(DIR_LOG); fi;
	if [ ! -e $(DIR_3RDPARTY) ]; then mkdir $(DIR_3RDPARTY); fi;
	cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/sqlite*.tar.gz
	cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/pcre*.tar.gz
	cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/openssl*.tar.gz
	cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/curl*.tar.gz
	cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/tinyxml2*.tar.gz
	cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/opus*.tar.gz
	- cd $(DIR_TMP) && tar xvfz $(DIR_ARCHIVES)/libav*.tar.gz
	cd $(DIR_TMP)/libav* && sed -re 's/page->granule = granule;/&\
			if (header \&\& page->segments_count == 255)\
				ogg_buffer_page(s, oggstream);/' ./libavformat/oggenc.c > ./libavformat/oggenc.patched
	cd $(DIR_TMP)/libav* && mv -f ./libavformat/oggenc.patched ./libavformat/oggenc.c
#---
init-opusfeed: clean-opusfeed
	if [ ! -e $(DIR_TMP) ]; then mkdir $(DIR_TMP); fi;
	if [ ! -e $(DIR_LOG) ]; then mkdir $(DIR_LOG); fi;
	if [ ! -e $(DIR_TMP)/opusfeed ]; then mkdir $(DIR_TMP)/opusfeed; fi;
	cd $(DIR_TMP)/opusfeed && cmake -G "Unix Makefiles" ../../src/ -DCMAKE_INSTALL_PREFIX=../../
#---
init: init-3rdparty init-opusfeed
#---
3rdparty-all: 3rdparty-sqlite 3rdparty-pcre 3rdparty-openssl 3rdparty-curl 3rdparty-tinyxml2 3rdparty-opus 3rdparty-libav
#---
3rdparty-sqlite:
	rm -f $(DIR_LOG)/sqlite.txt >/dev/null 2>&1
	cd $(DIR_TMP)/sqlite* && ./configure --prefix=$(DIR_3RDPARTY) --enable-shared=no 2>&1 | tee $(DIR_LOG)/sqlite.txt
	cd $(DIR_TMP)/sqlite* && make 2>&1 | tee -a $(DIR_LOG)/sqlite.txt
	cd $(DIR_TMP)/sqlite* && make install 2>&1 | tee -a $(DIR_LOG)/sqlite.txt
#---
3rdparty-pcre:
	rm -f $(DIR_LOG)/pcre.txt >/dev/null 2>&1
	cd $(DIR_TMP)/pcre* && ./configure --prefix=$(DIR_3RDPARTY) --enable-shared=no 2>&1 | tee $(DIR_LOG)/pcre.txt
	cd $(DIR_TMP)/pcre* && make 2>&1 | tee -a $(DIR_LOG)/pcre.txt
	cd $(DIR_TMP)/pcre* && make install 2>&1 | tee -a $(DIR_LOG)/pcre.txt
#---
3rdparty-openssl:
	rm -f $(DIR_LOG)/openssl.txt >/dev/null 2>&1
	cd $(DIR_TMP)/openssl* && ./config --prefix=$(DIR_3RDPARTY) no-shared 2>&1 | tee $(DIR_LOG)/openssl.txt
	cd $(DIR_TMP)/openssl* && make 2>&1 | tee -a $(DIR_LOG)/openssl.txt
	cd $(DIR_TMP)/openssl* && make install 2>&1 | tee -a $(DIR_LOG)/openssl.txt
#---
3rdparty-curl:
	rm -f $(DIR_LOG)/curl.txt >/dev/null 2>&1
	if [ "$(shell expr substr $(shell uname -s) 1 10)" != "MINGW32_NT" ]; then \
	export LIBS="-ldl"; \
	fi; \
	cd $(DIR_TMP)/curl* && ./configure --prefix=$(DIR_3RDPARTY) --enable-shared=no --with-ssl 2>&1 | tee $(DIR_LOG)/curl.txt
	cd $(DIR_TMP)/curl* && make 2>&1 | tee -a $(DIR_LOG)/curl.txt
	cd $(DIR_TMP)/curl* && make install 2>&1 | tee -a $(DIR_LOG)/curl.txt
#---
3rdparty-tinyxml2:
	rm -f $(DIR_LOG)/tinyxml2.txt >/dev/null 2>&1
	cd $(DIR_TMP)/tinyxml2* && cmake -G "Unix Makefiles" . -DCMAKE_INSTALL_PREFIX=$(DIR_3RDPARTY) -DBUILD_SHARED_LIBS=OFF | tee $(DIR_LOG)/tinyxml2.txt
	cd $(DIR_TMP)/tinyxml2* && make | tee -a $(DIR_LOG)/tinyxml2.txt
	cd $(DIR_TMP)/tinyxml2* && make install | tee -a $(DIR_LOG)/tinyxml2.txt
#---
3rdparty-opus:
	rm -f $(DIR_LOG)/opus.txt >/dev/null 2>&1
	cd $(DIR_TMP)/opus* && ./configure --prefix=$(DIR_3RDPARTY) --enable-shared=no 2>&1 | tee $(DIR_LOG)/opus.txt
	cd $(DIR_TMP)/opus* && make 2>&1 | tee -a $(DIR_LOG)/opus.txt
	cd $(DIR_TMP)/opus* && make install 2>&1 | tee -a $(DIR_LOG)/opus.txt
#---
3rdparty-libav:
	rm -f $(DIR_LOG)/libav.txt >/dev/null 2>&1
	cd $(DIR_TMP)/libav* && sed -re 's/page->granule = granule;/&\
			if (header \&\& page->segments_count == 255)\
				ogg_buffer_page(s, oggstream);/' ./libavformat/oggenc.c > ./libavformat/oggenc.patched
	cd $(DIR_TMP)/libav* && mv -f ./libavformat/oggenc.patched ./libavformat/oggenc.c
	cd $(DIR_TMP)/libav* && ./configure --prefix=$(DIR_3RDPARTY) --disable-programs --disable-network --enable-gpl --enable-nonfree --enable-libopus 2>&1 | tee $(DIR_LOG)/libav.txt
	cd $(DIR_TMP)/libav* && make 2>&1 | tee -a $(DIR_LOG)/libav.txt
	cd $(DIR_TMP)/libav* && make install 2>&1 | tee -a $(DIR_LOG)/libav.txt