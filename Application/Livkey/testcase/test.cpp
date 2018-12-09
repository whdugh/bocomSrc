#include "CUsbKeyLockRead.h"
#include <iostream>

int main(void)
{
	CUsbKeyLockRead *cuklw = new CUsbKeyLockRead;
	bool ret = false;

	ret = cuklw->OpenUsb();
	if (!ret)
	{
		//std::cout << "Open USBKey Error ! " << std::endl;
		if (cuklw)
		{
			delete cuklw;
		}
		return 0;
	}
	//else
	{
		std::cout << "begin test" << std::endl;
	}

	ret = cuklw->CompareLocalInfo();
	if (!ret)
	{
		std::cout << "test error" << std::endl;
	}
	else
	{
		std::cout << "test ok" << std::endl;
	}
	
	ret = cuklw->CloseUsb();
	/*if (!ret)
	{
		std::cout << "Close USBKey Error ! " << std::endl;
	}
	else*/
	{
		std::cout << "end test" << std::endl;
	}

	if (cuklw)
	{
		delete cuklw;
	}
	
	return 1;
}