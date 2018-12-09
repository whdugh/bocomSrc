/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <pthread.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#include "capture.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))


void Setting_Parameter(int flag, int Param, void *handle)
{
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;
	struct v4l2_queryctrl queryctrl;  
	struct v4l2_control control;      

	memset (&queryctrl, 0, sizeof (queryctrl));
	queryctrl.id = Param;

	if (-1 == ioctl (h->fd, VIDIOC_QUERYCTRL, &queryctrl)) {
	        if (errno != EINVAL) {
	                perror ("VIDIOC_QUERYCTRL");
	                exit (EXIT_FAILURE);
	        } else {
	                fprintf (stderr, "This parameter is not supported\n");
	        }
	} else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
	        fprintf (stderr, "This parameter is not supported\n");
	} else {
	        memset (&control, 0, sizeof (control));
	        control.id = Param;
		        control.value = queryctrl.default_value;
			control.value = flag;

        	if (-1 == ioctl (h->fd, VIDIOC_S_CTRL, &control)) {    
        	        perror ("VIDIOC_S_CTRL");
        	        exit (EXIT_FAILURE);
        	}
	}
}

int xioctl(int  fd, int  request, void * arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

struct v4l2_video_buffer*  GetNextFrame(void* handle)
{
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;
	fd_set fds;
	struct timeval tv;
	int r;

	struct v4l2_video_buffer *buffer = NULL;
	struct v4l2_buffer buf;

again:
	FD_ZERO (&fds);
	FD_SET (h->fd, &fds);

	/* Timeout. */
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	r = select (h->fd + 1, &fds, NULL, NULL, &tv);

	if (-1 == r)
	{
		if (EINTR == errno)
			goto again;

		return NULL;
	}

	if (0 == r)
	{
		return NULL;
	}

	CLEAR (buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (h->fd, VIDIOC_DQBUF, &buf))
	{
       	return NULL;
	}

    assert (buf.index < h->n_buffers);

	////////////////
	buffer = &h->buffers[buf.index];

	buffer->ts = get_timestamp(&buf.timestamp);
	//printf( "buf.index=%d,buffer->size=%d,sec=%ld,usec = %ld\r\n", buf.index,buffer->size ,buf.timestamp.tv_sec,buf.timestamp.tv_usec);
	//fflush(stdout);
	////////////////

	if (-1 == xioctl (h->fd, VIDIOC_QBUF, &buf))
		perror ("VIDIOC_QBUF");

	return buffer;
}

void stop_capturing (void* handle)
{
	int i = 0;
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;

    enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (h->fd, VIDIOC_STREAMOFF, &type))
		perror ("ioctl VIDIOC_STREAMOFF");

	for (i = 0; i < h->n_buffers; ++i)
	{
		if (-1 == munmap (h->buffers[i].data, h->buffers[i].size))
			perror ("munmap");
	}

	free (h->buffers);
}

int init_mmap(void* handle)
{
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;

	struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count               = 4;
    req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (h->fd, VIDIOC_REQBUFS, &req))
	{
		return -1;
    }

    if (req.count < 2)
	{
        return -1;
    }

    h->buffers = (struct v4l2_video_buffer*)calloc (req.count, sizeof (*h->buffers));

    if (!h->buffers)
	{
       return -1;
    }

    for (h->n_buffers = 0; h->n_buffers < req.count; ++h->n_buffers)
	{
       struct v4l2_buffer buf;

       CLEAR (buf);

       buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
       buf.memory      = V4L2_MEMORY_MMAP;
       buf.index       = h->n_buffers;

       if (-1 == xioctl (h->fd, VIDIOC_QUERYBUF, &buf))
               return -1;

       h->buffers[h->n_buffers].size = buf.length;
       h->buffers[h->n_buffers].data = mmap (NULL /* start anywhere */,buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              h->fd, buf.m.offset);

       if (MAP_FAILED == h->buffers[h->n_buffers].data)
               return -1;
    }

	return 0;
}

int  start_capturing (void *handle)
{
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;
    unsigned int i;
    enum v4l2_buf_type type;

	if(-1 == init_mmap (handle))
		return -1;

	for (i = 0; i < h->n_buffers; ++i)
	{
        struct v4l2_buffer buf;

		CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = i;

        if (-1 == xioctl (h->fd, VIDIOC_QBUF, &buf))
            return -1;
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (h->fd, VIDIOC_STREAMON, &type))
		return -1;

	return 0;
}




int init_device (void* handle)
{
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;

    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;

	if (-1 == xioctl (h->fd, VIDIOC_QUERYCAP, &cap))
	{
        return -1;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
        return -1;
    }

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		return -1;
	}

    /* Select video input, video standard and tune here. */
    if (-1 == xioctl (h->fd, VIDIOC_S_STD, &h->std))
	{
        return -1;
    }

    // Change to the default channel
    int channel = 0;
    if (-1 == xioctl (h->fd, VIDIOC_S_INPUT, &channel))
	{
        return -1;
    }
	CLEAR (cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl (h->fd, VIDIOC_CROPCAP, &cropcap))
	{
         crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
         crop.c = cropcap.defrect; /* reset to default */

         if (-1 == xioctl (h->fd, VIDIOC_S_CROP, &crop))
		 {
             return -1;
         }
     }

     CLEAR (h->fmt);

     h->fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     h->fmt.fmt.pix.width       = h->width;
     h->fmt.fmt.pix.height      = h->height;
     h->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
     h->fmt.fmt.pix.field       = V4L2_FIELD_ANY;
	 //h->fmt.fmt.pix.field       = V4L2_FIELD_ALTERNATE;

    if (-1 == xioctl (h->fd, VIDIOC_S_FMT, &h->fmt))
           return -1;
    /* Note VIDIOC_S_FMT may change width and height. */

	return 0;
}

int close_device   (void *handle)
{
	struct v4l2_video_handle *h = (struct v4l2_video_handle *)handle;

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

void*	open_device (char *  dev_name, int width, int height)
{
	//设备结构
	struct v4l2_video_handle* h =(struct v4l2_video_handle*)malloc(sizeof (struct v4l2_video_handle));
	if (NULL == h)
		return NULL;
	//初始化
	memset (h, 0, sizeof (struct v4l2_video_handle));

    h->fd = open (dev_name, O_RDWR /* required  | O_NONBLOCK, 0*/);

    if (-1 == h->fd)
	{
		//打开设备失败
		fprintf (stderr, "v4l2: open %s: %s\n", dev_name,strerror(errno));
        goto err;
    }

	h->width = width;
	h->height = height;
	h->std = V4L2_STD_PAL_B;

	if(-1 == init_device(h))
	{
		goto err;
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

int64_t get_timestamp(struct timeval *tv)
{
    int64_t ts;

	// 格式  timeval.tv_sec:timeval.tv_usec
	//      timeval.tv_sec:秒
	//      timeval.tv_usec:毫秒
    ts  = tv->tv_sec;
    ts *= 1000000;
    ts += tv->tv_usec;
   // ts *= 1000;
    return ts;
}
