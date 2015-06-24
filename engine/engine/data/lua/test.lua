SLB.using(SLB)

-- Funciones útiles de LUA
function print(s)
	logicManager:print(tostring(s));
end
function error(s)
	logicManager:print("ERROR: " .. tostring(s));
end

_print = print
clearCoroutines( )


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
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_2");
		logicManager:loadScene("data/scenes/escene_2.xml");
	end
end
----------------------------------------------------------
------- Escene 2 ------- Escene 2 ------- Escene 2 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_2()

	function onSwitchPressed_escene_2_int_push_false_way(who)

		print(tostring(who) .. " Interruptor escene_2_int_push_false_way pressed");
		local escene_2_int_push_false_way_platform = logicManager:getObject("escene_2_int_push_false_way_platform");
		escene_2_int_push_false_way_platform:move(Vector(86.0, 4.88892, 8.0), 1);

	end

	function onSwitchReleased_escene_2_int_push_false_way(who)

		print(tostring(who) .. " Interruptor escene_2_int_push_false_way released");
		local escene_2_int_push_false_way_platform = logicManager:getObject("escene_2_int_push_false_way_platform");
		escene_2_int_push_false_way_platform:move(Vector(86.0, 8.88892, 8.0), 1);

	end

end


---
----------------------------------------------------------
function onTriggerEnter_trigger_goto_escene_3(who)
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_3");
		logicManager:loadScene("data/scenes/escene_3.xml");
	end
end
----------------------------------------------------------
------- Escene 3 ------- Escene 3 ------- Escene 3 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_3()

	local crasher1 = logicManager:getPrismaticJoint("escene_3_grandmacrasher_01");
	crasher1:setLinearLimit(0.1, 10000000, 10000000);
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

	function onTimerEnd_escene_3_grandma_crasher01Restore()
		local crasher = logicManager:getPrismaticJoint("escene_3_grandmacrasher_01")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_escene_3_grandmacrasher_01_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("escene_3_grandmacrasher_01")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("escene_3_grandma_crasher01Restore", 3)
	end


	--- Sala Presentacion Agua ---

	function onTriggerExit_escene_3_trigger_pipedown(who)
		print(tostring(who) .. " ha dejado en el trigger de pipedown");
	
		if who == "escene_3_pipedown_top" then
			local m_door = logicManager:getHingeJoint("escene_3_puerta_gating_1");
			m_door:setMotor(1.55, 20000);
			logicManager:changeWaterLevel(-1,0.25);
		end
	
	end

	--- Sala Puzle Agua ---

	function onTriggerExit_escene_3_trigger_pipeout(who)
		print(tostring(who) .. " ha dejado en el trigger de pipeout");
	
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
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_4");
		logicManager:loadScene("data/scenes/escene_4.xml");
	end
end
----------------------------------------------------------
------- Escene 4 ------- Escene 4 ------- Escene 4 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_escene_4()


	function onSwitchPressed_interruptor_pulsar_plataforma_01(who)

		print(tostring(who) .. " Interruptor plataforma_01 pulsado");
		local plataforma_ascendente_01 = logicManager:getObject("plataforma_ascendente_01");
		plataforma_ascendente_01:move(Vector(50.3461, 9.77687, 27.29), 4);

	end

	function onSwitchReleased_interruptor_pulsar_plataforma_01(who)

		print(tostring(who) .. " Interruptor plataforma_01 relesed");
		local plataforma_ascendente_01 = logicManager:getObject("plataforma_ascendente_01");
		plataforma_ascendente_01:move(Vector(50.3461, -1.77687, 27.29), 1.2);

	end


	function onSwitchPressed_interruptor_pulsar_plataforma_02(who)

		print(tostring(who) .. " Interruptor plataforma_02 pulsado");
		local plataforma_ascendente_02 = logicManager:getObject("plataforma_ascendente_02");
		plataforma_ascendente_02:move(Vector(39.4995, 20.4613, 30.4338), 4);

	end

	function onSwitchReleased_interruptor_pulsar_plataforma_02(who)

		print(tostring(who) .. " Interruptor plataforma_02 relesed");
		local plataforma_ascendente_02 = logicManager:getObject("plataforma_ascendente_02");
		plataforma_ascendente_02:move(Vector(39.4995, 10.4613, 30.4338), 1.2);

	end

end


----
----------------------------------------------------------
function onTriggerEnter_trigger_goto_escene_5(who)
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_5");
		logicManager:loadScene("data/scenes/escene_5.xml");
	end
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



