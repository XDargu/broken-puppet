clearlistener()

allObjects = $*

objectDataCA = attributes objectData
(
parameters main rollout:params
(
id type:#float ui:ID default:0
cost type:#float ui:cost default:100
sound type:#string
)
 
rollout params "Object Custom Parameters"
(
spinner ID "ID" type: #float
spinner cost "Cost" type: #float
dropdownlist sound_dd "Sound" items:# ("wood","iron","mud","water")
 
on sound_dd selected i do sound = sound_dd.items [i]
)
)


for o in allObjects do
(
	print  "------------------------"	
	custAttributes.add o objectDataCA
	--print o.modifiers[#PhysX_Rigid_Body].
	print  "------------------------"
)	


