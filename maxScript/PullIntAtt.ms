	/**********************************************************************************************
	Object  structure Interruptor
	 Params:
	  - Is an interruptor
	***********************************************************************************************/
	objectDataPullInt = attributes objectPullInt
	(
		parameters main rollout:params
		(
			PullInt type:#boolean ui:PullInt default:true
			Logic_element type:#boolean ui:Logic_element default:true
		)

		rollout params "Object Custom Trigger"
		(
			checkbox PullInt "PullInt" type: #boolean
			checkbox Logic_element "Logic_element" type: #boolean
		)
	)
	
	fn addObjectsPullInt = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #PullInt != true) then
		(
			custAttributes.add objSelected objectDataPullInt	
		)	
		
	)
	
	
	/**********************************************************************************************
	Object  structure CollisionTag
	 Params:
	  - Is an CollisionTag
	***********************************************************************************************/
	objectDataCollisionTag = attributes objectCollisionTag
	(
		parameters main rollout:params
		(
			collisionTag type:#string ui:collisionTag default:"undefined"
		)

		rollout params "Object Custom collision tag"
		(
			Edittext collisionTag "collisionTag" type: #string
		)
	)
	
	fn addObjectsCollisionTag = 
	(		
		local objSelected = $
		if (isProperty objSelected.baseobject #collisionTag != true) then
		(
			custAttributes.add objSelected objectDataCollisionTag	
		)	
		
	)