// 明迈交通视频智能识别检测软件 V1.0
// Mimax Intelligent Transport Video Recognition & Detection Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include "v4l2_driver.h"


const __u32 xawtv_pixelformat[VIDEO_FMT_COUNT] = {
	[VIDEO_RGB08] = V4L2_PIX_FMT_HI240,
	[VIDEO_GRAY] = V4L2_PIX_FMT_GREY,
	[VIDEO_RGB15_LE] = V4L2_PIX_FMT_RGB555,
	[VIDEO_RGB16_LE] = V4L2_PIX_FMT_RGB565,
	[VIDEO_RGB15_BE] = V4L2_PIX_FMT_RGB555X,
	[VIDEO_RGB16_BE] = V4L2_PIX_FMT_RGB565X,
	[VIDEO_BGR24] = V4L2_PIX_FMT_BGR24,
	[VIDEO_BGR32] = V4L2_PIX_FMT_BGR32,
	[VIDEO_RGB24] = V4L2_PIX_FMT_RGB24,
	[VIDEO_YUYV] = V4L2_PIX_FMT_YUYV,
	[VIDEO_UYVY] = V4L2_PIX_FMT_UYVY,
	[VIDEO_YUV422P] = V4L2_PIX_FMT_YUV422P,
	[VIDEO_YUV420P] = V4L2_PIX_FMT_YUV420,
};


const unsigned int ng_vfmt_to_depth[] = {
    0,               /* unused   */
    8,               /* RGB8     */
    8,               /* GRAY8    */
    16,              /* RGB15 LE */
    16,              /* RGB16 LE */
    16,              /* RGB15 BE */
    16,              /* RGB16 BE */
    24,              /* BGR24    */
    32,              /* BGR32    */
    24,              /* RGB24    */
    32,              /* RGB32    */
    16,              /* LUT2     */
    32,              /* LUT4     */
    16,		     /* YUYV     */
    16,		     /* YUV422P  */
    12,		     /* YUV420P  */
    0,		     /* MJPEG    */
    0,		     /* JPEG     */
    16,		     /* UYVY     */
};


void print_bufinfo (struct v4l2_buffer *buf)
{
	static char *type[] = {
		[V4L2_BUF_TYPE_VIDEO_CAPTURE] = "video-cap",
		[V4L2_BUF_TYPE_VIDEO_OVERLAY] = "video-over",
		[V4L2_BUF_TYPE_VIDEO_OUTPUT] = "video-out",
		[V4L2_BUF_TYPE_VBI_CAPTURE] = "vbi-cap",
		[V4L2_BUF_TYPE_VBI_OUTPUT] = "vbi-out",
	};

	fprintf (stderr, "v4l2: buf %d: %s 0x%x+%d, used %d\n",
		 buf->index,
		 buf->type <
		 sizeof (type) / sizeof (char *)? type[buf->type] : "unknown",
		 buf->m.offset, buf->length, buf->bytesused);
}


//初始化捕捉数据
void v4l2_init_video_buf(struct v4l2_video_buf *buf)
{
    memset(buf,0,sizeof(*buf));
    pthread_mutex_init(&buf->lock,NULL);    
    pthread_cond_init(&buf->cond,NULL);
}

//释放使用的数据
int v4l2_release_video_buf(struct v4l2_video_buf *buf)
{
    int release;

    pthread_mutex_lock(&buf->lock);
    buf->refcount--;
    release = (buf->refcount == 0);
    pthread_mutex_unlock(&buf->lock);
    if (release && NULL != buf->release)
    {
		buf->release(buf);
		return 0;
    }
	else
		return -1;
}


void v4l2_wakeup_video_buf(struct v4l2_video_buf *buf)
{
    pthread_cond_signal(&buf->cond);
}

void v4l2_waiton_video_buf(struct v4l2_video_buf *buf)
{
    pthread_mutex_lock(&buf->lock);
    while (buf->refcount)
	pthread_cond_wait(&buf->cond, &buf->lock);
    pthread_mutex_unlock(&buf->lock);
}




int v4l2_queue_buffer (struct v4l2_handle *h)
{
	int frame = h->queue % h->reqbufs.count;
	int rc;
	if (0 != h->buf_me[frame].refcount)
	{
		if (0 != h->queue - h->waiton)
			return -1;
		fprintf (stderr, "v4l2: waiting for a free buffer\n");
		v4l2_waiton_video_buf (h->buf_me + frame);
	}

	rc = ioctl (h->fd, VIDIOC_QBUF, &h->buf_v4l2[frame]);
	if (0 == rc)
		h->queue++;
	return rc;
}

