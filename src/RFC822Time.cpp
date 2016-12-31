//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "RFC822Time.h"
//---
namespace RFC822Time{
	//---
	struct tz{
		char name[5];
		int offset;
	};
	const struct tz timezones[]= {
		{"GMT",0},      {"UTC",0},       {"WET",0},      {"BST",0-60},    {"WAT",60},     {"AST",240},
		{"ADT",240-60}, {"EST",300},     {"EDT",300-60}, {"CST",360},     {"CDT",360-60}, {"MST",420},
		{"MDT",420-60}, {"PST",480},     {"PDT",480-60}, {"YST",540},     {"YDT",540-60}, {"HST",600},
		{"HDT",600-60}, {"CAT",600},     {"AHST",600},   {"NT",660},      {"IDLW",720},   {"CET",-60},
		{"MET",-60},    {"MEWT",-60},    {"MEST",-60-60},{"CEST",-60-60}, {"MESZ",-60-60},{"FWT",-60},
		{"FST",-60-60}, {"EET",-120},    {"WAST",-420},  {"WADT",-420-60},{"CCT",-480},   {"JST",-540},
		{"EAST",-600},  {"EADT",-600-60},{"GST",-600},   {"NZT",-720},    {"NZST",-720},  {"NZDT",-720-60},
		{"IDLE",-720},  {"A",+1*60},     {"B",+2*60},    {"C",+3*60},     {"D",+4*60},    {"E",+5*60},
		{"F",+6*60},    {"G",+7*60},     {"H",+8*60},    {"I",+9*60},     {"K",+10*60},   {"L",+11*60},
		{"M",+12*60},   {"N",-1*60},     {"O",-2*60},    {"P",-3*60},     {"Q",-4*60},    {"R",-5*60},
		{"S",-6*60},    {"T",-7*60},     {"U",-8*60},    {"V",-9*60},     {"W",-10*60},   {"X",-11*60},
		{"Y",-12*60},   {"Z",0}
	};
	const char *const weekdays[]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	const char *const months[]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	//---
	int month(const char *str){
		for (int i=0; i<12; i++)
			if (strcmp(str,months[i])==0)
				return i;
		return -1;
	}
	//---
	int timezone(const char *str){
		for (unsigned int i=0;i<sizeof(timezones)/sizeof(tz);i++)
			if (strcmp(str,timezones[i].name)==0)
				return timezones[i].offset;
		return 0;
	}
	//---
	int64_t xtime2sec(xtime t){
		int64_t s=(t.h*60+t.i)*60+t.s;
		int64_t d=t.d+s/(60*60*24);
		s=s%(60*60*24);
		if (s<0){
			s+=60*60*24;
			d--;
		}
		int64_t y=t.y+t.m/12;
		int64_t m=t.m%12;
		if (m<0){
			m+=12;
			y--;
		}
		m=(m+10)%12;
		y=y-m/10;
		d=y*365+y/4-y/100+y/400+(m*306+5)/10+(d-1);
		return d*24*60*60+s;
	}
	//---
	xtime sec2xtime(int64_t t){
		int64_t s=t%(24*60*60);
		int64_t d=t/(24*60*60);
		int64_t y=(10000*d+14780)/3652425;
		int64_t dd=d-(365*y+y/4-y/100+y/400);
		if (dd<0){
			y--;
			dd=d-(365*y+y/4-y/100+y/400);
		}
		int64_t mi=(100*dd+52)/3060;
		int64_t m=(mi+2)%12;
		y=y+(mi+2)/12;
		d=dd-(mi*306+5)/10+1;
		xtime ret={(int)y,(int)m,(int)d,(int)(s/3600),(int)((s%3600)/60),(int)(s%60)};
		return ret;
	}
	//---
	int dayofweek(int y,int m,int d){
		//0=Sun,...
		static int t[]={0,3,2,5,0,3,5,1,4,6,2,4};
		y-=m<2;
		return (y+y/4-y/100+y/400+t[m]+d)%7;
	}
	//---
	int64_t nowgmt(){
		time_t n=time(NULL);
		struct tm *t=gmtime(&n);
		if (t==NULL)
			return -1;
		xtime x={
			t->tm_year+1900,
			t->tm_mon,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_sec
		};
		return xtime2sec(x);
	}
	//---
	string format(int64_t gmt,const char *timez,int offset){
		xtime t=sec2xtime(gmt);
		string timezstr;
		if (timez!=NULL){
			t.i+=timezone(timez);
			timezstr=timez;
		}else{
			t.i+=offset;
			char sign=offset>=0?'+':'-';
			offset=abs(offset);
			char z[6];
			sprintf(z,"%c%02d%02d",sign,offset/60,offset%60);
			timezstr=z;
		}
		t=sec2xtime(xtime2sec(t));
		char ret[512];
		sprintf(ret,"%s, %02d %s %04d %02d:%02d:%02d %s",
					weekdays[dayofweek(t.y,t.m,t.d)],
					t.d,
					months[t.m],
					t.y,
					t.h,
					t.i,
					t.s,
					timezstr.c_str());
		return string(ret);
	}
	//---
	int64_t parse(const char *str){
		Regex::match matches=Regex::extract("(\\S+,\\s+){0,1}(\\d{1,2})\\s+(\\S{3})\\s+(\\d{2}|\\d{4})\\s+([\\d:]+)\\s+(\\S+)",str,0);
		if (matches.size()){
			int d=atoi(matches[0][2].c_str());
			int m=month(matches[0][3].c_str());
			if (m==-1) return -1;
			int y=atoi(matches[0][4].c_str());
			if (y<100) y+=1900;
			int h,i,s;
			sscanf(matches[0][5].c_str(),"%d:%d:%d",&h,&i,&s);
			int off;
			if (matches[0][6][0]=='+' || matches[0][6][0]=='-'){
				int oh=atoi(string(&matches[0][6][1],2).c_str());
				int oi=atoi(&matches[0][6][3]);
				off=(matches[0][6][0]=='+')?oh*60+oi:-(oh*60+oi);
			}else
				off=timezone(matches[0][6].c_str());
			xtime t={y,m,d,h,i-off,s};
			t=sec2xtime(xtime2sec(t));
			return xtime2sec(t);
		}
		return -1;
	}
}
