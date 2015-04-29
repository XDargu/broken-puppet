#include "mcv_platform.h"
#include "all_components.h"
#include "comp_skeleton.h"
#include "comp_skeleton_lookat.h"
#include "comp_skeleton_ik.h"



DECL_OBJ_MANAGER(CEntity, "entity");
DECL_OBJ_MANAGER(TCompTransform, "transform");
DECL_OBJ_MANAGER(TCompLife, "life");
DECL_OBJ_MANAGER(TCompName, "name");
DECL_OBJ_MANAGER(TCompMesh, "mesh");
DECL_OBJ_MANAGER(TCompRender, "render");
DECL_OBJ_MANAGER(TCompCamera, "camera");
DECL_OBJ_MANAGER(TCompColliderBox, "collider");
DECL_OBJ_MANAGER(TCompColliderSphere, "colliderSphere");
DECL_OBJ_MANAGER(TCompColliderCapsule, "colliderCapsule");
DECL_OBJ_MANAGER(TCompRigidBody, "rigidbody");
DECL_OBJ_MANAGER(TCompStaticBody, "staticbody");
DECL_OBJ_MANAGER(TCompPlayerController, "playerController");
DECL_OBJ_MANAGER(TCompPlayerPivotController, "playerPivotController");
DECL_OBJ_MANAGER(TCompCameraPivotController, "cameraPivotController");
DECL_OBJ_MANAGER(TCompThirdPersonCameraController, "thirdPersonCameraController");
DECL_OBJ_MANAGER(TCompViewerCameraController, "viewerCameraController");
DECL_OBJ_MANAGER(TCompDistanceJoint, "distanceJoint");
DECL_OBJ_MANAGER(TCompDirectionalLight, "directionalLight");
DECL_OBJ_MANAGER(TCompAmbientLight, "ambientLight");
DECL_OBJ_MANAGER(TCompPointLight, "pointLight");
DECL_OBJ_MANAGER(TCompColliderMesh, "colliderMesh");
DECL_OBJ_MANAGER(TCompAABB, "aabb");
DECL_OBJ_MANAGER(TCompAiFsmBasic, "aiFSMBasic");
DECL_OBJ_MANAGER(TCompAiBT, "aiBT");
DECL_OBJ_MANAGER(TCompBtGrandma, "btGrandma");
DECL_OBJ_MANAGER(TCompEnemyController, "enemyController");
DECL_OBJ_MANAGER(TCompNeedle, "needle");
DECL_OBJ_MANAGER(TCompRope, "rope");
DECL_OBJ_MANAGER(TCompUnityCharacterController, "unityCharacterController");
DECL_OBJ_MANAGER(TCompCharacterController, "characterController");
DECL_OBJ_MANAGER(TCompTrigger, "trigger");
DECL_OBJ_MANAGER(TCompBasicPlayerController, "basicPlayerController");
DECL_OBJ_MANAGER(TCompVictoryCond, "victoryCondition"); 
DECL_OBJ_MANAGER(TCompDistanceText, "distanceText");
DECL_OBJ_MANAGER(TCompJointPrismatic, "jointPrismatic");
DECL_OBJ_MANAGER(TCompJointHinge, "jointHinge");
DECL_OBJ_MANAGER(TCompSkeleton, "skeleton");
DECL_OBJ_MANAGER(TCompSkeletonLookAt, "skeleton_lookat");
DECL_OBJ_MANAGER(TCompSkeletonIK, "skeleton_ik");
DECL_OBJ_MANAGER(TCompPlayerPosSensor, "sensorPlayerPos");
DECL_OBJ_MANAGER(TCompSensorNeedles, "sensorNeedles");
DECL_OBJ_MANAGER(TCompSwitchController, "switchController");
DECL_OBJ_MANAGER(TCompSwitchPushController, "switchPushController");
DECL_OBJ_MANAGER(TCompSwitchPullController, "switchPullController");
DECL_OBJ_MANAGER(TCompPlatformPath, "platformPath");
DECL_OBJ_MANAGER(TCompRagdoll, "ragdoll");
DECL_OBJ_MANAGER(TCompShadows, "shadows");
