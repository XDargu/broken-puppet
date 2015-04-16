SLB.using(SLB)

-- Funciones útiles de LUA
function print(s)
	logicManager:print(tostring(s));
end
function error(s)
	logicManager:print("ERROR: " .. tostring(s));
end

_print = print

-- Funciones del nivel
function f_generator()
	local value = 1111
	while true do
		coroutine.yield( value / 40847 )
		value = ( 837410213 * value ) % 40847
	end
end

function getRnd()
	all_ok, out = coroutine.resume( co_generator )
	return out
end

function onSceneLoad_my_file()
	
	-- Get the player 
	player = getBot("Player");
	player:teleport(0, 10, 0);

	print("Scene loaded")

	co_generator = coroutine.create(f_generator)

	--[[samples = 50000
	v_02 = 0
	v_24 = 0
	v_46 = 0
	v_68 = 0
	v_80 = 0
	a = 0
	total = 0
	while a < samples do
		out = getRnd()
		--print("VALUE: " .. tostring(out))
		total = total + out
		a = a + 1

		if out < 0.2 then
			v_02 = v_02 + 1
		end
		if out < 0.4 and out > 0.2 then
			v_24 = v_24 + 1
		end
		if out < 0.6 and out > 0.4 then
			v_46 = v_46 + 1
		end
		if out < 0.8 and out > 0.6 then
			v_68 = v_68 + 1
		end
		if out > 0.8 then
			v_80 = v_80 + 1
		end
	end

	print(samples .. " números aleatorios generados")
	print("Media: " .. total / samples)
	print("Valores entre 0 y 0.2: " .. v_02)
	print("Valores entre 0.2 y 0.4: " .. v_24)
	print("Valores entre 0.4 y 0.6: " .. v_46)
	print("Valores entre 0.6 y 0.8: " .. v_68)
	print("Valores entre 0.8 y 1: " .. v_80)]]--

end

function getBot(name)
	return logicManager:getBot(tostring(name));
end

-- Funciones de la escena

function onTriggerEnter_mTrigger(who)
	print(tostring(who) .. " ha entrado en el trigger");
end

function onSwitchPressed_int_pull_51()
	print("Interruptor pulsado");
end

function onSwitchReleased_int_pull_51()
	print("Interruptor soltado");
end