#ifndef _FLASHSERIAL_H_
#define _FLASHSERIAL_H_

#include <stdlib.h>
#include "AbstractSerial.h"


using namespace std;

class FlashSerial :public AbstractSerial
{
    private:
         FlashSerial(void);
    public:
        ~FlashSerial(void);
        static FlashSerial * CreateStance()
        {
            return m_flashSerial;
        }
        bool OpenDev();
        bool InputCmd(int,int);
        //void ChangeToASCII(char *InputString, string& OutputString);
        unsigned char Checksum(char* chksumData);
    private:
      static FlashSerial *m_flashSerial;
};

#endif
