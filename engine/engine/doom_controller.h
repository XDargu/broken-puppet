#ifndef INC_DOOM_CONTROLLER_H_
#define INC_DOOM_CONTROLLER_H_

class CEntityOld;

// -------------------------------
class CDoomController {
  float movement_velocity;
  float rotation_velocity;

public:
  CDoomController();
  void update(CEntityOld* e, float delta_time);
};

// -------------------------------
class CAimToController {
  float rotation_velocity;

public:
  CAimToController();
  void update(CEntityOld* who, CEntityOld* target, float delta_time);
};

// -------------------------------
class CLookAtController {
public:
  void update(CEntityOld* who, CEntityOld* target, float delta_time);
};

#endif
