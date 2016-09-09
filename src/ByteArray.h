//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <cstring>
#include <stdlib.h>
#include <inttypes.h>

class ByteArray{
	public:
		ByteArray();
		ByteArray(const ByteArray &r);
		~ByteArray();
		ByteArray &operator=(const ByteArray &r);
		bool append(const char *buff,uint64_t size);
		uint64_t size() const{return p_offset;}
		char *buffer() const{return p_buff;}
		void setAlignment(int i){p_alignment=i;}
		void cleanup();
	private:
		uint64_t p_offset;
		uint64_t p_buffsize;
		uint64_t p_alignment;
		char *p_buff;
};

#endif // BYTEARRAY_H
