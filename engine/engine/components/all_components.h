#ifndef INC_ALL_COMPONENTS_H_
#define INC_ALL_COMPONENTS_H_

#include "mcv_platform.h"
#include "XMLParser.h"
#include "handle/handle.h"
#include "../entity_manager.h"
#include "../render/collision_mesh.h"

// Basic
#include "comp_name.h"
#include "comp_transform.h"

#include "comp_life.h"
#include "comp_aabb.h"
#include "comp_camera.h"
#include "comp_tag.h"

// Controllers
#include "comp_player_controller.h"
#include "comp_player_pivot_controller.h"
#include "comp_camera_pivot_controller.h"
#include "comp_third_person_camera_controller.h"
#include "comp_enemy_controller.h"
#include "comp_unity_character_controller.h"
#include "comp_basic_player_controller.h"

// Physx
#include "comp_collider.h"
#include "comp_collider_sphere.h"
#include "comp_collider_mesh.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "comp_distance_joint.h"

// Render
#include "comp_mesh.h"
#include "comp_directional_light.h"
#include "comp_ambient_light.h"
#include "comp_point_light.h"

// AI
#include "comp_ai_fsm_basic.h"

// Varios
#include "comp_needle.h"
#include "comp_rope.h"
#include "comp_trigger.h"
#include "comp_victory_cond.h"
#include "comp_distance_text.h"

#endif
