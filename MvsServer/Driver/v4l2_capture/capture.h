#ifndef _V4L2_CAPTURE_H
#define _V4L2_CAPTURE_H

struct v4l2_video_buffer 
{
    unsigned char *         data;
    size_t                  size;
	int64_t          ts; 
};

struct v4l2_video_handle
{
	int    fd;
	struct v4l2_video_buffer *    buffers;
	unsigned int   n_buffers;

	int   width;
	int   height;

	v4l2_std_id    std;
	struct v4l2_format     fmt;
};
//void Setting_Parameter(int flag, int Param, void *handle);

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"{
#endif

struct v4l2_video_buffer*  GetNextFrame(void* handle);
int  start_capturing (void *handle);
void stop_capturing (void* handle);
int init_mmap(void* handle);
int init_device (void* handle);
int xioctl(int  fd, int  request, void * arg);
int close_device   (void *handle);
void*	open_device (char *  dev_name, int width, int height);
int64_t get_timestamp(struct timeval *tv);
void Setting_Parameter(int flag, int Param, void *handle);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif	//_V4L2_CAPTURE_H
