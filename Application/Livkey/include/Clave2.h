/**************************************************************************
* Copyright (c) 1995-2012, Beijing Senselock Software Technology Co.,Ltd.
* All rights reserved.
*
* filename: Clave2.h
*
* briefs:  API library interface declaration and the error code
*
* history:
* 07/31/2008   yingy   R&D   create
* 08/05/2008   zhaock  R&D   modify
**************************************************************************/

#ifndef _CLAVE2_H_
#define _CLAVE2_H_

#ifdef  _MSC_VER
#pragma comment(linker, "/defaultlib:setupapi.lib")
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIVAPI
#if defined WIN32 || defined _WIN32
#define LIVAPI __stdcall
#else
#define LIVAPI
#endif
#endif

// 错误码
#define LIV_SUCCESS                            0  // 成功
#define LIV_OPEN_DEVICE_FAILED                 1  // 打开设备失败
#define LIV_FIND_DEVICE_FAILED                 2  // 未找到符合条件的设备
#define LIV_INVALID_PARAMETER                  3  // 参数错误
#define LIV_INVALID_BLOCK_NUMBER               4  // 块号错误
#define LIV_HARDWARE_COMMUNICATE_ERROR         5  // 与硬件通信错误
#define LIV_INVALID_PASSWORD                   6  // 密码错误
#define LIV_ACCESS_DENIED                      7  // 没有权限
#define LIV_ALREADY_OPENED                     8  // 设备已经打开
#define LIV_ALLOCATE_MEMORY_FAILED             9  // 内存分配失败
#define LIV_INVALID_UPDATE_PACKAGE             10 // 不合法的升级包
#define LIV_SYN_ERROR                          11 // 线程同步错误
#define LIV_OTHER_ERROR                        12 // 其它未知异常、错误
#define LIV_OPERATION_NOT_SUPPORTED            13 // 不支持的操作
#define LIV_DEVICE_BLOCKED                     14 // 设备被锁
#define LIV_WRITE_FAILED                       15 // FLASH写失败

// 硬件信息结构
typedef struct {
    int    developerNumber; // 开发商编号
    char   serialNumber[8]; // 设备唯一序列号
    int    manufactureDate; // 生产日期
    int    setDate;         // 设置日期
	int    reservation;     // 保留
}LIV_hardware_info;

// 软件信息结构
typedef struct {
    int    version;        // 软件版本
	int    reservation;    // 保留
} LIV_software_info;

// 硬件版本信息
typedef struct {
	char   capacity[4];    // 第四字节为容量信息
	char   version[3];     // 三位固件版本信息
}LIV_version_info;

// @{
/**
    @LIV API function interface
*/

