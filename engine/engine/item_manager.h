#ifndef INC_ITEM_MANAGER_H_
#define INC_ITEM_MANAGER_H_

struct TCompNeedle;
struct TCompTransform;

class Citem_manager
{
public:
	std::vector<TCompNeedle*> needles;
	Citem_manager();
	~Citem_manager();
	void addNeedle(TCompNeedle* n);
	//std::vector<TCompNeedle*> needleInRange(XMVECTOR pos, float radius);
	void removeNeedle(TCompNeedle* n);
	static Citem_manager& get();
};
#endif

