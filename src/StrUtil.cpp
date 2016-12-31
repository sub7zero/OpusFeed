#include "StrUtil.h"
//---
namespace StrUtil{
	string str(int64_t i){
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
	int64_t val(const string &s){
		int64_t i=0;
		string t(s);
		reverse(t.begin(),t.end());
		const char *p=t.c_str();
		int n=0;
		while(*p){
			if (*(p+1)==0 && *p=='-'){
				i*=-1;
				break;
			}
			if (*p>'9' || *p<'0')
				return 0;
			int u=*p-'0';
			i+=u*pow(10,n);
			p++;
			n++;
		}
		return i;
	}
}
