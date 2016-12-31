//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef VARIANT_H
#define VARIANT_H

#include <typeinfo>
#include <stdlib.h>
using namespace std;

class Variant{
	private:
		template<class T> static void* allocate(const T &var){
			return new(new(malloc(sizeof(Base)+sizeof(T))) DynBase<T>()+1) T(var);
		}
		struct Base{
			Base(){}
			virtual ~Base(){}
			virtual void *clone()=0;
			virtual const type_info& type() const=0;
			void *variable(){return this+1;}
		};
		template<class T> struct DynBase:public Base{
			DynBase(){}
			virtual ~DynBase() {((T*)variable())->~T();}
			virtual void *clone() {return allocate<T>(*(const T*)variable());}
			virtual const type_info& type() const {return typeid(T);}
		};
	public:
		Variant():data(NULL){}
		Variant(const Variant &r):data(r.clone()){}
		template<class T> Variant(const T &v):data(allocate<T>(v)){}
		~Variant(){clear();}
		Variant &operator=(const Variant &r){
			clear();
			data=r.clone();
			return *this;
		}
		template<class T>Variant &operator=(const T &v){
			if (data && type()==typeid(T))
				*(T*)data=v;
			else{
				clear();
				data=allocate<T>(v);
			}
			return *this;
		}
		operator bool(){
			return data!=NULL;
		}
		const type_info &type() const{
			if (!data)
				throw bad_typeid();
			return base()->type();
		}
		bool empty() const{
			return data==NULL;
		}
		template<class T> const T &value() const{
			if (!data || type()!=typeid(T))
				throw bad_cast();
			return *(T*)data;
		}
		void clear(){
			if(data){
				Base *b=base();
				b->~Base();
				free(b);
				data=NULL;
			}
		}
	private:
		Base *base() const{return (Base*)data-1;}
		void *clone() const{return data?base()->clone():NULL;}
	private:
		void *data;
};



#endif // VARIANT_H
