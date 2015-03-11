#ifndef INC_COMP_LIFE_H_
#define INC_COMP_LIFE_H_

#include "base_component.h"

struct TCompLife : TBaseComponent {    // 2 ...
  float life;

  TCompLife() : life(100.0f) { }
  TCompLife(float alife) : life(alife) { }

  void loadFromAtts(const std::string& elem, MKeyValue &atts) {
    life = atts.getFloat("life", 100.f);
  }

  bool Hurt(float damage){
	  life -= damage;
	  if (life <= 0)
		  life = 0;
	  return life <= 0;
  }

  std::string toString() {
    return "Life: " + std::to_string(life);
  }
  void onExplosion(const TMsgExplosion& msg) {
    dbg("Life recv explosion of %f points when my life is %f\n", msg.damage, life);
    life -= msg.damage;
    if (life < 0) 
      life = 0;
  }
  void onDied(const TMsgDied& msg) {
    dbg("Life recv died from %d when my life is %f\n", msg.who, life);
  }
};

#endif
