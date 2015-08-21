#ifndef INC_ALL_COMPONENTS_H_
#define INC_ALL_COMPONENTS_H_

#include "physics_manager.h"

using namespace DirectX;

#include "mcv_platform.h"
#include "XMLParser.h"
#include "handle/handle.h"
#include "../entity_manager.h"
#include "../render/collision_mesh.h"
#include "../render/collision_convex.h"

// Basic
#include "comp_name.h"
#include "comp_transform.h"

#include "comp_life.h"
#include "comp_aabb.h"
#include "comp_camera.h"

// Controllers
#include "comp_player_pivot_controller.h"
#include "comp_camera_pivot_controller.h"
#include "comp_viewer_camera_controller.h"
#include "comp_third_person_camera_controller.h"
#include "comp_enemy_controller.h"
#include "comp_unity_character_controller.h"
#include "comp_basic_player_controller.h"

// Physx
#include "comp_collider_box.h"
#include "comp_collider_sphere.h"
#include "comp_collider_capsule.h"
#include "comp_collider_mesh.h"
#include "comp_collider_convex.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "comp_distance_joint.h"
#include "comp_joint_prismatic.h"
#include "comp_joint_hinge.h"
#include "comp_joint_d6.h"
#include "comp_joint_fixed.h"
#include "comp_ragdoll.h"

// Render
#include "comp_render.h"
#include "comp_mesh.h"
#include "comp_directional_light.h"
#include "comp_ambient_light.h"
#include "comp_point_light.h"
#include "comp_shadows.h"
#include "comp_particle_group.h"
#include "comp_occlusion_plane.h"

// Controllers 2
#include "comp_character_controller.h"
#include "comp_player_controller.h"

// AI
#include "comp_ai_fsm_basic.h"
#include "comp_ai_bt.h"
#include "comp_player_position_sensor.h"
#include "comp_sensor_needles.h"
#include "comp_sensor_tied.h"
#include "comp_sensor_distance_player.h"
#include "comp_bt_grandma.h"
#include "comp_bt_soldier.h"
#include "comp_recast_aabb.h"

// Logic
#include "comp_platform_path.h"
#include "comp_switch_controller.h"
#include "comp_switch_push_controller.h"
#include "comp_switch_pull_controller.h"
#include "comp_golden_needle_logic.h"
#include "comp_zone_aabb.h"

// Varios
#include "comp_needle.h"
#include "comp_rope.h"
#include "comp_trigger.h"
#include "comp_victory_cond.h"
#include "comp_distance_text.h"
#include "comp_audio_listener.h"
#include "comp_audio_source.h"
#include "comp_particle_editor.h"
#include "comp_anim_editor.h"
#include "comp_golden_needle.h"
#include "comp_golden_needle_item.h"
#include "comp_hfx_zone.h"

#endif
