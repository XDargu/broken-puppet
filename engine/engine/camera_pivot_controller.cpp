#include "mcv_platform.h"
#include "camera_pivot_controller.h"
#include "iostatus.h"
//#include "Player.h"

using namespace DirectX;

//Player			&player_ref = Player::get();

camera_pivot_controller::camera_pivot_controller()
{
}

void camera_pivot_controller::init(XMVECTOR pos, CEntity* player_ref){
	pos_ref = pos;
	assert(player_ref!=nullptr);
	player = player_ref;
	camera_pivot = entity_manager.create("Camera Pivot");
	camera_pivot->setPosition(player_ref->getPosition());
	camera_pivot->setRotation(player_ref->getRotation());

	player_pivot = entity_manager.create("Player Pivot");
	player_pivot->setPosition(player_ref->getPosition());
	player_pivot->setRotation(player_ref->getRotation());
}

CEntity* camera_pivot_controller::getCamPivot(){
	return camera_pivot;
}

CEntity* camera_pivot_controller::getPlayerPivot(){
	return player_pivot;
}

void camera_pivot_controller::setCamPivotPos(XMVECTOR cam_pivot_pos){
	assert(player != nullptr);
	assert(camera_pivot != nullptr);
	camera_pivot->setPosition(cam_pivot_pos);
	camera_pivot->setRotation(player->getRotation());
}

camera_pivot_controller::~camera_pivot_controller()
{
}

void camera_pivot_controller::update(){
	float x_dif = XMVectorGetX(pos_ref);
	float y_dif = XMVectorGetY(pos_ref);
	float z_dif = XMVectorGetZ(pos_ref);

	camera_pivot->setPosition(
		player_pivot->getPosition() 
			+ player_pivot->getUp() * y_dif 
			+ player_pivot->getLeft() * x_dif 
			+ player_pivot->getFront()*z_dif
		);
	player_pivot->setPosition(player->getPosition());
}
