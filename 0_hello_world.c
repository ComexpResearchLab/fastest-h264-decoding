/*
 * http://ffmpeg.org/doxygen/trunk/index.html
 *
 * Main components
 *
 * Format (Container) - a wrapper, providing sync, metadata and muxing for the streams.
 * Stream - a continuous stream (audio or video) of data over time.
 * Codec - defines how data are enCOded (from Frame to Packet)
 *        and DECoded (from Packet to Frame).
 * Packet - are the data (kind of slices of the stream data) to be decoded as raw frames.
 * Frame - a decoded raw frame (to be encoded or filtered).
 */

 #include <libavcodec/avcodec.h>
 #include <libavformat/avformat.h>
 #include <stdio.h>
 #include <stdarg.h>
 #include <stdlib.h>
 #include <string.h>
 #include <inttypes.h>
 #include <png.h>
#include <libswscale/swscale.h>
 
 // print out the steps and errors
 static void logging(const char *fmt, ...);
 // decode packets into frames
 static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, long long int save_f);
 // save a frame into a .pgm file
 static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);
 static int save_frame_to_png(AVFrame *frame, const char *filename);
 int save_frame_to_png(AVFrame *frame, const char *filename)
 {
  struct SwsContext *sws_ctx = sws_getContext(
    frame->width, frame->height, frame->format,
    frame->width, frame->height, AV_PIX_FMT_RGB24,
      SWS_BILINEAR, NULL, NULL, NULL);

  // Allocate a new AVFrame for the output RGB24 image
  AVFrame* rgb_frame = av_frame_alloc();

  // Set the properties of the output AVFrame
  rgb_frame->format = AV_PIX_FMT_RGB24;
  rgb_frame->width = frame->width;
  rgb_frame->height = frame->height;

  int ret = av_frame_get_buffer(rgb_frame, 0);
  if (ret < 0) {
      logging("Error while preparing RGB frame: %s", av_err2str(ret));
      return ret;
  }

  logging("Transforming frame format from YUV420P into RGB24...");
  ret = sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, rgb_frame->data, rgb_frame->linesize);
  if (ret < 0) {
      logging("Error while translating the frame format from YUV420P into RGB24: %s", av_err2str(ret));
      return ret;
  }



  

     
 
     logging("Creating PNG file -> %s", filename);
 
     // Open the PNG file for writing
     FILE *fp = fopen(filename, "wb");
     if (!fp) {
         fprintf(stderr, "Failed to open file '%s'\n", filename);
         return -1;
     }
 
     // Create the PNG write struct and info struct
     png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
     if (!png_ptr) {
         fprintf(stderr, "Failed to create PNG write struct\n");
         fclose(fp);
         return -1;
     }
 
     png_infop info_ptr = png_create_info_struct(png_ptr);
     if (!info_ptr) {
         fprintf(stderr, "Failed to create PNG info struct\n");
         png_destroy_write_struct(&png_ptr, NULL);
         fclose(fp);
         return -1;
     }
 
     // Set up error handling for libpng
     if (setjmp(png_jmpbuf(png_ptr))) {
         fprintf(stderr, "Error writing PNG file\n");
         png_destroy_write_struct(&png_ptr, &info_ptr);
         fclose(fp);
         return -1;
     }
 
     // Set the PNG file as the output for libpng
     png_init_io(png_ptr, fp);
 
     // Set the PNG image attributes
     png_set_IHDR(png_ptr, info_ptr, rgb_frame->width, rgb_frame->height, 8, PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
 
     // Allocate memory for the row pointers and fill them with the AVFrame data
     png_bytep *row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * rgb_frame->height);
     for (int y = 0; y < rgb_frame->height; y++) {
         row_pointers[y] = (png_bytep) (rgb_frame->data[0] + y * rgb_frame->linesize[0]);
     }
 
     // Write the PNG file
     png_set_rows(png_ptr, info_ptr, row_pointers);
     png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
 
     // Clean up
     free(row_pointers);
     png_destroy_write_struct(&png_ptr, &info_ptr);
     fclose(fp);
     av_frame_free(&rgb_frame);
     return ret;
 }
 
 int main(int argc, const char *argv[])
 {
   if (argc < 2)
   {
     printf("You need to specify a media file.\n");
     return -1;
   }
 
   logging("initializing all the containers, codecs and protocols.");
 
   // AVFormatContext holds the header information from the format (Container)
   // Allocating memory for this component
   // http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
   AVFormatContext *pFormatContext = avformat_alloc_context();
   if (!pFormatContext)
   {
     logging("ERROR could not allocate memory for Format Context");
     return -1;
   }
 
   logging("opening the input file (%s) and loading format (container) header", argv[1]);
   // Open the file and read its header. The codecs are not opened.
   // The function arguments are:
   // AVFormatContext (the component we allocated memory for),
   // url (filename),
   // AVInputFormat (if you pass NULL it'll do the auto detect)
   // and AVDictionary (which are options to the demuxer)
   // http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
   if (avformat_open_input(&pFormatContext, argv[1], NULL, NULL) != 0)
   {
     logging("ERROR could not open the file");
     return -1;
   }
 
   // now we have access to some information about our file
   // since we read its header we can say what format (container) it's
   // and some other information related to the format itself.
   logging("format %s, duration %lld us, bit_rate %lld", pFormatContext->iformat->name, pFormatContext->duration, pFormatContext->bit_rate);
 
   logging("finding stream info from format");
   // read Packets from the Format to get stream information
   // this function populates pFormatContext->streams
   // (of size equals to pFormatContext->nb_streams)
   // the arguments are:
   // the AVFormatContext
   // and options contains options for codec corresponding to i-th stream.
   // On return each dictionary will be filled with options that were not found.
   // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
   if (avformat_find_stream_info(pFormatContext, NULL) < 0)
   {
     logging("ERROR could not get the stream info");
     return -1;
   }
 
   // the component that knows how to enCOde and DECode the stream
   // it's the codec (audio or video)
   // http://ffmpeg.org/doxygen/trunk/structAVCodec.html
   AVCodec *pCodec = NULL;
   // this component describes the properties of a codec used by the stream i
   // https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
   AVCodecParameters *pCodecParameters = NULL;
   int video_stream_index = -1;
 
   // loop though all the streams and print its main information
   for (int i = 0; i < pFormatContext->nb_streams; i++)
   {
 
     AVCodecParameters *pLocalCodecParameters = NULL;
     pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
     logging("AVStream->time_base before open coded %d/%d", pFormatContext->streams[i]->time_base.num, pFormatContext->streams[i]->time_base.den);
     logging("AVStream->r_frame_rate before open coded %d/%d", pFormatContext->streams[i]->r_frame_rate.num, pFormatContext->streams[i]->r_frame_rate.den);
     logging("AVStream->start_time %" PRId64, pFormatContext->streams[i]->start_time);
     logging("AVStream->duration %" PRId64, pFormatContext->streams[i]->duration);
 
     logging("finding the proper decoder (CODEC)");
 
     AVCodec *pLocalCodec = NULL;
 
     // finds the registered decoder for a codec ID
     // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
     pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
     printf("cid: %s\n", pLocalCodec->long_name);
     if (pLocalCodec == NULL)
     {
       logging("ERROR unsupported codec!");
       // In this example if the codec is not found we just skip it
       continue;
     }
     
     // when the stream is a video we store its index, codec parameters and codec
     if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
     {
       if (video_stream_index == -1)
       {
         video_stream_index = i;
         pCodec = pLocalCodec;
         pCodecParameters = pLocalCodecParameters;
 
         printf("cn: %s\n", pLocalCodec->long_name);
       }
 
       logging("Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
     }
     else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO)
     {
       logging("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->ch_layout.nb_channels, pLocalCodecParameters->sample_rate);
     }
 
     // print its name, id and bitrate
     logging("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
   }
 
   if (video_stream_index == -1)
   {
     logging("File %s does not contain a video stream!", argv[1]);
     return -1;
   }
 
   // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
   AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
   if (!pCodecContext)
   {
     logging("failed to allocated memory for AVCodecContext");
     return -1;
   }
 
   // Fill the codec context based on the values from the supplied codec parameters
   // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
 
   if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
   {
     logging("failed to copy codec params to codec context");
     return -1;
   }
 
   AVDictionary * codec_options = NULL;
   av_dict_set(&codec_options, "preset", "ultrafast", 0);
 
 
 
 
   // Initialize the AVCodecContext to use the given AVCodec.
   // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
   if (avcodec_open2(pCodecContext, pCodec, &codec_options) < 0)
   {
     logging("failed to open codec through avcodec_open2");
     return -1;
   }
 
 
   // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
   AVFrame *pFrame = av_frame_alloc();
   if (!pFrame)
   {
     logging("failed to allocate memory for AVFrame");
     return -1;
   }
   // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
   AVPacket *pPacket = av_packet_alloc();
   if (!pPacket)
  {
    logging("failed to allocate memory for AVPacket");
    return -1;
  }

  int response = 0;
  int how_many_packets_to_process = 8;

  // fill the Packet with data from the Stream
  // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
  long long int counter = 0;
  while (av_read_frame(pFormatContext, pPacket) >= 0)
  {

    // if it's the video stream
    if (pPacket->stream_index == video_stream_index)
    {
      counter++;
      // logging("AVPacket->pts %" PRId64, pPacket->pts);

      response = decode_packet(pPacket, pCodecContext, pFrame, counter);
      if (response < 0)
        break;

      // stop it, otherwise we'll be saving hundreds of frames
      // if (--how_many_packets_to_process <= 0) break;
    }
    // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
    av_packet_unref(pPacket);
  }

  logging("releasing all the resources");

  avformat_close_input(&pFormatContext);
  av_packet_free(&pPacket);
  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecContext);
  return 0;
}

static void logging(const char *fmt, ...)
{
  va_list args;
  fprintf(stderr, "LOG: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

static int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, long long int save_f)
{
  // static int print_frames = 0;
  static int print_frames = 1;
  // Supply raw packet data as input to a decoder
  // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
  int response = avcodec_send_packet(pCodecContext, pPacket);

  if (response < 0)
  {
    logging("Error while sending a packet to the decoder: %s", av_err2str(response));
    return response;
  }
  long long unsigned int counter = 1;
  while (response >= 0)
  {
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
    {
      break;
    }
    else if (response < 0)
    {
      logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
      return response;
    }

    if (response >= 0)
    {
      // success

      // logging(
      //     "Frame %d (type=%c, size=%d bytes, format=%d) pts %d key_frame %d ");
      //     pCodecContext->frame_num,
      //     av_get_picture_type_char(pFrame->pict_type),
      //    pFrame->pkt_size,
      //    pFrame->format,
      //    pFrame->pts,
      //    pFrame->key_frame);

      char frame_filename[1024];
      char png_filename[1024];
      snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_num);
      snprintf(png_filename, sizeof(png_filename), "temp/%s-%d.png", "frame", pCodecContext->frame_num);
      if(print_frames && pCodecContext->frame_num>2000){return -1;}
      // save a grayscale frame into a .pgm file
      if (
        print_frames && 
        1)
           save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
           save_frame_to_png(pFrame, png_filename);
    }
  }
  return 0;
}

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{


  FILE *f;
  int i;
  const char fname[1024];
  memset(fname, 0, sizeof(fname));
  strcat(fname, "temp/");
  strcat(fname, filename);
  printf("%s\n", fname);
  printf("cmdbench point\n");
  fflush(stdout);
  f = fopen(fname, "w");
  // writing the minimal required header for a pgm file format
  // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
  fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

  // writing line by line
  for (i = 0; i < ysize; i++)
    fwrite(buf + i * wrap, 1, xsize, f);
  fclose(f);
}
