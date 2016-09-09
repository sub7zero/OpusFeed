//------------------------------------------------------------------------------------------//
// This file is part of the 'OpusFeed' project                                              //
// By : Alex Izeld                                                                          //
// Email : sub7zero@hotmail.com                                                             //
//                                                                                          //
//------------------------------------------------------------------------------------------//
//                          RENOUNCE GOD'S LIGHT AND HIS WORDS SPOKEN                       //
//                         WE REGAIN HEAVEN AND CRUSH HIS SKY WIDE OPEN                     //
//------------------------------------------------------------------------------------------//
#include "OpusConverter.h"
namespace OpusConverter{
	//---
	int err;
	//---
	class InputAVFormatContextDeleter{
		public:
			InputAVFormatContextDeleter(AVFormatContext **format_context){
				p=format_context;
			}
			~InputAVFormatContextDeleter(){
				avformat_close_input(p);
			}
		private:
			AVFormatContext **p;
	};
	//---
	class OutputAVFormatContextDeleter{
		public:
			OutputAVFormatContextDeleter(AVFormatContext *format_context){
				p=format_context;
			}
			~OutputAVFormatContextDeleter(){
				avformat_free_context(p);
			}
		private:
			AVFormatContext *p;
	};
	//---
	class AVCodecContextCloser{
		public:
			AVCodecContextCloser(AVCodecContext *codec_context){
				p=codec_context;
			}
			~AVCodecContextCloser(){
				avcodec_close(p);
			}
		private:
			AVCodecContext *p;
	};
	//---
	class AVCodecContextDeleter{
		public:
			AVCodecContextDeleter(AVCodecContext **codec_context){
				p=codec_context;
			}
			~AVCodecContextDeleter(){
				avcodec_free_context(p);
			}
		private:
			AVCodecContext **p;
	};
	//---
	class AVIOContextDeleter{
		public:
			AVIOContextDeleter(AVIOContext *io_context){
				p=io_context;
			}
			~AVIOContextDeleter(){
				avio_close(p);
			}
		private:
			AVIOContext *p;
	};
	//---
	class AVAudioResampleContextDeleter{
		public:
			AVAudioResampleContextDeleter(AVAudioResampleContext **resample_context){
				p=resample_context;
			}
			~AVAudioResampleContextDeleter(){
				avresample_close(*p);
				avresample_free(p);
			}
		private:
			AVAudioResampleContext **p;
	};
	//---
	class AVPacketDeleter{
		public:
			AVPacketDeleter(AVPacket *packet){
				p=packet;
			}
			~AVPacketDeleter(){
				av_packet_unref(p);
			}
		private:
			AVPacket *p;
	};
	//---
	class AVFrameDeleter{
		public:
			AVFrameDeleter(AVFrame **frame){
				p=frame;
			}
			~AVFrameDeleter(){
				av_frame_free(p);
			}
		private:
			AVFrame **p;
	};
	//---
	class SwsContextDeleter{
		public:
			SwsContextDeleter(SwsContext *sws_ctx){
				p=sws_ctx;
			}
			~SwsContextDeleter(){
				sws_freeContext(p);
			}
		private:
			SwsContext *p;
	};
	//---
	class AVMemoryDeleter{
		public:
			AVMemoryDeleter(void *memory){
				p=memory;
			}
			~AVMemoryDeleter(){
				av_free(p);
			}
		private:
			void *p;
	};
	//---
	inline uint32_t bigendian32(uint32_t i){
		return ((i&0xff000000)>>24)|((i&0x00ff0000)>>8)|((i&0x0000ff00)<<8)|(i<<24);
	}
	//---
	char *const averror(const int errorcode){
		static char buff[255];
		av_strerror(errorcode, buff, sizeof(buff));
		return buff;
	}
	//---
	string time_format(uint64_t time){
		char tmp[64];
		sprintf(tmp,"%02d:%02d:%02d",(int)(time/3600),(int)((time%3600)/60),(int)(time%60));
		return string(tmp);
	}
	//---
	void init(){
		av_register_all();
		#ifndef DEBUG
		av_log_set_level(AV_LOG_QUIET);
		#endif // DEBUG
	}
	//---
	bool convert(const char *ifile,const char *ofile,int samplerate,int channels,int bitrate,bool embed_cover,bool use_video,int cover_w,int cover_h,bool preserve_ar,int cover_quality){
		//-----------------------------
		// Validate options
		//-----------------------------
		log(verbose,true,false,"- validating options...");
		if (samplerate!=8000 &&
			samplerate!=12000 &&
			samplerate!=16000 &&
			samplerate!=24000 &&
			samplerate!=48000){
			log(normal,true,true,"! invalid sample-rate, supported :(8000,12000,16000,24000,48000)");
			return false;
		}
		if (channels!=1 && channels!=2){
			log(normal,true,true,"! invalid number of channels, supported : (1,2)");
			return false;
		}
		if (embed_cover){
			if (cover_quality<0 || cover_quality>10){
				log(normal,true,true,"! invalid cover art quality, quality range is [0-10]");
				return false;
			}
			if (cover_h<0 || cover_w<0){
				log(normal,true,true,"! invalid cover dimensions");
				return false;
			}
		}
		log(verbose,false,true,"ok");
		//-----------------------------
		// Decoder Setup
		//-----------------------------
		//-open file
		log(verbose,true,false,"- opening input file...");
		AVFormatContext *ifmt_ctx=NULL;
		if ((err=avformat_open_input(&ifmt_ctx,ifile,NULL,NULL))<0){
			log(normal,true,true,"! couldn't open input file : %s",averror(err));
			return false;
		}
		log(verbose,false,true,"ok");
		InputAVFormatContextDeleter ifmt_ctx_deleter(&ifmt_ctx);
		//-read stream info
		log(verbose,true,false,"- reading stream info...");
		if ((err=avformat_find_stream_info(ifmt_ctx,NULL))<0) {
			log(normal,true,true,"! couldn't find stream information: %s",averror(err));
			return false;
		}
		log(verbose,false,true,"ok");
		//-find audio stream
		log(verbose,true,false,"- looking for audio stream...");
		int iaudio_stream_idx=av_find_best_stream(ifmt_ctx,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
		if (iaudio_stream_idx<0){
			log(normal,true,true,"! no audio stream was found!");
			return false;
		}
		log(verbose,false,true,"(stream : %d)",iaudio_stream_idx);
		//-find a suitable decoder
		log(verbose,true,false,"- looking for a suitable decoder...");
		AVCodec *iaudio_codec=avcodec_find_decoder(ifmt_ctx->streams[iaudio_stream_idx]->codec->codec_id);
		if (!iaudio_codec){
			log(normal,true,true,"! couldn't find a suitable decoder for the audio stream");
			return false;
		}
		log(verbose,false,true,iaudio_codec->long_name);
		//-open the decoder
		log(verbose,true,false,"- opening decoder...");
		if ((err=avcodec_open2(ifmt_ctx->streams[iaudio_stream_idx]->codec,iaudio_codec,NULL))<0){
			log(normal,true,true,"! couldn't open input codec : %s",averror(err));
			return false;
		}
		log(verbose,false,true,"ok");
		AVCodecContext *icdc_ctx=ifmt_ctx->streams[iaudio_stream_idx]->codec;
		AVCodecContextCloser icdc_ctx_closer(icdc_ctx);
		//-----------------------------
		// Encoder Setup
		//-----------------------------
		//-open output file
		log(verbose,true,false,"- opening output file...");
		AVIOContext *io_ctx=NULL;
		if ((err=avio_open(&io_ctx,ofile,AVIO_FLAG_WRITE))<0){
			log(normal,true,true,"! couldn't open output file: %s",averror(err));
			return false;
		}
		AVIOContextDeleter io_ctx_deleter(io_ctx);
		AVFormatContext *ofmt_ctx=avformat_alloc_context();
		if (!ofmt_ctx){
			log(normal,true,true,"! couldn't allocate output format context");
			return false;
		}
		log(verbose,false,true,"ok");
		OutputAVFormatContextDeleter ofmt_ctx_deleter(ofmt_ctx);
		ofmt_ctx->pb=io_ctx;
		av_strlcpy(ofmt_ctx->filename,ofile,sizeof(ofmt_ctx->filename));
		//-set the container format for the output file to ogg
		log(verbose,true,false,"- setting output container format...");
		if (!(ofmt_ctx->oformat=av_guess_format("ogg",NULL,NULL))){
			log(normal,true,true,"! couldn't guess the output file format, ogg must be missing from libav");
			return false;
		}
		log(verbose,false,true,ofmt_ctx->oformat->long_name);
		//-find opus encoder
		log(verbose,true,false,"- looking for opus encoder...");
		AVCodec *oaudio_codec=avcodec_find_encoder(AV_CODEC_ID_OPUS);
		if (!oaudio_codec){
			log(normal,true,true,"! couldn't find encoder, opus must be missing from libav");
			return false;
		}
		log(verbose,false,true,oaudio_codec->long_name);
		//-create audio stream
		log(verbose,true,false,"- creating audio stream...");
		AVStream *oaudio_stream=avformat_new_stream(ofmt_ctx,oaudio_codec);
		if (!oaudio_stream){
			log(normal,true,true,"! couldn't create audio stream");
			return false;
		}
		int oaudio_stream_idx=oaudio_stream->index;
		log(verbose,false,true,"(stream : %d)",oaudio_stream_idx);
		AVCodecContext *ocdc_ctx=oaudio_stream->codec;
		AVCodecContextCloser ocdc_ctx_closer(ocdc_ctx);
		avcodec_get_context_defaults3(oaudio_stream->codec,oaudio_codec);
		oaudio_stream->time_base=(AVRational){1,samplerate};
		//-set encoder options
		log(verbose,true,true,"- setting encoder options");
		ocdc_ctx->channels      =channels;
		ocdc_ctx->channel_layout=av_get_default_channel_layout(channels);
		ocdc_ctx->sample_rate   =samplerate;
		ocdc_ctx->sample_fmt    =AV_SAMPLE_FMT_S16;
		ocdc_ctx->bit_rate      =bitrate;
		if (ofmt_ctx->oformat->flags&AVFMT_GLOBALHEADER)
			ocdc_ctx->flags|=CODEC_FLAG_GLOBAL_HEADER;
		//-open encoder
		log(verbose,true,false,"- opening encoder...");
		if ((err=avcodec_open2(ocdc_ctx,oaudio_codec,NULL))<0){
			log(normal,true,true,"! couldn't open the opus encoder : %s",averror(err));
			return false;
		}
		log(verbose,false,true,"ok");
		//-----------------------------
		// Resampler Setup
		//-----------------------------
		//-allocate resample context
		log(verbose,true,false,"- allocating resampler context...");
		AVAudioResampleContext *resample_ctx=avresample_alloc_context();
		if (!resample_ctx){
			log(normal,true,true,"! couldn't allocate resample context");
			return false;
		}
		log(verbose,false,true,"ok");
		AVAudioResampleContextDeleter resample_ctx_deleter(&resample_ctx);
		//-set resampler options
		log(verbose,true,true,"- setting resampler options");
		av_opt_set_int(resample_ctx,"in_channel_layout",av_get_default_channel_layout(icdc_ctx->channels),0);
		av_opt_set_int(resample_ctx,"out_channel_layout",av_get_default_channel_layout(ocdc_ctx->channels),0);
		av_opt_set_int(resample_ctx,"in_sample_rate",icdc_ctx->sample_rate,0);
		av_opt_set_int(resample_ctx,"out_sample_rate",ocdc_ctx->sample_rate,0);
		av_opt_set_int(resample_ctx,"in_sample_fmt",icdc_ctx->sample_fmt,0);
		av_opt_set_int(resample_ctx,"out_sample_fmt",ocdc_ctx->sample_fmt,0);
		//-open resampler
		log(verbose,true,false,"- opening resampler...");
		if (avresample_open(resample_ctx)<0){
			log(normal,true,true,"! couldn't open resample context");
			return false;
		}
		log(verbose,false,true,"ok");
		//-----------------------------
		// Clone meta-data
		//-----------------------------
		log(verbose,true,false,"- cloning meta-data...");
		int tagscount=0;
		AVDictionaryEntry *tag=NULL;
		while ((tag=av_dict_get(ifmt_ctx->metadata,"",tag, AV_DICT_IGNORE_SUFFIX))){
			av_dict_set(&ofmt_ctx->metadata,tag->key,tag->value,0);
			tagscount++;
		}
		log(verbose,false,true,"%d tag(s)",tagscount);
		//-----------------------------
		// Cover art
		//-----------------------------
		//important note : at the time of writing this (using libav version 11.7)
		//"libavformat/oggenc.c" has a bug in the function ogg_buffer_data(),
		//where vorbis comments with data bigger than MAX_PAGE_SIZE=65025 aren't
		//being broken down into multiple ogg pages when writing file header
		//resulting a segmentation fault when calling :
		//avformat_write_header() > ogg_write_header() > ogg_buffer_data()
		//fix (flush current page & write to a new one) :
		//in libavformat/oggenc.c, ogg_buffer_data() right after :
		//  if (i == total_segments)
		//      page->granule = granule;
		//add :
		//  if (header && page->segments_count == 255)
		//      ogg_buffer_page(s, oggstream);
		//PS. this patch is applied automatically when building libav via 'build-3rdparty.sh'
		if (embed_cover){
			int ivideo_stream_idx=-1;
			//-check if there's an embedded cover art
			log(verbose,true,false,"- checking for embedded cover-art...");
			bool embeddedart=false;
			for (unsigned int i=0;i<ifmt_ctx->nb_streams;i++){
				if (ifmt_ctx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC){
					ivideo_stream_idx=i;
					embeddedart=true;
					break;
				}
			}
			log(verbose,false,true,(ivideo_stream_idx==-1)?"no":"yes");
			//-if there's no cover art, use a video stream if there's any (if use_video is true)
			if (ivideo_stream_idx==-1){
				if (!use_video)
					goto skip_cover;
				log(verbose,true,false,"- checking for video streams...");
				ivideo_stream_idx=av_find_best_stream(ifmt_ctx,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
				if (ivideo_stream_idx<0){
					ivideo_stream_idx=-1;
					log(verbose,false,true,"no");
				}else
					log(verbose,false,true,"yes (stream : %d)",ivideo_stream_idx);

			}
			//-skip if there's neither a cover art nor a video stream available
			if (ivideo_stream_idx==-1){
				log(verbose,true,true,"! no video streams found (skipping)");
				goto skip_cover;
			}
			//-find image decoder
			log(verbose,true,false,"- looking for a suitable decoder...");
			AVCodec *ivideo_codec=avcodec_find_decoder(ifmt_ctx->streams[ivideo_stream_idx]->codec->codec_id);
			if (!ivideo_codec){
				log(verbose,true,true,"! couldn't find a valid video decoder (skipping)");
				goto skip_cover;
			}
			log(verbose,false,true,ivideo_codec->long_name);
			//-open the decoder
			log(verbose,true,false,"- opening decoder...");
			if ((err=avcodec_open2(ifmt_ctx->streams[ivideo_stream_idx]->codec,ivideo_codec,NULL))<0){
				log(verbose,true,true,"! couldn't open the video decoder (skipping) : %s",averror(err));
				goto skip_cover;
			}
			log(verbose,false,true,"ok");
			AVCodecContext *ivideo_cdc_ctx=ifmt_ctx->streams[ivideo_stream_idx]->codec;
			AVCodecContextCloser ivideo_cdc_ctx_closer(ivideo_cdc_ctx);
			//-allocate input frame
			log(verbose,true,false,"- decoding image...");
			AVFrame *iframe=av_frame_alloc();
			if (!iframe){
				log(verbose,true,true,"! couldn't allocate input frame (skipping)");
				goto skip_cover;
			}
			AVFrameDeleter iframe_deleter(&iframe);
			//-decode packet to a frame
			if (embeddedart){
				//-decode directly
				int decoded;
				if ((err=avcodec_decode_video2(ivideo_cdc_ctx,iframe,&decoded,&ifmt_ctx->streams[ivideo_stream_idx]->attached_pic))<0){
					log(verbose,true,true,"! couldn't decode image (skipping) : %s",averror(err));
					goto skip_cover;
				}
				if (!decoded)
					goto skip_cover;
			}else{
				//-seek to 10% of the input video stream
				int64_t seek=ifmt_ctx->streams[ivideo_stream_idx]->duration/10;
				if ((err=av_seek_frame(ifmt_ctx,ivideo_stream_idx,seek,0))<0){
					log(verbose,true,true,"! couldn't seek to the specified timestamp (skipping) : %s",averror(err));
					goto skip_cover;
				}
				//-keep reading packets and feed them to the decoder until one decoded frame is obtained
				int decoded=0;
				do{
					AVPacket ipacket;
					av_init_packet(&ipacket);
					ipacket.data=NULL;
					ipacket.size=0;
					AVPacketDeleter ipacket_deleter(&ipacket);
					if ((err=av_read_frame(ifmt_ctx,&ipacket))<0){
						log(verbose,true,true,"! couldn't read frame (skipping) : %s",averror(err));
						break;
					}
					if (ivideo_stream_idx!=ipacket.stream_index)
						continue;
					if ((err=avcodec_decode_video2(ivideo_cdc_ctx,iframe,&decoded,&ipacket))<0){
						log(verbose,true,true,"! couldn't decode image (skipping) : %s",averror(err));
						break;
					}
				} while(decoded==0);
				//-seek back to the beginning of the file
				av_seek_frame(ifmt_ctx,ivideo_stream_idx,0,0);
				if (decoded==0)
					goto skip_cover;
			}
			log(verbose,false,true,"ok");
			//-resize image and convert pixel format
			log(verbose,true,false,"- setting up scaling filter...");
			if (preserve_ar && cover_h==0 && cover_w==0)
				preserve_ar=false;
			if (cover_w==0) cover_w=preserve_ar?cover_h*iframe->width/iframe->height:iframe->width;
			if (cover_h==0) cover_h=preserve_ar?cover_w*iframe->height/iframe->width:iframe->height;
			//-get resizing context
			SwsContext *resize_ctx=sws_getContext(iframe->width,
														 iframe->height,
														 (AVPixelFormat)iframe->format,
														 cover_w,
														 cover_h,
														 PIX_FMT_YUVJ422P,
														 SWS_BICUBIC,
														 NULL,
														 NULL,
														 NULL);
			if (!resize_ctx){
				log(verbose,true,true,"! couldn't get resizer context (skipping)");
				goto skip_cover;
			}
			log(verbose,false,true,"ok");
			SwsContextDeleter resize_ctx_deleter(resize_ctx);
			//-allocate output frame
			log(verbose,true,false,"- resizing image to (%dx%d)...",cover_w,cover_h);
			AVFrame *oframe=av_frame_alloc();
			if (!oframe){
				log(verbose,true,true,"! couldn't allocate output (skipping)");
				goto skip_cover;
			}
			AVFrameDeleter oframe_deleter(&oframe);
			int imgsize=avpicture_get_size(PIX_FMT_YUVJ422P,cover_w,cover_h);
			uint8_t *imgbuff=(uint8_t*)av_malloc(imgsize*sizeof(uint8_t));
			if (imgbuff==NULL){
				log(verbose,true,true,"! couldn't allocate frame buffer (skipping)");
				goto skip_cover;
			}
			AVMemoryDeleter imgbuff_deleter(imgbuff);
			avpicture_fill((AVPicture*)oframe,imgbuff,PIX_FMT_YUVJ422P,cover_w,cover_h);
			//-resize frame
			sws_scale(resize_ctx,
					  iframe->data,
					  iframe->linesize,
					  0,
					  iframe->height,
					  oframe->data,
					  oframe->linesize);
			log(verbose,false,true,"ok");
			//-find jpeg encoder
			log(verbose,true,false,"- looking for jpeg encoder...");
			AVCodec *ovideo_codec=avcodec_find_encoder(AV_CODEC_ID_MJPEG);
			if (!ovideo_codec){
				log(verbose,true,true,"! couldn't find encoder, jpeg must be missing from libav (skipping)");
				goto skip_cover;
			}
			log(verbose,false,true,ovideo_codec->long_name);
			//-allocate codec context
			log(verbose,true,true,"- setting encoder options");
			AVCodecContext *ovideo_cdc_ctx=avcodec_alloc_context3(ovideo_codec);
			AVCodecContextDeleter ovideo_cdc_ctx_deleter(&ovideo_cdc_ctx);
			ovideo_cdc_ctx->pix_fmt=PIX_FMT_YUVJ422P;
			AVRational ovideo_tb={1,24};
			ovideo_cdc_ctx->time_base=ovideo_tb;
			ovideo_cdc_ctx->width=cover_w;
			ovideo_cdc_ctx->height=cover_h;
			ovideo_cdc_ctx->flags=CODEC_FLAG_QSCALE;
			ovideo_cdc_ctx->global_quality=FF_QP2LAMBDA*(ovideo_cdc_ctx->qmin+(10-cover_quality)*(ovideo_cdc_ctx->qmax-ovideo_cdc_ctx->qmin)/10);
			//-open jpeg encoder
			log(verbose,true,false,"- opening encoder...");
			if ((err=avcodec_open2(ovideo_cdc_ctx,ovideo_codec,NULL))<0){
				log(verbose,true,true,"! couldn't open the jpeg encoder (skipping) : %s",averror(err));
				goto skip_cover;
			}
			log(verbose,false,true,"ok");
			//-encode frame
			log(verbose,true,false,"- encoding image...");
			AVPacket opacket;
			av_init_packet(&opacket);
			opacket.data=NULL;
			opacket.size=0;
			AVPacketDeleter opacket_deleter(&opacket);
			oframe->quality=ovideo_cdc_ctx->global_quality;
			int encoded;
			if ((err=avcodec_encode_video2(ovideo_cdc_ctx,&opacket,oframe,&encoded))<0){
				log(verbose,true,true,"! couldn't encode packet (skipping) : %s",averror(err));
				goto skip_cover;
			}
			log(verbose,false,true,"ok");
			//-fill METADATA_BLOCK_PICTURE (https://xiph.org/flac/format.html#metadata_block_picture)
			log(verbose,true,false,"- forming vorbis comment...");
			char mimetype[]="image/jpeg";
			int mimetype_len=strlen(mimetype);
			uint64_t buffsize=8*4+                      //8 32-bit fields
							  mimetype_len+             //mime str len
							  0+                        //desc str len (description is auto filled using 'type')
							  opacket.size;             //img buffer size
			uint32_t *buff=(uint32_t*)av_malloc(buffsize);
			if (buff==NULL){
				log(verbose,true,true,"! couldn't allocate the vorbis-comment buffer (skipping)");
				goto skip_cover;
			}
			AVMemoryDeleter buff_deleter(buff);
			uint32_t *p=buff;
			//-
			*p++=bigendian32(0x03);                     //type = front cover (ID3v2)
			*p++=bigendian32(mimetype_len);             //mimetype len
			memcpy(p,mimetype,mimetype_len);            //mimetype str
			p=(uint32_t*)((uint8_t*)p+mimetype_len);
			*p++=bigendian32(0);                        //desc len
			*p++=bigendian32(0);                        //width (unused)
			*p++=bigendian32(0);                        //height (unused)
			*p++=bigendian32(0);                        //color depth (unused)
			*p++=bigendian32(0);                        //color depth for indexed imgs (unused)
			*p++=bigendian32(opacket.size);             //img buff size
			memcpy(p,opacket.data,opacket.size);        //img buff
			//-encode to base64 and write to dictionary
			int buff64size=AV_BASE64_SIZE(buffsize);
			char *buff64=(char*)av_malloc(buff64size);
			if (buff==NULL){
				log(verbose,true,true,"! couldn't allocate the base64 vorbis-comment buffer (skipping)");
				goto skip_cover;
			}
			AVMemoryDeleter buff64_deleter(buff64);
			av_base64_encode(buff64,buff64size,(uint8_t*)buff,buffsize);
			av_dict_set(&ofmt_ctx->metadata,"METADATA_BLOCK_PICTURE",buff64,0);
			log(verbose,false,true,"ok");
		}
		skip_cover:
		//-----------------------------
		// Converting
		//-----------------------------
		//-write output file header
		log(verbose,true,false,"- writing output file header...");
		if ((err=avformat_write_header(ofmt_ctx,NULL))<0){
			log(normal,true,true,"! couldn't write output file header : %s",averror(err));
			return false;
		}
		log(verbose,false,true,"ok");
		//-convert
		log(verbose,true,true,"- decoding,resampling & encoding");
		bool finished=false;
		int64_t pts=0;
		int64_t prevtime=0;
		int64_t duration=ifmt_ctx->duration/AV_TIME_BASE;
		string duration_str=time_format(duration);
		while(!finished){
			//-read from input,decode & fill resampler's fifo buffer
			while (avresample_available(resample_ctx)<ocdc_ctx->frame_size){
				//-read packet from the decoder
				AVPacket ipacket;
				av_init_packet(&ipacket);
				ipacket.data=NULL;
				ipacket.size=0;
				AVPacketDeleter ipacket_deleter(&ipacket);
				if ((err=av_read_frame(ifmt_ctx,&ipacket))<0){
					if (err==AVERROR_EOF){
						finished=true;
						break;
					}else{
						log(normal,true,true,"! couldn't read frame : %s",averror(err));
						return false;
					}
				}
				//-skip packets from other streams
				if (iaudio_stream_idx!=ipacket.stream_index)
					continue;
				//-print progress
				AVRational rescale_tb={1,1};
				int64_t time=av_rescale_q(ipacket.pts,ifmt_ctx->streams[iaudio_stream_idx]->time_base,rescale_tb);
				if (time!=prevtime){
					log(normal,false,false,"\r- (%s-%s~) %0.2d%%",time_format(time).c_str(),duration_str.c_str(),(int)(time*100/duration));
					fflush(stdout);
					prevtime=time;
				}
				//-decode audio packet & resample resulted frame(s)
				while (ipacket.size!=0){
					AVFrame *iframe=av_frame_alloc();
					if (!iframe){
						log(normal,true,true,"! could not allocate input frame");
						return false;
					}
					AVFrameDeleter iframe_deleter(&iframe);
					int decoded;
					int bytesconsumed=avcodec_decode_audio4(icdc_ctx,iframe,&decoded,&ipacket);
					if (bytesconsumed<0){
						log(verbose,true,true,"! couldn't decode packet (dropping) : %s",averror(bytesconsumed));
						//return false;
						break;
					}
					ipacket.size-=bytesconsumed;
					ipacket.data+=bytesconsumed;
					if (decoded){
						if ((err=avresample_convert(resample_ctx,NULL,0,0,iframe->extended_data,0,iframe->nb_samples))<0){
							log(normal,true,true,"! couldn't resample input packet : %s",averror(err));
							return false;
						}
					}
				}
			}
			if (finished){
				//-flush delayed data in the decoder if there's any
				int decoded;
				do{
					AVPacket ipacket;
					av_init_packet(&ipacket);
					ipacket.data=NULL;
					ipacket.size=0;
					AVPacketDeleter ipacket_deleter(&ipacket);
					AVFrame *iframe=av_frame_alloc();
					if (!iframe){
						log(normal,true,true,"! could not allocate input frame");
						return false;
					}
					AVFrameDeleter iframe_deleter(&iframe);
					avcodec_decode_audio4(icdc_ctx,iframe,&decoded,&ipacket);
					if (decoded){
						if ((err=avresample_convert(resample_ctx,NULL,0,0,iframe->extended_data,0,iframe->nb_samples))<0){
							log(normal,true,true,"! couldn't resample input packet : %s",averror(err));
							return false;
						}
					}
				} while(decoded);
				//-resample delay data in the resampling buffer
				if (avresample_get_delay(resample_ctx)){
					if ((err=avresample_convert(resample_ctx,NULL,0,0,NULL,0,0))<0){
						log(normal,true,true,"! couldn't resample input packet : %s",averror(err));
						return false;
					}
				}
			}
			//-read from resampler's buffer, encoder & write to output
			while (avresample_available(resample_ctx)>=ocdc_ctx->frame_size || (finished && avresample_available(resample_ctx)>0)){
				//-construct frame for the encoder
				AVFrame *oframe=av_frame_alloc();
				if (!oframe){
					log(normal,true,true,"! could not allocate output frame");
					return false;
				}
				AVFrameDeleter oframe_deleter(&oframe);
				oframe->nb_samples     = FFMIN(avresample_available(resample_ctx),ocdc_ctx->frame_size);
				oframe->channel_layout = ocdc_ctx->channel_layout;
				oframe->format         = ocdc_ctx->sample_fmt;
				oframe->sample_rate    = ocdc_ctx->sample_rate;
				pts+=av_rescale_q(oframe->nb_samples,ofmt_ctx->streams[oaudio_stream_idx]->codec->time_base,ofmt_ctx->streams[oaudio_stream_idx]->time_base);
				oframe->pts =pts;
				//-allocate samples buffer
				if ((err=av_frame_get_buffer(oframe,0))<0){
					log(normal,true,true,"! couldn't allocate output frame samples : %s",averror(err));
					return false;
				}
				//-fill frame with samples from the sampler's fifo buffer
				if (avresample_read(resample_ctx,(uint8_t **)oframe->data,oframe->nb_samples)<oframe->nb_samples){
					log(normal,true,true,"! could not read enough samples from the resampler");
					return false;
				}
				//-push frame to the encoder
				AVPacket opacket;
				av_init_packet(&opacket);
				opacket.data = NULL;
				opacket.size = 0;
				AVPacketDeleter opacket_deleter(&opacket);
				int encoded;
				if ((err=avcodec_encode_audio2(ocdc_ctx,&opacket,oframe,&encoded))<0){
					log(normal,true,true,"! couldn't decode packet : %s",averror(err));
					return false;
				}
				//-write to output if a packet is available
				if (encoded){
					if ((err=av_write_frame(ofmt_ctx,&opacket))<0){
						log(normal,true,true,"! couldn't write packet : %s",averror(err));
						return false;
					}
				}
			}
			if (finished){
				//-flush remaining data in the encoder when finished
				int encoded;
				do{
					AVPacket opacket;
					av_init_packet(&opacket);
					opacket.data = NULL;
					opacket.size = 0;
					AVPacketDeleter opacket_deleter(&opacket);
					if ((err=avcodec_encode_audio2(ocdc_ctx,&opacket,NULL,&encoded))<0){
						log(normal,true,true,"! couldn't decode packet : %s",averror(err));
						return false;
					}
					//-write to output if a packet is available
					if (encoded){
						if ((err=av_write_frame(ofmt_ctx,&opacket))<0){
							log(normal,true,true,"! couldn't write packet : %s",averror(err));
							return false;
						}
					}
				} while(encoded);
				log(normal,false,true,"");//linefeed after the converting status line
			}
		}
		//-write output file trailer
		log(verbose,true,false,"- writing output file trailer...");
		if ((err=av_write_trailer(ofmt_ctx))<0){
			log(normal,true,true,"! couldn't write output file trailer : %s",averror(err));
			return false;
		}
		log(verbose,false,true,"ok");
		//-----------------------------
		return true;
	}
	//---
}
