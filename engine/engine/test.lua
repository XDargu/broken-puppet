SLB.using(SLB)

-- Funciones �tiles de LUA
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

function onSceneLoad_my_file2()

	-- Test anim
	obj = logicManager:getObject("dvn_rampa_bloque_2m_simple_01_109.0")
	
	obj:move(Vector(51.0528, 8.24185, 28.1348), 1)
	
	-- Get the player 
	player = getBot("Player");
	-- player:teleport(0, 10, 0);

	print("Scene loaded")

	co_generator = coroutine.create(f_generator)

	-- Star gear motors

	ActiveGears();

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

	print(samples .. " n�meros aleatorios generados")
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
	logicManager:setTimer("Load", 3)
end

function onSwitchReleased_int_pull_51()
	print("Interruptor soltado");
end


function onTimerEnd_Load()
	logicManager:loadScene("data/scenes/milestone2.xml");
end

function onTimerEnd_GrandmaCrasher003Restore()
	local crasher = logicManager:getPrismaticJoint("GrandmaCrasher003")
	crasher:setLinearLimit(2.4, 10000000, 10000000)
	print("Timer ends")
end


function onTriggerEnter_GrandmaCrasher003_Trigger(who)
	print(tostring(who) .. " ha entrado en el trigger")
	local crasher = logicManager:getPrismaticJoint("GrandmaCrasher003")
	crasher:setLinearLimit(1000, 0, 0)

	logicManager:setTimer("GrandmaCrasher003Restore", 3)
end

-------------------------- MS3 ---------------------------

----------------------------------------------------------
------- Escene 1 ------- Escene 1 ------- Escene 1 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_1()

	local crasher1 = logicManager:getPrismaticJoint("escene_1_grandmacrasher_01");
	crasher1:setLinearLimit(0.1, 10000000, 10000000);
	local crasher2 = logicManager:getPrismaticJoint("escene_1_grandmacrasher_02");
	crasher2:setLinearLimit(0.1, 10000000, 10000000);

	local m_door_L = logicManager:getHingeJoint("escene_1_door_L");
	m_door_L:setMotor(-1.55, 20000);

	local m_door_R = logicManager:getHingeJoint("escene_1_door_R");
	m_door_R:setMotor(1.55, 20000);

		-- PUZZLE ATREZZO

	function onSwitchPressed_escene_1_push_int(who)

		print(tostring(who) .. "Interruptor pulsado escene_1_door_switch");
		-- local m_door_L = logicManager:getHingeJoint("escene_1_door_L");
		m_door_L:setMotor(1.55, 20000);

		-- local m_door_R = logicManager:getHingeJoint("escene_1_door_R");
		m_door_R:setMotor(-1.55, 20000);

	end

	
	function onSwitchReleased_escene_1_push_int(who)

		print(tostring(who) .. "Interruptor liberado escene_1_door_switch");
		-- local m_door_L = logicManager:getHingeJoint("escene_1_door_L");
		m_door_L:setMotor(-1.55, 20000);

		-- local m_door_R = logicManager:getHingeJoint("escene_1_door_R");
		m_door_R:setMotor(1.55, 20000);

	end

	-- PUZZLE MACHACABUELAS

	function onTimerEnd_escene_1_grandma_crasher01Restore()
		local crasher = logicManager:getPrismaticJoint("escene_1_grandmacrasher_01")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_escene_1_grandmacrasher_01_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("escene_1_grandmacrasher_01")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("escene_1_grandma_crasher01Restore", 3)
	end

	
	function onTimerEnd_escene_1_grandma_crasher02Restore()
		local crasher = logicManager:getPrismaticJoint("escene_1_grandmacrasher_02")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_escene_1_grandmacrasher_02_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("escene_1_grandmacrasher_02")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("escene_1_grandma_crasher02Restore", 3)
	end


end

---
----------------------------------------------------------
function onTriggerEnter_trigger_goto_escene_2(who)
	print(tostring(who) .. " ha entrado en el trigger de cambio a escena_2");
	logicManager:loadScene("data/scenes/escene_2.xml");
end
----------------------------------------------------------
------- Escene 2 ------- Escene 2 ------- Escene 2 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_2()

	function onSwitchPressed_escene_2_int_push_false_way(who)

		print(tostring(who) .. " Interruptor escene_2_int_push_false_way pressed");
		local escene_2_int_push_false_way_platform = logicManager:getObject("escene_2_int_push_false_way_platform");
		escene_2_int_push_false_way_platform:move(Vector(86.0, 45, 8.0), 6);

	end

	function onSwitchReleased_escene_2_int_push_false_way(who)

		print(tostring(who) .. " Interruptor escene_2_int_push_false_way released");
		local escene_2_int_push_false_way_platform = logicManager:getObject("escene_2_int_push_false_way_platform");
		escene_2_int_push_false_way_platform:move(Vector(86.0, 48.8562, 8.0), 1);

	end

