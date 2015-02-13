#ifndef INC_DOOM_CONTROLLER_H_
#define INC_DOOM_CONTROLLER_H_

class CEntity;

// -------------------------------
class CThirdPersonController {
  float movement_velocity;
  float rotation_velocity;

public:
	CThirdPersonController();
  void update(CEntity* e, CEntity* pivot, float delta_time);
};

// -------------------------------
class CAimToController {
  float rotation_velocity;

public:
  CAimToController();
  void update(CEntity* who, CEntity* target, float delta_time);
};

// -------------------------------
class CLookAtController {
public:
  void update(CEntity* who, CEntity* target, float delta_time);
};

#endif
