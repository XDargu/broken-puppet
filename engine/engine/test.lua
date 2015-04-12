SLB.using(SLB)


function onSceneInit()
	
	-- Get the player 
	player = logicManager:getBot("Player");	

end

_print = print

function print(s)
	logicManager:print(tostring(s));
end