void v4l2_queue_all (struct v4l2_handle *h)
{
	for (;;)
	{
		if (h->queue - h->waiton >= h->reqbufs.count)
			return;
		if (0 != v4l2_queue_buffer (h))
			return;
	}
}




//取设备性能参数
void get_device_capabilities (struct v4l2_handle *h)
{
	int i = 0;

	//设备INPUTS
	for (h->ninputs = 0; h->ninputs < LINUX_V4L2_MAX_INPUT; h->ninputs++)
	{
		h->inp[h->ninputs].index = h->ninputs;
		if (-1 == ioctl(h->fd, VIDIOC_ENUMINPUT, &h->inp[h->ninputs]))
			break;
	}
	//设备格式描述
	for (h->nfmts = 0; h->nfmts < LINUX_V4L2_MAX_FORMAT; h->nfmts++)
	{
		h->fmt[h->nfmts].index = h->nfmts;
		h->fmt[h->nfmts].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == ioctl (h->fd, VIDIOC_ENUM_FMT, &h->fmt[h->nfmts]))
			break;
	}
	//stream 依靠参数
	h->streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl (h->fd, VIDIOC_G_PARM, &h->streamparm);

	//控制代码
	for (i = 0; i < LINUX_V4L2_MAX_CTRL; i++)
	{
		h->ctl[i].id = V4L2_CID_PRIVATE_BASE + i;
		//取对应控制属性值,判断是否有效
		if (-1 == ioctl (h->fd, VIDIOC_QUERYCTRL, &h->ctl[i])||(h->ctl[i].flags & V4L2_CTRL_FLAG_DISABLED))
			h->ctl[i].id = LINUX_V4L2_UNSET;
#ifdef _DEBUG
	printf("query ctrl id = %d max = %d min = %d \r\n",h->ctl[i].id,h->ctl[i].maximum,h->ctl[i].minimum);
#endif

	}

	return;
}

//打开设备
void* v4l2_open (char *device)
{
	int i;
	//设备结构
	struct v4l2_handle* h =(struct v4l2_handle*)malloc(sizeof (struct v4l2_handle));
	if (NULL == h)
		return NULL;
	//初始化
	memset (h, 0, sizeof (struct v4l2_handle));
	//打开设备
	if (-1 == (h->fd = open (device, O_RDWR)))
	{
		//打开设备失败
		fprintf (stderr, "v4l2: open %s: %s\n", device,strerror(errno));
		goto err;
	}
	//取驱动信息
	if (-1 == ioctl (h->fd, VIDIOC_QUERYCAP, &h->cap))
		goto err;

#ifdef _DEBUG
		fprintf (stderr, "v4l2: open\n");
#endif
	//设置 close on exec
	fcntl (h->fd, F_SETFD, FD_CLOEXEC);

#ifdef _DEBUG
		fprintf (stderr, "v4l2: device info:\n"
			 "  %s %d.%d.%d / %s @ %s\n",
			 h->cap.driver,
			 (h->cap.version >> 16) & 0xff,
			 (h->cap.version >> 8) & 0xff,
			 h->cap.version & 0xff, h->cap.card, h->cap.bus_info);
#endif
	//取设备相关性能参数
	get_device_capabilities (h);


	//初始化捕捉数据
	for (i = 0; i < LINUX_V4L2_BUFFERS; i++)
	{
		v4l2_init_video_buf (h->buf_me + i);
		h->buf_me[i].release = v4l2_wakeup_video_buf;
	}

	//返回结构
	return h;

err:
	//关闭设备描述符
	if (h->fd != -1)
		close (h->fd);
	//释放设备结构
	if (h)
		free (h);
	return NULL;
}
//关闭设备
int v4l2_close (void *handle)
{
	struct v4l2_handle *h = handle;

#ifdef _DEBUG
		fprintf (stderr, "v4l2: close\n");
#endif
	//关闭设备
	if(h)
	{
		if(h->fd != -1)
			close (h->fd);
		free (h);
	}
	h = NULL;

	return 0;
}
//设备视频参数
int v4l2_setformat (void *handle, struct v4l2_video_fmt *fmt)
{
	struct v4l2_handle *h = handle;
	h->fmt_v4l2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//视频格式
	h->fmt_v4l2.fmt.pix.pixelformat = xawtv_pixelformat[fmt->fmtid];
	//宽
	h->fmt_v4l2.fmt.pix.width = fmt->width;
	//高
	h->fmt_v4l2.fmt.pix.height = fmt->height;
	h->fmt_v4l2.fmt.pix.field = V4L2_FIELD_ANY;
	
	if (fmt->bytesperline !=
	    fmt->width * ng_vfmt_to_depth[fmt->fmtid] / 8)
		h->fmt_v4l2.fmt.pix.bytesperline = fmt->bytesperline;
	else
		h->fmt_v4l2.fmt.pix.bytesperline = 0;

	if (-1 == ioctl (h->fd, VIDIOC_S_FMT, &h->fmt_v4l2))
		return -1;
	if (h->fmt_v4l2.fmt.pix.pixelformat != xawtv_pixelformat[fmt->fmtid])
		return -1;

	fmt->width = h->fmt_v4l2.fmt.pix.width;
	fmt->height = h->fmt_v4l2.fmt.pix.height;
	fmt->bytesperline = h->fmt_v4l2.fmt.pix.bytesperline;
	if (0 == fmt->bytesperline)
		fmt->bytesperline =
			fmt->width * ng_vfmt_to_depth[fmt->fmtid] / 8;
	//保存数据
	h->fmt_me = *fmt;


#ifdef _DEBUG
		fprintf (stderr,
			 "v4l2: new capture params (%dx%d, %c%c%c%c, %d byte)\n",
			 h->fmt_v4l2.fmt.pix.width,
			 h->fmt_v4l2.fmt.pix.height,
			 h->fmt_v4l2.fmt.pix.pixelformat & 0xff,
			 (h->fmt_v4l2.fmt.pix.pixelformat >> 8) & 0xff,
			 (h->fmt_v4l2.fmt.pix.pixelformat >> 16) & 0xff,
			 (h->fmt_v4l2.fmt.pix.pixelformat >> 24) & 0xff,
			 h->fmt_v4l2.fmt.pix.sizeimage);
#endif
	return 0;
}

