#ifndef INC_DOOM_CONTROLLER_H_
#define INC_DOOM_CONTROLLER_H_

class CEntity;

// -------------------------------
class CDoomController {
  float movement_velocity;
  float rotation_velocity;

public:
  CDoomController();
  void update(CEntity* e, float delta_time);
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