end


---
----------------------------------------------------------
function onTriggerEnter_trigger_goto_escene_3(who)
	print(tostring(who) .. " ha entrado en el trigger de cambio a escena_3");
	logicManager:loadScene("data/scenes/escene_3.xml");
end
----------------------------------------------------------
------- Escene 3 ------- Escene 3 ------- Escene 3 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_3()

	--- Sala Presentacion Abuelas---

	function onSwitchPressed_escene_3_int_pull_grandma(who)

		print(tostring(who) .. " Interruptor escene_3_int_pull_grandma pressed");
		local platform = logicManager:getObject("escene_3_int_pull_grandma_platform");
		platform:move(Vector(-2.07411, 1.0, 19.8556), 6);

	end

	function onSwitchReleased_escene_3_int_pull_grandma(who)

		print(tostring(who) .. " Interruptor escene_3_int_pull_grandma released");
		local platform = logicManager:getObject("escene_3_int_pull_grandma_platform");
		platform:move(Vector(-2.07411, -6.06088, 19.8556), 6);

	end


	--- Sala Presentacion Agua ---

	function onTriggerExit_escene_3_trigger_pipedown(who)
		print(tostring(who) .. " ha dejado en el trigger de cambio a escena_3");
	
		if who == "escene_3_pipedown_top" then
			logicManager:changeWaterLevel(-1,0.25);
		end
	
	end

	--- Sala Puzle Agua ---

	function onTriggerExit_escene_3_trigger_pipeout(who)
		print(tostring(who) .. " ha dejado en el trigger de cambio a escena_3");
	
		if who == "escene_3_pipeout_top" then
			logicManager:changeWaterLevel(-11.1,0.25);
		end
	
	end

	function onSwitchPressed_escene_3_int_push_final_door(who)

		print(tostring(who) .. " Interruptor escene_3_int_push_final_door pressed");
		local platform = logicManager:getObject("escene_3_final_door");
		platform:move(Vector(-21.466, 5.55807, 113.98), 6);

	end

	function onSwitchReleased_escene_3_int_push_final_door(who)

		print(tostring(who) .. " Interruptor escene_3_int_push_final_doora released");
		local platform = logicManager:getObject("escene_3_final_door");
		platform:move(Vector(-21.466, 1.55807, 113.98), 6);

	end

end

----
----------------------------------------------------------
function onTriggerEnter_trigger_goto_escene_4(who)
	print(tostring(who) .. " ha entrado en el trigger de cambio a escena_4");
	logicManager:loadScene("data/scenes/escene_4.xml");
end
----------------------------------------------------------
------- Escene 4 ------- Escene 4 ------- Escene 4 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_4()


	function onSwitchPressed_interruptor_pulsar_plataforma_01(who)

		print(tostring(who) .. " Interruptor plataforma_01 pulsado");
		local plataforma_ascendente_01 = logicManager:getObject("plataforma_ascendente_01");
		plataforma_ascendente_01:move(Vector(51.1903, 56, 18.1153), 6);

	end

	function onSwitchReleased_interruptor_pulsar_plataforma_01(who)

		print(tostring(who) .. " Interruptor plataforma_01 relesed");
		local plataforma_ascendente_01 = logicManager:getObject("plataforma_ascendente_01");
		plataforma_ascendente_01:move(Vector(51.1903, -1, 18.1153), 1);

	end

end


----
----------------------------------------------------------
function onTriggerEnter_trigger_goto_escene_5(who)
	print(tostring(who) .. " ha entrado en el trigger de cambio a escena_5");
	logicManager:loadScene("data/scenes/escene_5.xml");
end


