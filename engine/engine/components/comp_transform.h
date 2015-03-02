#ifndef INC_COMP_TRANSFORM_H_
#define INC_COMP_TRANSFORM_H_

#include "mcv_platform.h"
#include "base_component.h"
#include "transform.h"

// ----------------------------------------
struct TCompTransform : TBaseComponent {     // 1
	XMVECTOR position;
	XMVECTOR rotation;
	XMVECTOR scale;

	TCompTransform() : position(XMVectorSet(0.f, 0.f, 0.f, 1.f)), rotation(XMQuaternionIdentity()), scale(XMVectorSet(1, 1, 1, 1)) {}
	TCompTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : position(np), rotation(nr), scale(ns) {}

	void loadFromAtts(MKeyValue& atts) {
		position = atts.getPoint("position");
		rotation = atts.getQuat("rotation");
		scale = atts.getPoint("scale");
	}

	XMMATRIX getWorld() const {
		XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		XMMATRIX m = XMMatrixAffineTransformation(scale, zero, rotation, position);
		return m;
	}

	XMVECTOR getFront() const {
		XMMATRIX m = XMMatrixRotationQuaternion(rotation);
		return m.r[2];
	}

	XMVECTOR getLeft() const {
		XMMATRIX m = XMMatrixRotationQuaternion(rotation);
		return m.r[0];
	}

	XMVECTOR getUp() const {
		XMMATRIX m = XMMatrixRotationQuaternion(rotation);
		return m.r[1];
	}

	// Returns true if the point is in the positive part of my front
	bool isInFront(XMVECTOR loc) const {
		return XMVectorGetX(XMVector3Dot(getFront(), loc - position)) > 0.f;
	}

	bool isInLeft(XMVECTOR loc) const {
		return XMVectorGetX(XMVector3Dot(getLeft(), loc - position)) > 0.f;
	}

	bool isInFov(XMVECTOR loc, float fov_in_rad) const {
		XMVECTOR unit_delta = XMVector3Normalize(loc - position);
		float cos_angle = XMVectorGetX(XMVector3Dot(getFront(), unit_delta));
		return(cos_angle < cos(fov_in_rad * 0.5f));
	}

	// Aim the transform to a position instantly
	void lookAt(XMVECTOR new_target, XMVECTOR new_up_aux) {

		XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
		rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
	}

	// Aim the transform to a position with SLerp
	void aimAt(XMVECTOR new_target, XMVECTOR new_up_aux, float t) {
		XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
		rotation = XMQuaternionSlerp(rotation, XMQuaternionInverse(XMQuaternionRotationMatrix(view)), t);
	}

	// Transforms a direction from World space to Local space
	XMVECTOR inverseTransformDirection(XMVECTOR dir) {
		// Formula: (World direction - position) * inverse(rotation)
		return XMVector3Rotate((dir - position), XMQuaternionInverse(rotation));
	}

	std::string toString() {
		return "Position: (" + std::to_string(XMVectorGetX(position)) + ", " + std::to_string(XMVectorGetY(position)) + ", " + std::to_string(XMVectorGetZ(position)) + ")\n" +
			"Rotation: (" + std::to_string(XMVectorGetX(rotation)) + ", " + std::to_string(XMVectorGetY(rotation)) + ", " + std::to_string(XMVectorGetZ(rotation)) + ", " + std::to_string(XMVectorGetW(rotation)) + ")\n" +
			"Scale: (" + std::to_string(XMVectorGetX(scale)) + ", " + std::to_string(XMVectorGetY(scale)) + ", " + std::to_string(XMVectorGetZ(scale)) + ")";
	}
};

#endif
