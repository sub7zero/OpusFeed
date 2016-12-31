//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "ByteArray.h"
//---
ByteArray::ByteArray(){
	p_offset=0;
	p_buffsize=0;
	p_alignment=1024*512; //512 kb
	p_buff=NULL;
};
//---
ByteArray::ByteArray(const ByteArray &r){
	p_offset=0;
	p_buffsize=0;
	p_alignment=1024*512; //512 kb
	p_buff=NULL;
	append(r.buffer(),r.size());
}
//---
ByteArray::~ByteArray(){
	cleanup();
};
//---
ByteArray &ByteArray::operator=(const ByteArray &r){
	cleanup();
	append(r.buffer(),r.size());
	return *this;
}
//---
bool ByteArray::append(const char *buff,uint64_t size){
	if (p_offset+size>p_buffsize){
		int newbuffsize=(((p_offset+size)/p_alignment)+1)*p_alignment;
		char *newbuff=(char*)realloc(p_buff,newbuffsize);
		if (!newbuff)
			return false;
		p_buff=newbuff;
		p_buffsize=newbuffsize;
	}
	memcpy(&p_buff[p_offset],buff,size);
	p_offset+=size;
	return true;
}
//---
void ByteArray::cleanup(){
	if (p_buff){
		free(p_buff);
		p_buffsize=0;
		p_buff=NULL;
		p_offset=0;
	}
}
