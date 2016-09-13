//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#ifndef OPUSCONVERTER_H
#define OPUSCONVERTER_H

#include <stdlib.h>
#include "Log.h"

#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avstring.h>
#include <libavutil/frame.h>
#include <libavutil/base64.h>
#include <libavutil/opt.h>
#include <libavresample/avresample.h>
#include <libswscale/swscale.h>
}

namespace OpusConverter{
	void init();
	bool convert(const char *i_file,
				 const char *o_file,
				 int samplerate,         //48000,24000,16000,12000,8000
				 int channels,
				 int bitrate,
				 bool embed_cover=true,
				 bool use_video=true,
				 int cover_w=-1,         //0 for same as the input if preserve_ar is false, calculated from cover_h otherwise
				 int cover_h=-1,         //0 for same as the input if preserve_ar is false, calculated from cover_w otherwise
				 bool preserve_ar=true,  //preserve aspect ratio, effective only if one of cover_w,cover_h is set to 0
				 int cover_quality=6     //jpeg quality [0-10]
				 );
};
#endif // OPUSCONVERTER_H
