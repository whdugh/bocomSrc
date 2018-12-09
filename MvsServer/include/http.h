#ifndef HTTP_H
#define HTTP_H
extern "C"
{


int Save_File(char *filebuf,int filelength,char *filename);
int HTTP_GetResponseCode(void);
int HTTP_GetRecvLength(char *revbuf);
int HTTP_GetContentLength(char *revbuf);
int HTTP_Recv(int sockfd,char *buf_recv);
int HTTP_GetFileName(char *url,char *filename);
int HTTP_GetPath(char *url,char *path);
int HTTP_Get_IP_PORT(char *url,char *ip,char *port);
void Package_Url_Get_File(char *path, char *range);
int Package_Url_Get_FileSize(char *url);
int HTTP_GetFileSize(int sockfd,char *path);
int HTTP_GetFile(int sockfd,char *path,int filelength,int download_size,char *filebuf);
int HTTP_DownloadFile(char *url,bool bFisrtDown = true);
};

#endif
