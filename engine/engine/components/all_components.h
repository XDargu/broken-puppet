#ifndef INC_ALL_COMPONENTS_H_
#define INC_ALL_COMPONENTS_H_

#include "mcv_platform.h"
#include "XMLParser.h"
#include "handle/handle.h"
#include "../entity_manager.h"

// Basic
#include "comp_name.h"
#include "comp_transform.h"

#include "comp_life.h"
#include "comp_aabb.h"
#include "comp_camera.h"

// Controllers
#include "comp_player_controller.h"
#include "comp_player_pivot_controller.h"
#include "comp_camera_pivot_controller.h"
#include "comp_third_person_camera_controller.h"

// Physx
#include "comp_collider.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "comp_distance_joint.h"

// Render
#include "comp_mesh.h"
#include "comp_directional_light.h"
#include "comp_ambient_light.h"
#include "comp_point_light.h"

#endif
