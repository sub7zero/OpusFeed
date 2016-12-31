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

#include "Log.h"
#include "Variant.h"
#include "SmartPtr.h"
using namespace std;

class AbstractParam{
	public:
		virtual ~AbstractParam(){}
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
		OptionsManager():m_active(NULL){
		}
		OptionsManager(const OptionsManager &r){
			vector<Option*>::const_iterator i=r.m_olist.begin();
			while(i!=r.m_olist.end()){
				Option *o=new Option(**i);
				m_olist.push_back(o);
				m_omap[o->name]=o;
				i++;
			}
			m_active=(r.m_active==NULL)?NULL:m_omap[r.m_active->name];
		}
		OptionsManager &operator=(OptionsManager &r){
			vector<Option*>::iterator d=m_olist.begin();
			while(d!=m_olist.end()){
				delete *d;
				d++;
			}
			m_olist.clear();
			m_omap.clear();
			vector<Option*>::const_iterator i=r.m_olist.begin();
			while(i!=r.m_olist.end()){
				Option *o=new Option(**i);
				m_olist.push_back(o);
				m_omap[o->name]=o;
				i++;
			}
			m_active=(r.m_active==NULL)?NULL:m_omap[r.m_active->name];
			return *this;
		}
		~OptionsManager(){
			vector<Option*>::iterator i=m_olist.begin();
			while(i!=m_olist.end()){
				delete *i;
				i++;
			}
		}
		void registerOption(const string &name,const SmartPtr<AbstractParam> &param,const Variant &defaultvalue,const string &description){
			Option *o=new Option;
			o->name=name;
			o->param=param;
			o->value=defaultvalue;
			o->description=description;
			m_olist.push_back(o);
			m_omap[name]=o;
		}
		void registerOption(bool required,const string &name,const SmartPtr<AbstractParam> &param,const string &description){
			Option *o=new Option;
			o->name=name;
			o->param=param;
			o->description=description;
			o->required=required;
			m_olist.push_back(o);
			m_omap[name]=o;
		}
		void registerOption(const string &name,const string &description){
			Option *o=new Option;
			o->name=name;
			o->description=description;
			m_olist.push_back(o);
			m_omap[name]=o;
		}
		void registerHeader(const string &name){
			Option *o=new Option;
			o->name=name;
			o->header=true;
			m_olist.push_back(o);
		}
		void setDependency(const string &parent,const string &child){ // ensures : (parent.value.value<bool>() == true) <-> (children.value)
			m_omap[parent]->children.push_back(child);
			m_omap[child]->parent=parent;
		}
		Variant operator[](const string &s){
			map<string,Option*>::iterator i;
			if ((i=m_omap.find(s))==m_omap.end())
				return Variant();
			return i->second->value;
		}
		void print(){
			vector<Option*>::iterator i=m_olist.begin();
			int maxlen=0;
			int maxhlen=0;
			while(i!=m_olist.end()){
				if ((*i)->header){
					int len=(*i)->name.size();
					if (len>maxhlen) maxhlen=len;
				}else{
					int len=(*i)->name.size();
					if (len>maxlen) maxlen=len;
				}
				i++;
			}
			//-
			i=m_olist.begin();
			while(i!=m_olist.end()){
				Option *o=(*i);
				if (o->header){
					cout<<std::left<<"  "<<setw(maxhlen)<<o->name<<" : "<<endl;
					i++;
					continue;
				}
				ostringstream stream;
				stream<<std::left<<"    "<<setw(maxlen)<<o->name<<" : ";
				int startat=stream.tellp();
				cout<<stream.str();
				stream.str("");
				stream.clear();
				int breakat=79;
				stream<<o->description;
				if (o->param!=NULL){
					string ops=o->param->options();
					if (!ops.empty())
						stream<<" ["<<ops<<"]";
					if (!o->value.empty())
						stream<<" (default:"<<o->param->str(o->value)<<")";
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
				i++;
			}
		}
		bool validate(){
			bool ret=true;
			vector<Option*>::iterator i=m_olist.begin();
			while(i!=m_olist.end()){
				Option *o=*i;
				if (o->header){
					i++;
					continue;
				}
				if (o->required && o->value.empty()){
					Log::log(Log::normal,true,true,"! option '%s' is required",o->name.c_str());
					ret=false;
				}
				if (o->value && !o->parent.empty()){
					map<string,Option*>::iterator u=m_omap.find(o->parent);
					if (u==m_omap.end() || !u->second->value.value<bool>()){
						Log::log(Log::normal,true,true,"! option '%s' must be set for '%s' to work",o->parent.c_str(),o->name.c_str());
						ret=false;
					}
				}
				if (!o->children.empty() && o->value.value<bool>()){
                    vector<string>::iterator l=o->children.begin();
                    while(l!=o->children.end()){
						map<string,Option*>::iterator u=m_omap.find(*l);
						if (u==m_omap.end() || !u->second->value){
							Log::log(Log::normal,true,true,"! option '%s' requires '%s' to be set",o->name.c_str(),l->c_str());
							ret=false;
						}
						l++;
                    }
				}
				i++;
			}
			if (m_active){
				Log::log(Log::normal,true,true,"! option '%s' wasn't assigned a value",m_active->name.c_str());
				ret=false;
			}
			return ret;
		}
		bool append(const string &arg){
			const char *argc=arg.c_str();
			if (m_active!=NULL){
				if (!m_active->param->valid(arg)){
					Log::log(Log::normal,true,true,"! invalid value [%s] for option '%s'",argc,m_active->name.c_str());
					m_active=NULL;
					return false;
				}
				m_active->value=m_active->param->val(arg);
				m_active=NULL;
				return true;
			}
			//-
			const char *v=NULL;
			const char *p=argc;
			while(*p){
				if (*p=='=' && *(p+1)){
					v=p+1;
					break;
				}
				p++;
			}
			if (v){
				string key(argc,v-argc-1);
				map<string,Option*>::iterator i=m_omap.find(key);
				if (i==m_omap.end()){
					Log::log(Log::normal,true,true,"! invalid option '%s'",key.c_str());
					return false;
				}
				string val(v);
				if (!i->second->param->valid(val)){
					Log::log(Log::normal,true,true,"! invalid value [%s] for option '%s'",val.c_str(),key.c_str());
					return false;
				}
				i->second->value=i->second->param->val(val);
			}else{
				string key(argc);
				map<string,Option*>::iterator i=m_omap.find(key);
				if (i==m_omap.end()){
					Log::log(Log::normal,true,true,"! invalid option '%s'",key.c_str());
					return false;
				}
				if (i->second->param!=NULL)
					m_active=i->second;
				else
					i->second->value=true;
			}
			return true;
		}
	private:
		struct Option{
			Option():param(NULL),required(false),header(false){}
			string name;
			string description;
			SmartPtr<AbstractParam> param;
            Variant value;
            bool required;
            bool header;
            //-
            vector<string> children;
            string parent;
		};
	private:
		vector<Option*> m_olist;
		map<string,Option*> m_omap;
		Option *m_active;
};
#endif // OPTIONSMANAGER_H
