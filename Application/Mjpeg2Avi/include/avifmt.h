#ifndef _AVIFMT_H_
#define _AVIFMT_H_

/* for use in AVI_avih.flags */
const unsigned int AVIF_HASINDEX = 0x00000010;	/* index at end of file */
const unsigned int AVIF_MUSTUSEINDEX = 0x00000020;
const unsigned int AVIF_ISINTERLEAVED = 0x00000100;
const unsigned int AVIF_TRUSTCKTYPE = 0x00000800;
const unsigned int AVIF_WASCAPTUREFILE = 0x00010000;
const unsigned int AVIF_COPYRIGHTED = 0x00020000;


struct AVI_avih 
{
  unsigned int	us_per_frame;	/* frame display rate (or 0L) */
  unsigned int max_bytes_per_sec;	/* max. transfer rate */
  unsigned int padding;	/* pad to multiples of this size; */
  /* normally 2K */
  unsigned int flags;
  unsigned int tot_frames;	/* # frames in file */
  unsigned int init_frames;
  unsigned int streams;
  unsigned int buff_sz;  
  unsigned int width;
  unsigned int height;
  unsigned int reserved[4];
};


struct AVI_strh
{
  unsigned char type[4];      /* stream type */
  unsigned char handler[4];
  unsigned int flags;
  unsigned int priority;
  unsigned int init_frames;       /* initial frames (???) */
  unsigned int scale;
  unsigned int rate;
  unsigned int start;
  unsigned int length;
  unsigned int buff_sz;           /* suggested buffer size */
  unsigned int quality;
  unsigned int sample_sz;
};


struct AVI_strf
{       
  unsigned int sz;
  unsigned int width;
  unsigned int height;
  unsigned int planes_bit_cnt;
  unsigned char compression[4];
  unsigned int image_sz;
  unsigned int xpels_meter;
  unsigned int ypels_meter;
  unsigned int num_colors;        /* used colors */
  unsigned int imp_colors;        /* important colors */
  /* may be more for some codecs */
};


/*
  AVI_list_hdr

  spc: a very ubiquitous AVI struct, used in list structures
       to specify type and length
*/

struct AVI_list_hdr 
{
  unsigned char id[4];   /* "LIST" */
  unsigned int sz;              /* size of owning struct minus 8 */
  unsigned char type[4]; /* type of list */
};


struct AVI_list_odml 
{
  struct AVI_list_hdr list_hdr;

  unsigned char id[4];
  unsigned int sz;
  unsigned int frames;
};


struct AVI_list_strl 
{
  struct AVI_list_hdr list_hdr;
  
  /* chunk strh */
  unsigned char strh_id[4];
  unsigned int strh_sz;
  struct AVI_strh strh;

  /* chunk strf */
  unsigned char strf_id[4];
  unsigned int strf_sz;
  struct AVI_strf strf;

  /* list odml */
  struct AVI_list_odml list_odml;
};


struct AVI_list_hdrl 
{
  struct AVI_list_hdr list_hdr;

  /* chunk avih */
  unsigned char avih_id[4];
  unsigned int avih_sz;
  struct AVI_avih avih;
  
  /* list strl */
  struct AVI_list_strl strl;
};


#endif
