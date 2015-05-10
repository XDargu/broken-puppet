#include "mcv_platform.h"
#include "comp_platform_path.h"
#include "comp_rigid_body.h"

void TCompPlatformPath::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	if (elem == "platformPath") {
		movement_speed = atts.getFloat("speed", 2);
	}
	// Get the list of points
	if (elem == "point") {
		mPath.push_back(Physics.XMVECTORToPxVec3(atts.getPoint("position")));
	}
}

void TCompPlatformPath::init(){

	follow_line = false;

	next_target = 0;
	distance_enough = 0.5f;

	mRigidbody = assertRequiredComponent<TCompRigidBody>(this);

	// Check if there are a rigid body in the entity
	XASSERT(mRigidbody.isValid(), "The comp platform path requires a ridigbody in the entity");
}

void TCompPlatformPath::fixedUpdate(float elapsed) {

	if (follow_line != true) return;

	PxRigidDynamic* px_platform = ((TCompRigidBody*)mRigidbody)->rigidBody;

	// Check if we have arrived
	float distance = (px_platform->getGlobalPose().p - mPath[next_target]).magnitude();

	if (distance <= distance_enough){
		// If the platform arrived, then change objetive
		next_target++;
		next_target = next_target % mPath.size();
	}

	next_pos = mPath[next_target];

	// Follow the point
	PxTransform my_trans = px_platform->getGlobalPose();

	PxVec3 dir = next_pos - my_trans.p;
	dir.normalize();

	float speed = lerp(distance, movement_speed, .8f);

	my_trans.p = my_trans.p + (dir * speed * elapsed);

	px_platform->setKinematicTarget(my_trans);
}