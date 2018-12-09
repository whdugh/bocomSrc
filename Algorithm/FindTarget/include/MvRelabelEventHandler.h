#ifndef MV_RELABEL_EVENT_HANDLER
#define MV_RELABEL_EVENT_HANDLER

class MvRelabelEventHandler
{
public:
	virtual void OnRelabeled(int nOldId, int nNewId)=0;
};

#endif