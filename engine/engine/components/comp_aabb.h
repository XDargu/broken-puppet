#ifndef INC_COMP_AABB_H_
#define INC_COMP_AABB_H_

#include "base_component.h"

struct TCompAABB : TBaseComponent{
private:
	CHandle		transform;
	XMVECTOR		bbpoints[8];	// Bounding Box with no rotation

	// Used to check if the transform has been moved since the last frame, should be moved to transform component
	XMVECTOR prev_position;
	XMVECTOR prev_rotation;
	XMVECTOR prev_scale;

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

	XMVECTOR min;
	XMVECTOR max;
	XMVECTOR getCenter() { return (min + max) / 2; };
	XMVECTOR getExtents() { return (max - min) / 2; };
	XMVECTOR getSize() { return max - min; };

	TCompAABB() {}

	void loadFromAtts(MKeyValue &atts) {
		XMVECTOR identity_min = atts.getPoint("min");
		XMVECTOR identity_max = atts.getPoint("max");

		setIdentityMinMax(identity_min, identity_max);
	}

	void init() {
		CEntity* e = CHandle(this).getOwner();
		transform = e->get<TCompTransform>();
		TCompTransform* trans = (TCompTransform*)transform;

		assert(trans || fatal("TAABB requieres a TTransform component"));

		prev_position = trans->position;
		prev_rotation = trans->rotation;
		prev_scale = trans->scale;
		recalcMinMax();
	}

	// Updates the min and max variables, if needed
	void update(float elapsed) {
		CEntity* e = CHandle(this).getOwner();
		TCompName* name = e->get<TCompName>();

		TCompTransform* trans = (TCompTransform*)transform;

		bool posEqual = XMVectorGetX(XMVectorEqual(prev_position, trans->position)) && XMVectorGetY(XMVectorEqual(prev_position, trans->position)) && XMVectorGetZ(XMVectorEqual(prev_position, trans->position));
		bool rotEqual = XMVectorGetX(XMVectorEqual(prev_rotation, trans->rotation)) && XMVectorGetY(XMVectorEqual(prev_rotation, trans->rotation)) && XMVectorGetZ(XMVectorEqual(prev_rotation, trans->rotation)) && XMVectorGetW(XMVectorEqual(prev_rotation, trans->rotation));
		bool sclEqual = XMVectorGetX(XMVectorEqual(prev_scale, trans->scale)) && XMVectorGetY(XMVectorEqual(prev_scale, trans->scale)) && XMVectorGetZ(XMVectorEqual(prev_scale, trans->scale));

		if (!(posEqual && rotEqual && sclEqual))
			recalcMinMax();

		prev_position = trans->position;
		prev_rotation = trans->rotation;
		prev_scale = trans->scale;
	}

	// Squared distance from a point to the AABB
	float sqrDistance(XMVECTOR point) {
		XMVECTOR nearestPoint = XMVectorZero();
		nearestPoint = XMVectorSetX(nearestPoint, (XMVectorGetX(point) <  XMVectorGetX(min)) ? XMVectorGetX(min) : (XMVectorGetX(point) > XMVectorGetX(max)) ? XMVectorGetX(max) : XMVectorGetX(point));
		nearestPoint = XMVectorSetY(nearestPoint, (XMVectorGetY(point) <  XMVectorGetY(min)) ? XMVectorGetY(min) : (XMVectorGetY(point) > XMVectorGetY(max)) ? XMVectorGetY(max) : XMVectorGetY(point));
		nearestPoint = XMVectorSetZ(nearestPoint, (XMVectorGetZ(point) <  XMVectorGetZ(min)) ? XMVectorGetZ(min) : (XMVectorGetZ(point) > XMVectorGetZ(max)) ? XMVectorGetZ(max) : XMVectorGetZ(point));

		return XMVectorGetX(XMVector3LengthSq(nearestPoint - point));
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

	// Check if the AABB contains a point
	bool containts(XMVECTOR point) {
		return XMVector3InBounds(point - getCenter(), getExtents());
	}

	// Checks if the aabb intersects with another one
	bool intersects(TCompAABB* aabb) {
		return XMVector3Greater(max, aabb->min) && XMVector3Greater(aabb->max, min);
	}

	std::string toString() {
		return "AABB Min: (" + std::to_string(XMVectorGetX(min)) + ", " + std::to_string(XMVectorGetY(min)) + ", " + std::to_string(XMVectorGetZ(min)) + ")" +
			"\nAABB Max: (" + std::to_string(XMVectorGetX(max)) + ", " + std::to_string(XMVectorGetY(max)) + ", " + std::to_string(XMVectorGetZ(max)) + ")";
	}
};


#endif
