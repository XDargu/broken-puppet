#ifndef INC_ITEM_MANAGER_H_
#define INC_ITEM_MANAGER_H_

struct TCompNeedle;
struct TCompTransform;
struct TCompRope;

struct needle_rope;

class Citem_manager
{
public:
	std::vector<needle_rope> needles;
	Citem_manager();
	~Citem_manager();
	void addNeedle(TCompNeedle* needle, TCompRope* rope);
	//std::vector<TCompNeedle*> needleInRange(XMVECTOR pos, float radius);
	void removeNeedle(needle_rope n);
	static Citem_manager& get();
};
#endif

