#ifndef _BAOXIN_NETPORTOL_H_
#define _BAOXIN_NETPORTOL_H_

int BX_CreateServer();

int BX_CloseServer();

int BX_45000Send( const unsigned char *sendData,int len,int state );


#endif //_BAOXIN_NETPORTOL_H_
