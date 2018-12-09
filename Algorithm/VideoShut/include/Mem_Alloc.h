#ifndef MEM_ALLOC_H_H
#define MEM_ALLOC_H_H


unsigned char *Mem_Alloc(int len);
unsigned char * Mem_Realloc(void *buffer,int nNew, int nOld);
void Mem_Free(void *ptr);
void Mem_Free_Ext(void *ptr, int len);



#endif  // MEM_ALLOC_H_H_