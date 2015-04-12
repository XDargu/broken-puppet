SLB.using(SLB)

function onSceneLoad_my_file()
	
	-- Get the player 
	player = getBot("Player");
	player:teleport(0, 100, 0);

	a = Vector(1, 2, 3);
	print(a.x);

end

-- Funciones útiles de LUA
_print = print

function print(s)
	logicManager:print(tostring(s));
end

function error(s)
	logicManager:print("ERROR: " .. tostring(s));
end

function getBot(name)
	return logicManager:getBot(tostring(name));
end

function onTriggerEnter_mTrigger(who)
	print(tostring(who) .. " ha entrado en el trigger");
end

