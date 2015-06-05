#ifndef INC_ROPE_MANAGER_H_
#define INC_ROPE_MANAGER_H_

#include "handle\handle.h"
#include "entity_manager.h"


class CRope_manager
{
private:
	std::deque<CHandle> strings;
	static const int max_strings = 4;
public:
	CRope_manager();
	~CRope_manager();
	void addString(CHandle string);
	void removeString();
	void removeBackString();
	void clearStrings();
	static CRope_manager& get();
};
#endif

