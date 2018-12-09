#ifndef MV_UION_MANAGER
#define MV_UION_MANAGER

#include <map>
#include <vector>
#include "CarManager.h"
#include "MvLoopManager.h"
#include "MvUnion.h"
#include "MvFindTargetByVirtualLoop.h"

using namespace PRJ_NAMESPACE_NAME;


class MvLoopManager;

class MvUnionManager
{
public:

	// 
	MvUnionManager(CarManager *pCarManager, MvLoopManager* pLoopManager, VirtualLoop *pVlp, MvFindTargetByVirtualLoop *pFtg);

	~MvUnionManager();

	// 将车牌、线圈目标、vlp目标抽取过来形成结合体。
	void Update(unsigned int uFrameSeq, int64 ts);


	// 提取需要输出的union，放入ret
	void Output( std::vector<_MvUnion*> &ret, unsigned int uFrameSeq, int64 ts );


	//



	// 删除需要删除的Union
	void Delete(unsigned int uFrameSeq, int64 ts);

private:


	// 结合体Id-> 结合体。存储了所有的union
	std::map<int, _MvUnion*> m_mapUnions;


	//
	CarManager*    m_pCarManager;
	MvLoopManager* m_pLoopManager;
	MvFindTargetByVirtualLoop* m_pFtg;
};

#endif
