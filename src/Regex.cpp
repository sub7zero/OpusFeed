//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "Regex.h"
//---
namespace Regex{
	match extract(const char *regex,const char *str,int flags){
		match ret;
		const char *errstr;
		int erroffset;
		pcre *re=pcre_compile(regex,flags,&errstr,&erroffset,0);
		if (!re){
			log(normal,true,true,"! invalid regex expression");
			return ret;
		}
		int matches[100];
		int offset=0;
		int len=strlen(str);
		int rc;
		while (offset<len && (rc=pcre_exec(re,0,str,len,offset,0,matches,100))>= 0){
			vector<string> m;
			for(int i=0;i<rc;++i)
				m.push_back(string(str+matches[2*i],matches[2*i+1]-matches[2*i]));
			ret.push_back(m);
			offset=matches[1];
		}
		pcre_free(re);
		return ret;
	}
}
