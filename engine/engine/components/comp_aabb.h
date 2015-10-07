#ifndef INC_COMP_AABB_H_
#define INC_COMP_AABB_H_

#include "base_component.h"
#include "comp_transform.h"
#include "aabb.h"
#include "ai\logic_manager.h"

struct TCompAABB : public AABB, TBaseComponent{
private:
	CHandle		transform;
	XMVECTOR	bbpoints[8];	// Bounding Box with no rotation
	float		counter;

	void recalcMinMax() {
		// Recalcultate AABB:
		// - Take the non rotated AABB min and max position and calculate the 8 bounding box points
		// - Apply a rotation to those points
		// - Find the min and max values for the points
		// - Set min and max new values		

		XMVECTOR rotatePoint;
		XMVECTOR minValue = XMVectorSet(D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX, 1);
		XMVECTOR maxValue = -minValue;
		for (int i = 0; i < 8; ++i) {
			rotatePoint = XMVector3Rotate(bbpoints[i] * ((TCompTransform*)transform)->scale, ((TCompTransform*)transform)->rotation);

			if (XMVectorGetX(rotatePoint) < XMVectorGetX(minValue))
				minValue = XMVectorSetX(minValue, XMVectorGetX(rotatePoint));
			if (XMVectorGetY(rotatePoint) < XMVectorGetY(minValue))
				minValue = XMVectorSetY(minValue, XMVectorGetY(rotatePoint));
			if (XMVectorGetZ(rotatePoint) < XMVectorGetZ(minValue))
				minValue = XMVectorSetZ(minValue, XMVectorGetZ(rotatePoint));

			if (XMVectorGetX(rotatePoint) > XMVectorGetX(maxValue))
				maxValue = XMVectorSetX(maxValue, XMVectorGetX(rotatePoint));
			if (XMVectorGetY(rotatePoint) > XMVectorGetY(maxValue))
				maxValue = XMVectorSetY(maxValue, XMVectorGetY(rotatePoint));
			if (XMVectorGetZ(rotatePoint) > XMVectorGetZ(maxValue))
				maxValue = XMVectorSetZ(maxValue, XMVectorGetZ(rotatePoint));
		}

		min = ((TCompTransform*)transform)->position + minValue;
		max = ((TCompTransform*)transform)->position + maxValue;
	}
public:
	bool		auto_update;

	TCompAABB() : AABB() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		XMVECTOR identity_min = atts.getPoint("min");
		XMVECTOR identity_max = atts.getPoint("max");

		setIdentityMinMax(identity_min, identity_max);
		transform = assertRequiredComponent<TCompTransform>(this);
		TCompTransform* trans = (TCompTransform*)transform;

		recalcMinMax();
		counter = 0;
	}

	void init() {
		
	}

	// Updates the min and max variables, if needed
	void update(float elapsed) {
		if (auto_update) {
			TCompTransform* trans = (TCompTransform*)transform;

			/*bool posEqual = XMVectorGetX(XMVectorEqual(prev_position, trans->position)) && XMVectorGetY(XMVectorEqual(prev_position, trans->position)) && XMVectorGetZ(XMVectorEqual(prev_position, trans->position));
			bool rotEqual = XMVectorGetX(XMVectorEqual(prev_rotation, trans->rotation)) && XMVectorGetY(XMVectorEqual(prev_rotation, trans->rotation)) && XMVectorGetZ(XMVectorEqual(prev_rotation, trans->rotation)) && XMVectorGetW(XMVectorEqual(prev_rotation, trans->rotation));
			bool sclEqual = XMVectorGetX(XMVectorEqual(prev_scale, trans->scale)) && XMVectorGetY(XMVectorEqual(prev_scale, trans->scale)) && XMVectorGetZ(XMVectorEqual(prev_scale, trans->scale));*/

			counter += elapsed;
			if (trans->transformChanged()) {
				recalcMinMax();
				if (counter > 1) {
					((TCompTransform*)transform)->room_id = CLogicManager::get().getPointZoneID(((TCompTransform*)transform)->position);
					counter = 0;
				}
			}
		}
		else {
			counter = 0;
		}
	}

	// Sets the identity rotation AABB points
	void setIdentityMinMax(XMVECTOR identity_min, XMVECTOR identity_max) {

		// Initialize bbpoints		
		bbpoints[0] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_min), XMVectorGetZ(identity_min), 1);
		bbpoints[1] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_min), XMVectorGetZ(identity_max), 1);
		bbpoints[2] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_max), XMVectorGetZ(identity_min), 1);
		bbpoints[3] = XMVectorSet(XMVectorGetX(identity_min), XMVectorGetY(identity_max), XMVectorGetZ(identity_max), 1);
		bbpoints[4] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_min), XMVectorGetZ(identity_min), 1);
		bbpoints[5] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_min), XMVectorGetZ(identity_max), 1);
		bbpoints[6] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_max), XMVectorGetZ(identity_min), 1);
		bbpoints[7] = XMVectorSet(XMVectorGetX(identity_max), XMVectorGetY(identity_max), XMVectorGetZ(identity_max), 1);
	}
};


#endif