/**
    根据开发商编号和索引，打开符合条件的设备

    @parameter vendor           [in]  开发商编号(0= 所有)
    @parameter index            [in]  设备索引(0= 第一个，依次类推)
    @parameter handle           [out] 返回的设备句柄

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_open(int vendor, int index, int *handle);

/**
    关闭已经打开的设备

    @parameter handle           [in]  打开的设备句柄

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_close(int handle);

/**
    校验设备口令

    @parameter handle           [in]  打开的设备句柄
    @parameter type             [in]  口令类型（管理员0，普通1，认证2）
    @parameter passwd           [in]  口令(8字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_passwd(int handle, int type, unsigned char *passwd);

/**
    读指定区块的数据

    @parameter handle           [in]  打开的设备句柄
    @parameter block            [in]  要读取的区块号
    @parameter buffer           [out] 读取数据缓冲区（必须大于等于512字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_read(int handle, int block, unsigned char *buffer);

/**
    写指定区块的数据

    @parameter handle           [in]  打开的设备句柄
    @parameter block            [in]  要写入的区块号
    @parameter buffer           [in]  写入数据缓冲区（必须大于等于512字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_write(int handle, int block, unsigned char *buffer);

/**
    使用AES算法加密数据

    @parameter handle           [in]  打开的设备句柄
    @parameter plaintext        [in]  要加密的明文数据(16字节)
    @parameter ciphertext       [out] 加密后的密文数据(16字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_encrypt(int handle, unsigned char *plaintext,  unsigned char *ciphertext);

/**
    使用AES算法解密数据

    @parameter handle           [in]  打开的设备句柄
    @parameter ciphertext       [in]  要解密的密文数据(16字节)
    @parameter plaintext        [out] 解密后的明文数据(16字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_decrypt(int handle, unsigned char *ciphertext, unsigned char *plaintext);

/**
    设置新的口令，需要开发商权限

    @parameter handle           [in]  打开的设备句柄
    @parameter type             [in]  口令类型（管理员0，普通1，认证2）
    @parameter newpasswd        [in]  口令(8字节)
    @parameter retries          [in]  错误计数(1-15) -1表示不使用错误计数

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_set_passwd(int handle, int type, unsigned char *newpasswd, int retries);

/**
    修改口令

    @parameter handle           [in]  打开的设备句柄
    @parameter type             [in]  口令类型（管理员0，普通1，认证2）
    @parameter oldpasswd        [in]  旧口令(8字节)
    @parameter newpasswd        [in]  新口令(8字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_change_passwd(int handle, int type, unsigned char *oldpasswd, unsigned char *newpasswd);

/**
    生成升级包
	@parameter serial           [in]  升级锁的序列号
	@parameter block            [in]  要升级的区块号
	@parameter buffer           [in]  升级内容（区块3为384字节，其它512字节）
	@parameter key              [in]  远程升级密钥（20字节）
	@parameter uptPkg           [out] 升级包（549字节）

	@return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_gen_update_pkg(unsigned char * serial, int block, unsigned char *buffer, unsigned char *key, unsigned char *uptPkg);


/**
    远程更新指定的区块

    @parameter handle           [in]  打开的设备句柄
    @parameter buffer           [in]  更新数据包缓冲区


    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_update(int handle, unsigned char *buffer);

/**
    取硬件信息

    @parameter handle           [in]  打开的设备句柄
    @parameter info             [out] 硬件信息

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_get_hardware_info(int handle, LIV_hardware_info *info);

/**
取硬件相关版本信息

    @parameter handle           [in]  打开的设备句柄
    @parameter buffer           [out] 硬件版本相关信息

    @return value
成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_get_version_info(int handle,LIV_version_info *versioninfo);

/**
    取软件信息

    @parameter info             [out] 软件信息

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_get_software_info(LIV_software_info *info);

/**
    硬件计算hmac

    @parameter handle           [in]  打开的设备句柄
    @parameter text             [in]  要计算hmac值的数据
    @parameter textlen          [in]  数据长度( >= 0)
    @parameter digest           [out] hmac值(20字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_hmac(int handle, unsigned char *text, int textlen, unsigned char *digest);

/**
    软件计算hmac值

    @parameter text            [in]  要计算hmac值的数据
    @parameter textlen         [in]  数据长度(0-(未确定) 字节)
    @parameter key             [in]  hmac算法的key(20字节)
    @parameter digest          [out] hmac值(20字节)

    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_hmac_software(unsigned char *text, int textlen, unsigned char * key, unsigned char *digest);

/**
    设置密钥

    @parameter handle           [in]  打开的设备句柄
    @parameter type             [in]  密钥类型（0--远程升级密钥，1--认证密钥 ）
	@parameter key              [in]  密钥(20字节)


    @return value
    成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_set_key(int handle, int type, unsigned char *key);

/**
LED控制

    @parameter handle        [in]  打开设备句柄
    @parameter types         [in]  闪烁类型 （0亮 0xff灭 1-3不同频率）

    @return value
成功，返回0，失败，返回预定义的错误码
*/
int LIVAPI LIV_flash_led(int handle, unsigned char types);

#ifdef __cplusplus
}
#endif

#endif /* end of "#ifndef _LIVING1_H_" */
