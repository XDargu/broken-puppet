#ifndef INC_TRANSFORM_H_
#define INC_TRANSFORM_H_

#include "mcv_platform.h"

// ----------------------------------------
struct TTransform {     // 1
	XMVECTOR position;
	XMVECTOR rotation;
	XMVECTOR scale;

	TTransform() : position(XMVectorSet(0.f, 0.f, 0.f, 1.f)), rotation(XMQuaternionIdentity()), scale(XMVectorSet(1, 1, 1, 1)) {}
	TTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : position(np), rotation(nr), scale(ns) {}

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

	bool isInUp(XMVECTOR loc) const {
		return XMVectorGetX(XMVector3Dot(getUp(), loc - position)) > 0.f;
	}

	bool isInFov(XMVECTOR loc, float fov_in_rad) const {
		XMVECTOR unit_delta = XMVector3Normalize(loc - position);
		float cos_angle = XMVectorGetX(XMVector3Dot(getFront(), unit_delta));
		return(cos_angle > cos(fov_in_rad * 0.5f));
	}

	// Aim the transform to a position instantly
	void lookAt(XMVECTOR new_target, XMVECTOR new_up_aux) {
		bool posEqual = XMVectorGetX(XMVectorEqual(new_target, position)) && XMVectorGetY(XMVectorEqual(new_target, position)) && XMVectorGetZ(XMVectorEqual(new_target, position));
		if (!posEqual) {
			XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
			rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
		}
	}

	// Aim the transform to a position with SLerp
	void aimAt(XMVECTOR new_target, XMVECTOR new_up_aux, float t) {
		bool posEqual = XMVectorGetX(XMVectorEqual(new_target, position)) && XMVectorGetY(XMVectorEqual(new_target, position)) && XMVectorGetZ(XMVectorEqual(new_target, position));
		if (!posEqual) {
			XMMATRIX view = XMMatrixLookAtRH(position, position - (new_target - position), new_up_aux);
			rotation = XMQuaternionSlerp(rotation, XMQuaternionInverse(XMQuaternionRotationMatrix(view)), t);
		}
	}

	// Transforms a direction from World space to Local space
	XMVECTOR inverseTransformDirection(XMVECTOR dir) {
		// Formula: (World direction - position) * inverse(rotation)
		return XMVector3Rotate((dir - position), XMQuaternionInverse(rotation));
	}

	// Transform position from local space to world space
	XMVECTOR transformPoint(XMVECTOR point) {
		return position + XMVector3Rotate(point, rotation);
	}

	// Transform position from world space to local space
	XMVECTOR inverseTransformPoint(XMVECTOR point) {		
		return position - XMVector3Rotate(point, XMQuaternionInverse(rotation));
	}

	// Transform rotation from local space to world space
	XMVECTOR transformDirection(XMVECTOR dir) {
		return XMQuaternionMultiply(rotation, dir);
	}

	TTransform transform(TTransform other_trans) {
		TTransform ret = TTransform(position, rotation, scale);
		ret.position = ret.position + XMVector3Rotate(other_trans.position, ret.rotation);
		ret.rotation = XMQuaternionMultiply(other_trans.rotation, ret.rotation);
		return ret;
	}

};

#endif
