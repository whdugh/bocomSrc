//This head file include variable declaration and function prototype.

extern uint8  SHAVM_Message[64];	//SHA-1 input buffer.
extern uint8  SHAVM_MAC[20];		//MAC buffer.
void SHAVM_Compute(void);
