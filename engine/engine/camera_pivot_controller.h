#ifndef _CAMERA_PIVOT_CONTROLLER_H
#define _CAMERA_PIVOT_CONTROLLER_H

class CEntity;

class camera_pivot_controller
{
public:
	camera_pivot_controller();
	~camera_pivot_controller();
	void init(XMVECTOR pos, CEntity* player_ref);
	void update();
	void setCamPivotPos(XMVECTOR cam_pivot_pos);
	CEntity* getCamPivot();
	CEntity* getPlayerPivot();
private:
	XMVECTOR pos_ref;
	CEntity* player;
	CEntity* camera_pivot;
	CEntity* player_pivot;
};
#endif
