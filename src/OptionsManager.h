//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef OPTIONSMANAGER_H
#define OPTIONSMANAGER_H

#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <typeinfo>
#include "Variant.h"
using namespace std;

class AbstractParam{
	public:
		virtual string str(const Variant &v){return string();}
		virtual Variant val(const string &s){return Variant();}
		virtual bool valid(const string &s){return true;}
		virtual string options(){return string();}
};
//---
class BoolParam:public AbstractParam{
	public:
		string str(const Variant &v){
			return v.value<bool>()?p_truestr:p_falsestr;
		}
		Variant val(const string &s){
			return (s==p_truestr)?Variant(true):Variant(false);
		}
		bool valid(const string &s){
			return (s==p_truestr || s==p_falsestr);
		}
		string options(){
			ostringstream ret;
			ret<<p_truestr<<","<<p_falsestr;
			return ret.str();
		}
	public:
		BoolParam():p_truestr("true"),p_falsestr("false"){}
		void setStrings(const string &truestr,const string &falsestr){
			p_truestr=truestr;
			p_falsestr=falsestr;
		}
	private:
		string p_truestr;
		string p_falsestr;
};
//---
template<class T> class IntParam:public AbstractParam{
	public:
		string str(const Variant &v){
			ostringstream stream;
			stream<<v.value<T>();
			return stream.str();
		}
		Variant val(const string &s){
			istringstream stream(s);
			T ret;
			stream>>ret;
			return Variant(ret);
		}
};
//---
class StrParam:public AbstractParam{
	public:
		string str(const Variant &v){
			return v.value<string>();
		}
		Variant val(const string &s){
			return Variant(s);
		}
};
//---
template<class T> class RangeParam:public IntParam<T>{
	public:
		Variant val(const string &s){
			return IntParam<T>::val(s);
		}
		bool valid(const string &s){
			Variant v=val(s);
			T i=v.value<T>();
			return (i>=p_min && i<=p_max);
		}
		string options(){
			ostringstream ret;
			ret<<p_min<<"-"<<p_max;
			return ret.str();
		}
	public:
		RangeParam():p_min(0),p_max(100){}
		void setAcceptableRange(T min,T max){
			p_min=min;
			p_max=max;
		}
	private:
		T p_min;
		T p_max;
};
//---
template<class T> class SetParam:public AbstractParam{
	public:
		string str(const Variant &v){
			ostringstream stream;
			stream<<v.value<T>();
			return stream.str();
		}
		Variant val(const string &s){
			istringstream stream(s);
			T ret;
			stream>>ret;
			return Variant(ret);
		}
		bool valid(const string &s){
			Variant v=val(s);
			return (values.find(v.value<T>())!=values.end());
		}
		string options(){
			ostringstream ret;
			typename set<T>::iterator iter=values.begin();
			while (iter!=values.end()){
				ret<<str(*iter);
				if (++iter!=values.end())
					ret<<",";
			}
			return ret.str();
		}
	public:
		void addValue(const T &s){
			values.insert(s);
		}
		void delValue(const T &s){
			values.erase(s);
		}
		void clear(){
			values.clear();
		}
	private:
		set<T> values;
};
//---
template<class T> class MapParam:public AbstractParam{
	public:
		string str(const Variant &v){
			return bmap[v.value<T>()];
		}
		Variant val(const string &s){
			return Variant(fmap[s]);
		}
		bool valid(const string &s){
			return (fmap.find(s)!=fmap.end());
		}
		string options(){
			ostringstream ret;
			typename map<string,T>::iterator iter=fmap.begin();
			while (iter!=fmap.end()){
				ret<<iter->first;
				if (++iter!=fmap.end())
					ret<<",";
			}
			return ret.str();
		}
	public:
		void addValue(const string &s,const T &v){
			fmap[s]=v;
			bmap[v]=s;
		}
		void delValue(const string &s){
			bmap.erase(fmap[s]);
			fmap.erase(s);
		}
		void clear(){
			fmap.clear();
			bmap.clear();
		}
	private:
		map<string,T> fmap;
		map<T,string> bmap;
};
//---
class OptionsManager{
	public:
		OptionsManager(){
			hungry=NULL;
		}
		~OptionsManager(){
			vector<Option*>::iterator i=options_list.begin();
			while(i!=options_list.end()){
				delete *i;
				i++;
			}
		}
		void reg(const char *name,AbstractParam *type,const char *description,const Variant &defaultvalue,bool required){
			Option *o=new Option;
			o->name=name;
			o->type=type;
			o->description=description;
			o->value=defaultvalue;
			o->required=required;
			options_list.push_back(o);
			options_map[name]=o;
		}
		void reg(const char *name,const char *description){
			reg(name,NULL,description,false,false);
		}
		Variant operator[](const string &s){
			map<string,Option*>::iterator i;
			if ((i=options_map.find(s))==options_map.end())
				return Variant();
			return i->second->value;
		}
		void print(){
			vector<Option*>::iterator iter=options_list.begin();
			int maxlen=0;
			while(iter!=options_list.end()){
				int len=(*iter)->name.size();
				if (len>maxlen) maxlen=len;
				iter++;
			}
			iter=options_list.begin();
			//-
			while(iter!=options_list.end()){
				Option *o=(*iter);
				ostringstream stream;
				stream<<std::left<<"  "<<setw(maxlen)<<o->name<<" : ";
				int startat=stream.tellp();
				cout<<stream.str();
				stream.str("");
				stream.clear();
				int breakat=79;
				stream<<o->description;
				if (o->type!=NULL){
					string ops=o->type->options();
					if (!ops.empty())
						stream<<" ["<<ops<<"]";
					if (!o->value.empty())
						stream<<" (default:"<<o->type->str(o->value)<<")";
				}
				//-
				string str=stream.str();
				const char *p=str.c_str();
				const char *lastspace=NULL;
				while(*p){
					const char *c=p;
					while (*c && startat+c-p<breakat){
						if (*c=='\n')
							break;
						if (*c==' ')
							lastspace=c;
						c++;
					}
					if (*c=='\n'){
						cout.write(p,c-p);
						cout<<endl<<setw(startat)<<"";
						p=c+1;
					}else if (*c){
						if (lastspace){
							cout.write(p,lastspace-p);
							cout<<endl<<setw(startat)<<"";
							p=lastspace+1;
							lastspace=NULL;
						}else{
							cout.write(p,breakat-startat);
							cout<<endl<<setw(startat)<<"";
							p+=breakat-startat;
						}
					}else{
						cout.write(p,c-p);
						p=c;
					}
				}
				cout<<endl;
				iter++;
			}
		}
		bool append(const char *arg){
			if (hungry!=NULL){
				string val(arg);
				if (!hungry->type->valid(val)){
					cout<<"invalid value ("<<val<<") for option : "<<hungry->name<<endl;
					hungry=NULL;
					return false;
				}
				hungry->value=hungry->type->val(val);
				hungry=NULL;
				return true;
			}
			//-
			const char *v=NULL;
			const char *p=arg;
			while(*p){
				if (*p=='=' && *(p+1)){
					v=p+1;
					break;
				}
				p++;
			}
			if (v){
				string key(arg,v-arg-1);
				map<string,Option*>::iterator i=options_map.find(key);
				if (i==options_map.end()){
					cout<<"invalid option : "<<key<<endl;
					return false;
				}
				string val(v);
				if (!i->second->type->valid(val)){
					cout<<"invalid value ("<<val<<") for option : "<<key<<endl;
					return false;
				}
				i->second->value=i->second->type->val(val);
			}else{
				string key(arg);
				map<string,Option*>::iterator i=options_map.find(key);
				if (i==options_map.end()){
					cout<<"invalid option : "<<key<<endl;
					return false;
				}
				if (i->second->type!=NULL)
					hungry=i->second;
				else
					i->second->value=true;
			}
			return true;
		}
		bool validate(){
			vector<Option*>::iterator i=options_list.begin();
			while(i!=options_list.end()){
				if ((*i)->required && (*i)->value.empty()){
					cout<<"option : "<<(*i)->name<<" must be set"<<endl;
					return false;
				}
				i++;
			}
			return true;
		}
	private:
		struct Option{
			string name;
			AbstractParam *type;
			string description;
			Variant value;
			bool required;
		};
		Option *hungry;
		vector<Option*> options_list;
		map<string,Option*> options_map;
};

#endif // OPTIONSMANAGER_H