//开始捕捉流
int v4l2_start_streaming (struct v4l2_handle *h, int buffers)
{
	int disable_overlay = 0;
	unsigned int i;
	//缓冲空间
	h->reqbufs.count = buffers;
	h->reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	h->reqbufs.memory = V4L2_MEMORY_MMAP;
	if (-1 == ioctl (h->fd, VIDIOC_REQBUFS, &h->reqbufs))
		return -1;

	for (i = 0; i < h->reqbufs.count; i++)
	{
		h->buf_v4l2[i].index = i;
		h->buf_v4l2[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		h->buf_v4l2[i].memory = V4L2_MEMORY_MMAP;	
		if (-1 == ioctl (h->fd, VIDIOC_QUERYBUF, &h->buf_v4l2[i]))
			return -1;

		h->buf_me[i].fmt = h->fmt_me;
		h->buf_me[i].size = h->buf_me[i].fmt.bytesperline *
			h->buf_me[i].fmt.height;

		h->buf_me[i].data = mmap (NULL, h->buf_v4l2[i].length,
					  PROT_READ | PROT_WRITE, MAP_SHARED,
					  h->fd, h->buf_v4l2[i].m.offset);

		if (MAP_FAILED == h->buf_me[i].data)
		{
			perror ("mmap");
			return -1;
		}
#ifdef _DEBUG
			print_bufinfo (&h->buf_v4l2[i]);
#endif
	}

	//
	v4l2_queue_all (h);

try_again:
	//开始捕捉
	if (-1 == ioctl (h->fd, VIDIOC_STREAMON, &h->fmt_v4l2.type))
	{
		//设备忙,不重试
		if (errno == EBUSY)
		{
#ifdef _DEBUG
		fprintf (stderr, "v4l2: busy now\n");
#endif
			return -1;
		}
		return -1;
	}
	return 0;
}

//停止捕捉流
void v4l2_stop_streaming (struct v4l2_handle *h)
{
	unsigned int i;

	//停止捕捉数据
	if (-1 == ioctl (h->fd, VIDIOC_STREAMOFF, &h->fmt_v4l2.type))
		perror ("ioctl VIDIOC_STREAMOFF");

	//释放空间
	for (i = 0; i < h->reqbufs.count; i++)
	{
		if (0 != h->buf_me[i].refcount)
			v4l2_waiton_video_buf (&h->buf_me[i]);
#ifdef _DEBUG
			print_bufinfo (&h->buf_v4l2[i]);
#endif
		if (-1 == munmap (h->buf_me[i].data, h->buf_me[i].size))
			perror ("munmap");
	}
	h->queue = 0;
	h->waiton = 0;
}

//开始捕捉
int v4l2_startvideo (void *handle, int fps, unsigned int buffers)
{
	struct v4l2_handle *h = handle;

	if (0 != h->fps)
		fprintf (stderr, "v4l2_startvideo: oops: fps!=0\n");
	//帧率
	h->fps = fps;
	//
	//h->first = 1;
	//h->start = 0;

	if (h->cap.capabilities & V4L2_CAP_STREAMING)
		return v4l2_start_streaming (h, buffers);
	return 0;
}

//停止捕捉
void v4l2_stopvideo (void *handle)
{
	struct v4l2_handle *h = handle;

	if (0 == h->fps)
		fprintf (stderr, "v4l2_stopvideo: oops: fps==0\n");
	h->fps = 0;

	if (h->cap.capabilities & V4L2_CAP_STREAMING)
		v4l2_stop_streaming (h);
}

int v4l2_waiton (struct v4l2_handle *h)
{
	struct v4l2_buffer buf;
	struct timeval tv;
	fd_set rdset;

	//等待下一帧数据
again:
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	FD_ZERO (&rdset);
	FD_SET (h->fd, &rdset);
	switch (select (h->fd + 1, &rdset, NULL, NULL, &tv))
	{
	case -1:
		if (EINTR == errno)
			goto again;
		perror ("v4l2: select");
		return -1;
	case 0:
		fprintf (stderr, "v4l2: oops: select timeout\n");
		return -1;
	}
	//取数据
	memset (&buf, 0, sizeof (buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (h->fd, VIDIOC_DQBUF, &buf))
		return -1;
	h->waiton++;
	h->buf_v4l2[buf.index] = buf;
	return buf.index;
}

int64_t v4l2_tofday_to_timestamp(struct timeval *tv)
{
    long long ts;

	// 格式  timeval.tv_sec:timeval.tv_usec
	//      timeval.tv_sec:秒
	//      timeval.tv_usec:毫秒
    ts  = tv->tv_sec;
    ts *= 1000000;
    ts += tv->tv_usec;
   // ts *= 1000;
    return ts;
}

//捕捉下一帧
struct v4l2_video_buf* v4l2_nextframe (void *handle)
{
	struct v4l2_handle *h = handle;
	struct v4l2_video_buf *buf = NULL;
	int rc, size, frame = 0;

	if (h->cap.capabilities & V4L2_CAP_STREAMING)
	{
	//	printf("v4l2_nextframe\r\n");
		v4l2_queue_all (h);
		frame = v4l2_waiton (h);
		if (-1 == frame)
		{
		//	printf("v4l2_waiton\r\n");

			return NULL;
		}

		h->buf_me[frame].refcount++;
		buf = &h->buf_me[frame];
		memset (&buf->info, 0, sizeof (buf->info));
		buf->info.ts =
			v4l2_tofday_to_timestamp (&h->buf_v4l2[frame].
						timestamp);
	}
//	if (h->first)
//	{
//		h->first = 0;
//		h->start = buf->info.ts;
//#ifdef _DEBUG
//			fprintf (stderr, "v4l2: start ts=%lld\n", h->start);
//#endif
//	}
//	buf->info.ts -= h->start;
	return buf;
}

//设置视频参数
//设置亮度
int v4l2_setbrightness(void *handle,unsigned int uBrightness)
{
	struct v4l2_handle *h = handle;
	struct v4l2_control c;
	//亮度
	c.id = V4L2_CID_BRIGHTNESS;
	c.value = uBrightness;
	return ioctl (h->fd, VIDIOC_S_CTRL, &c);
}

//设置对比度
int v4l2_setcontrast(void *handle,unsigned int uContrast)
{
	struct v4l2_handle *h = handle;
	struct v4l2_control c;
	//对比度
	c.id = V4L2_CID_CONTRAST;
	c.value = uContrast;
	return ioctl (h->fd, VIDIOC_S_CTRL, &c);
}

//设置饱和度
int v4l2_setsaturation(void *handle,unsigned int uSaturation)
{
	struct v4l2_handle *h = handle;
	struct v4l2_control c;
	//饱和度
	c.id = V4L2_CID_SATURATION;
	c.value = uSaturation;
	return ioctl (h->fd, VIDIOC_S_CTRL, &c);
}

//设置色调
int v4l2_sethue(void *handle,unsigned int uHue)
{
	struct v4l2_handle *h = handle;
	struct v4l2_control c;
	//色调
	c.id = V4L2_CID_HUE;
	c.value = uHue;
	return ioctl (h->fd, VIDIOC_S_CTRL, &c);
}

//设置视频源
int v4l2_setsource(void *handle,unsigned int uSource)
{
	struct v4l2_handle *h = handle;
	return ioctl (h->fd, VIDIOC_S_INPUT, &uSource);
}