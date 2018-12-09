//This head file defines error value used in one wire system
#define		ERROR		    0x00
#define		OK				0x01
#define		ENABLE			0x02
#define		DISABLE			0x03
#define		NODEVICE		0x04			//No device on 1 wire bus.
#define		IDCRC8ER		0x05			//CRC8 error on ROMID
#define		CRC16ER			0x06			//CRC16 error
#define		OUTRAN			0x07			//Range error in DS2411.
#define		FCER			0x08			//Family code error.
#define		LESS			0x09			//A < B.
#define		BIG				0x0A			//A > B.
#define		EQ				0x0B			//A = B.
#define		WRSCC16ER		0x0C			//Write scratch pad CRC16 error.
#define		RDSCC16ER		0x0D			//Read scratch pad CRC16 error.
#define		PGER			0x0E			//Page error used in 1 wire EEPROM.
#define		SPGER			0x0F			//Subpage error used in 1 wire EEPROM.
#define		BYTER			0x10			//Too many bytes to be read.
#define		COPYOK			0x11			//Copy scrachpad Ok.
#define		COPYER			0x12			//Copy scrachpad error.
#define		WRERR			0x13			//Write memory error.
#define		STPER			0x14			//Writing position error.
#define		FILBER			0x15			//Too many bytes to be writen to a subpage.
#define		RDEEER			0x16			//Read EEPROM error.
#define		P3PRO			0x17			//Page3 read protected.
#define		RWSCER			0x18			//Read Write scratch error.
#define		SETSEER			0x19			//Set secret error.
#define		LOADER			0x20			//Load secret error.
#define		RWEEP			0x21			//Indicating rewriting EEPROM.
