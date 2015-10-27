#ifndef INC_COMP_PLAYER_ROPE_H_
#define INC_COMP_PLAYER_ROPE_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompPlayerRope : TBaseComponent {

private:	

	std::vector<CHandle>* entity_chain;
	//std::vector<PxJoint*>* joint_chain;	
	//PxDistanceJoint* reel_joint;
	float links_distance;
	PxDistanceJoint* hand_joint;
	PxDistanceJoint* back_joint;
	PxRigidDynamic* first_chain;
	PxRigidDynamic* last_chain;
public:
	TCompPlayerRope();
	~TCompPlayerRope();

	void init();
	void update(float elapsed){};
	void fixedUpdate(float elapsed);
	void loadFromAtts(const std::string& elem, MKeyValue &atts);

};

#endif
