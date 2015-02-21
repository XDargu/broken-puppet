clearlistener()

PlayerName = "Player"
PlayerPivotName = "PlayerPivot"
CameraPivotName = "CameraPivot"
CameraName = "PlayerCamera"

addPlayerController ()
addPlayerPivotController ()
addCameraPivotController ()
addCameraController ()

/**********************************************************************************************
Object Data Player Controller structure
 Params:
  - mevementVelocity
  - rotationVelocity
***********************************************************************************************/
objectDataPlayerController = attributes objectPlayerData
(
	parameters main rollout:params
	(
		movementVelocity type:#float ui:movementVelocity default:6
		rotationVelocity type:#float ui:rotationVelocity default:90
	)
	 
	rollout params "Player Controller"
	(
		spinner movementVelocity "movementVelocity" range:[-500,500,6]  type: #float
		spinner rotationVelocity "rotationVelocity" range:[-500,500,90]  type: #float
	)
)

/**********************************************************************************************
Set the Player Controller to the player
***********************************************************************************************/
fn addPlayerController = (
	local allObjects = $*
	
	for obj in allObjects do
	(	
		if (obj.name == PlayerName) then
		(
			if (isProperty obj.baseObject #objectPlayerData != true) then
			(
				custAttributes.add obj objectDataPlayerController		
			)	
		)
	)
)


/**********************************************************************************************
Object Data Player Pivot Controller structure
 Params:
  - rotationVelocity
***********************************************************************************************/
objectDataPlayerPivotController = attributes objectPlayerPivotData
(
	parameters main rollout:params
	(
		rotationVelocity type:#float ui:rotationVelocity default:270
	)
	 
	rollout params "Player Pivot Controller"
	(
		spinner rotationVelocity "rotationVelocity"  range:[-500,500,270] type: #float
	)
)

/**********************************************************************************************
Set the Player Pivot Controller
***********************************************************************************************/
fn addPlayerPivotController = (
	local allObjects = $*
	
	for obj in allObjects do
	(	
		if (obj.name == PlayerPivotName) then
		(
			if (isProperty obj.baseObject #objectPlayerPivotData != true) then
			(
				custAttributes.add obj objectDataPlayerPivotController		
			)	
		)
	)
)


/**********************************************************************************************
Object Data Camera Pivot Controller structure
 Params:
  - rotationVelocity
***********************************************************************************************/
objectDataCameraPivotController = attributes objectCameraPivotData
(
	parameters main rollout:params
	(
		tiltVelocity type:#float ui:tiltVelocity default:270
	)
	 
	rollout params "Camera Pivot Controller"
	(
		spinner tiltVelocity "tiltVelocity" range:[-500,500,270]  type: #float
	)
)

/**********************************************************************************************
Set the Camera Pivot Controller
***********************************************************************************************/
fn addCameraPivotController = (
	local allObjects = $*
	
	for obj in allObjects do
	(	
		if (obj.name == CameraPivotName) then
		(
			if (isProperty obj.baseObject #objectCameraPivotData != true) then
			(
				custAttributes.add obj objectDataCameraPivotController		
			)	
		)
	)
)


/**********************************************************************************************
Object Data Camera Controller structure
 Params:
  - rotationVelocity
***********************************************************************************************/
objectDataCamera = attributes objectCameraData
(
	parameters main rollout:params
	(		
		offsetX type:#float ui:offsetX default:0.2
		offsetY type:#float ui:offsetY default:-0.33
		offsetZ type:#float ui:offsetZ default:2
	)
	 
	rollout params "Camera Controller"
	(
		spinner offsetX "offsetX" range:[-500,500,0.2]  type: #float
		spinner offsetY "offsetY" range:[-500,500,-0.33]  type: #float
		spinner offsetZ "offsetZ" range:[-500,500,2]  type: #float
	)
)

/**********************************************************************************************
Set the Camera Controller
***********************************************************************************************/
fn addCameraController = (
	local allObjects = $*
	 
	for obj in allObjects do
	(	
		if (obj.name == CameraName) then
		(
			
			if (isProperty obj.baseObject #objectCameraData != true) then
			(
				custAttributes.add obj objectDataCamera		
			)	
		)
	)
)