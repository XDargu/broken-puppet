clearlistener()

allObjects = $*

for o in allObjects do
	if (o.modifiers[#PhysX_Rigid_Body] != undefined) then
(
	print  "------------------------"
	print o.name
	print o.modifiers[#PhysX_Rigid_Body].type	-- tipe 1 = Dinamic, tipe 2 = Kinematic, tipe 3 = Static
	print o.modifiers[#PhysX_Rigid_Body].mass
	print o.modifiers[#PhysX_Rigid_Body].density
	print o.modifiers[#PhysX_Rigid_Body].StaticFriction
	print o.modifiers[#PhysX_Rigid_Body].DynamicFriction
	print o.modifiers[#PhysX_Rigid_Body].Bounciness
	print o.modifiers[#PhysX_Rigid_Body].MeshType
	
	--print o.modifiers[#PhysX_Rigid_Body].
	print  "------------------------"
)	



-- <objPhysx>