----------------------------------------------------------
------- Escene 5 ------- Escene 5 ------- Escene 5 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_5()

	local escene_5_clockround = false;

	local m_gear_r = logicManager:getHingeJoint("escene_5_main_gear_part_R");
	local m_gear_l = logicManager:getHingeJoint("escene_5_main_gear_part_L");

	m_gear_r:setMotor(-1, 10000);
	m_gear_l:setMotor(1, 10000);

	local crasher1 = logicManager:getPrismaticJoint("grandma_crasher01")
	crasher1:setLinearLimit(0.1, 10000000, 10000000)
	local crasher2 = logicManager:getPrismaticJoint("grandma_crasher02")
	crasher2:setLinearLimit(0.1, 10000000, 10000000)
	local crasher3 = logicManager:getPrismaticJoint("grandma_crasher03")
	crasher3:setLinearLimit(0.1, 10000000, 10000000)



	-- PUZLE MACHACABUELAS

	function onTimerEnd_grandma_crasher01Restore()
		local crasher = logicManager:getPrismaticJoint("grandma_crasher01")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_grandma_crasher01_Trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("grandma_crasher01")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("grandma_crasher01Restore", 3)
	end

	
	function onTimerEnd_grandma_crasher02Restore()
		local crasher = logicManager:getPrismaticJoint("grandma_crasher02")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_grandma_crasher02_Trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("grandma_crasher02")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("grandma_crasher02Restore", 3)
	end


	function onTimerEnd_grandma_crasher03Restore()
		local crasher = logicManager:getPrismaticJoint("grandma_crasher03")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_grandma_crasher03_Trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("grandma_crasher03")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("grandma_crasher03Restore", 3)
	end


	-- PUZZLE ATREZZO

	function onTriggerEnter_clock_round_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger clock_round_trigger");
		if who == "escene_5_minutero" then	
			
			escene_5_clockround = true;

		end
	end

	function onTriggerEnter_escene_5_clock_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger escene_5_clock_trigger");
		if who == "escene_5_minutero" then	
			if escene_5_clockround then

				print("Clock Blocked!");
				m_minutero = logicManager:getHingeJoint("escene_5_minutero");
				m_minutero:setLimit(10);
		
				m_atrezzo01 = logicManager:getHingeJoint("atrezzo_plataforma01");
				m_atrezzo01:setMotor(10, 100000);
				m_atrezzo02 = logicManager:getHingeJoint("atrezzo_plataforma02");
				m_atrezzo02:setMotor(10, 100000);
				m_atrezzo03 = logicManager:getHingeJoint("atrezzo_plataforma03");
				m_atrezzo03:setMotor(10, 1000);

				escene_5_clockround = false;
			end
		end
	end


	function onSwitchPressed_escene_5_clock_switch(who)

		print(tostring(who) .. "Interruptor pulsado escene_5_clock_switch");
		m_minutero = logicManager:getHingeJoint("escene_5_minutero");
		m_minutero:setLimit(0);
		m_minutero:setMotor(-1.55, 20000);

		m_atrezzo01 = logicManager:getHingeJoint("atrezzo_plataforma01");
		m_atrezzo01:setMotor(-1000, 100000);
		m_atrezzo02 = logicManager:getHingeJoint("atrezzo_plataforma02");
		m_atrezzo02:setMotor(-1000, 100000);
		m_atrezzo03 = logicManager:getHingeJoint("atrezzo_plataforma03");
		m_atrezzo03:setMotor(-1000, 100000);

		m_puerta = logicManager:getHingeJoint("puertaIzda");
		m_puerta:setMotor(-1, 1000);
	end

end




-------------------------- MS2 --------------------------
					-- WATER ROOM --

function onSwitchPressed_InterruptorTirarAgua(who)
	print(tostring(who) .. "Interruptor pulsado");
	logicManager:changeWaterLevel(-1,0.25);

	--local m_platform = logicManager:getMovingPlatform("plataforma_madera");
	--m_platform:start(0.001);

end

function onSwitchReleased_InterruptorTirarAgua(who)
	print(tostring(who) .. "Interruptor pulsado");
	--logicManager:changeWaterLevel(-3.58394,0.015);

	--local m_platform = logicManager:getMovingPlatform("plataforma_madera");
	--m_platform:start(0.001);

end


					-- INTERMEDIATE ROOM --

function onSwitchPressed_PushIntIntermediate(who)
	print(tostring(who) .. "Interruptor pulsado");
	m_minutero = logicManager:getHingeJoint("armarioGatingClock");
	m_minutero:setLimit(0);
	m_minutero:setMotor(-2, 60000);
end


					-- CLOCK ROOM --



-- Engranajes giratorios

function ActiveGears()
	
	local m_gear_1 = logicManager:getHingeJoint("engraje1");
	local m_gear_2 = logicManager:getHingeJoint("eje1");
	local m_gear_3 = logicManager:getHingeJoint("eje5");

	m_gear_1:setMotor(-1, 1000);
	m_gear_2:setMotor(-0.6, 1000);
	m_gear_3:setMotor(0.2, 1000);

end



					-- VICTORY --

function onTriggerEnter_Trigger_Victory(who)
	if who == "Player" then
		print(tostring(who) .. " VICTORIAAAA!!!");
		logicManager:pushPlayerLegsState("fbp_Victory");
	end
end


                     -- DEAD --

function onPlayerDead()
	local player = logicManager:getBot("Player");
	player:teleport(5.008, 0.479939, 47.0041);
end



