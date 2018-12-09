#ifndef EJ_HTTPUPLOAD_H_
#define EJ_HTTPUPLOAD_H_

extern "C"{

	/** 文件上传接口
	 * param: sHostName 服务器IP地址
	 * param: nPort 服务端口号
	 * param：sUrl 服务名
	 * param: sLocalFile 本地文件全路径名
	 * param: sFileName 本地文件名
	 * param: sServerPath 文件保存在服务器的路径
	 * return: 0-success; other-error
     */
	int httpUpload(char* sHostName, int nPort, char* sUrl, char* sLocalFile, char* sFileName, char* sServerPath);

}

#endif
