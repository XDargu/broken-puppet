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

function onSceneLoad_my_file()
--logicManager:loadScene("data/scenes/scene_1.xml");
	--logicManager:loadScene("data/scenes/scene_3_iluminada_ps.xml");
	--logicManager:setBands(true)

	player = logicManager:getBot("Player");
	initPos = player:getPos();

	logicManager:playSubtitles("NARR002");
	logicManager:setTimer("Subs001", 5);

	function onTriggerEnter_PitTrigger(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then
			--logicManager:pushPlayerLegsState("fbp_WakeUp");
			logicManager:pushPlayerLegsState("fbp_WakeUpTeleport");
			player:teleportToPos(initPos);
		end

	end

	function onTimerEnd_Subs001()
		logicManager:playSubtitles("NARR001");
	end

	-- ASCENSOOOOOOOOOR DE PRUEBA

	ascensor_usado = false

	function onTriggerEnter_escena1_ascensor_fin(who)
		logicManager:pushPlayerLegsState("fbp_Idle");
	end

	function onTriggerEnter_escena1_ascensor(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then

			if ascensor_usado == false then
				ascensor_usado = true
				print(tostring(who) .. "Player en el trigger");
				local scene1_elevator = logicManager:getObject("ascensor_desvan_cabina");
				local platform_pos = scene1_elevator:getPos();
				print(platform_pos);
				logicManager:pushPlayerLegsState("fbp_IdleElevator");
				scene1_elevator:move(Vector(platform_pos.x, (platform_pos.y + 24), platform_pos.z), 5);
			end

		end

	end

	function onTriggerEnter_trigger_asc_crematorio_fin(who)
		logicManager:pushPlayerLegsState("fbp_Idle");
	end

	function onTriggerEnter_trigger_asc_crematorio(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then

			if ascensor_usado == false then
				ascensor_usado = true
				print(tostring(who) .. "Player en el trigger");
				local scene1_elevator = logicManager:getObject("ascensor_crematorio_cabina");
				local platform_pos = scene1_elevator:getPos();
				print(platform_pos);
				logicManager:pushPlayerLegsState("fbp_IdleElevator");
				scene1_elevator:move(Vector(platform_pos.x, (platform_pos.y + 23), platform_pos.z), 5);
			end

		end

	end

end

initPos = 0;


----------------------------------------------------------
--- scene my_file --- scene my_file --- scene my_file ----
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_test_dificultad()

	player = logicManager:getBot("Player");
	initPos = player:getPos();

	local fallingDoor = logicManager:getPrismaticJoint("puerta_descendente");
	fallingDoor:setLinearLimit(0.1, 10000000, 10000000);

	function onSwitchPressed_interruptor_puerta_descendente(who)

		print(tostring(who) .. " Interruptor interruptor_puerta_descendente");
		fallingDoor:setLinearLimit(100, 1, 1);

	end

	function onSwitchReleased_interruptor_puerta_descendente(who)
		print(tostring(who) .. " Interruptor interruptor_puerta_descendente relesed");
		fallingDoor:setLinearLimit(0.1, 10000000, 10000000);
	end

end



----------------------------------------------------------
-- scene_boss --- scene scene_boss --- scene scene_boss --
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_scene_boss()

	player = logicManager:getBot("Player");
	initPos = player:getPos();
	
	function onTriggerEnter_PitTrigger(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then
			--logicManager:pushPlayerLegsState("fbp_WakeUp");
			logicManager:pushPlayerLegsState("fbp_WakeUpTeleport");
			player:teleportToPos(initPos);
		end

	end

end


-------------------------- MS3 ---------------------------

----------------------------------------------------------
------- scene 1 ------- scene 1 ------- scene 1 ----------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_scene_1_noenemy()
	onSceneLoad_scene_1()
end

function onSceneLoad_scene_1()
--logicManager:loadScene("data/scenes/test_dificultad.xml");
	player = logicManager:getBot("Player");
	initPos = player:getPos();

	local crasher1 = logicManager:getPrismaticJoint("scene_1_grandmacrasher_01");
	crasher1:setLinearLimit(0.1, 10000000, 10000000);
	local crasher2 = logicManager:getPrismaticJoint("scene_1_grandmacrasher_02");
	crasher2:setLinearLimit(0.1, 10000000, 10000000);

	local m_door_L = logicManager:getHingeJoint("scene_1_door_L");
	m_door_L:setMotor(-1.55, 2000000);

	local m_door_R = logicManager:getHingeJoint("scene_1_door_R");
	m_door_R:setMotor( 1.55, 2000000);

		-- PUZZLE ATREZZO

	function onTimerEnd_scene_1_door_stop()
		m_door_L:setMotor(0, 0);
		m_door_R:setMotor(0, 0);
		print("Timer ends")
	end

	function onSwitchPressed_scene_1_push_int(who)

		print(tostring(who) .. "Interruptor pulsado scene_1_door_switch");
		-- local m_door_L = logicManager:getHingeJoint("scene_1_door_L");
		m_door_L:setMotor(1.55, 2000000);

		-- local m_door_R = logicManager:getHingeJoint("scene_1_door_R");
		m_door_R:setMotor(-1.55, 2000000);

		logicManager:setTimer("scene_1_door_stop", 0.1)

	end

	function onTriggerEnter_scene1_elevator_trigger(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then

			print(tostring(who) .. "Player en el trigger");
			local scene1_elevator = logicManager:getObject("scene1_elevator");
			local platform_pos = scene1_elevator:getPos();
			print(platform_pos);
			scene1_elevator:move(Vector(platform_pos.x, (platform_pos.y + 24), platform_pos.z), 5);

		end

	end
	
	function onSwitchReleased_scene_1_push_int(who)

		print(tostring(who) .. "Interruptor liberado scene_1_door_switch");
		-- local m_door_L = logicManager:getHingeJoint("scene_1_door_L");
		m_door_L:setMotor(-1.55, 2000000);

		-- local m_door_R = logicManager:getHingeJoint("scene_1_door_R");
		m_door_R:setMotor( 1.55, 2000000);

	end


	-- PUZZLE MACHACABUELAS

	function onTimerEnd_scene_1_grandma_crasher01Restore()
		local crasher = logicManager:getPrismaticJoint("scene_1_grandmacrasher_01")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_scene_1_grandmacrasher_01_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("scene_1_grandmacrasher_01")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("scene_1_grandma_crasher01Restore", 3)
	end

	
	function onTimerEnd_scene_1_grandma_crasher02Restore()
		local crasher = logicManager:getPrismaticJoint("scene_1_grandmacrasher_02")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_scene_1_grandmacrasher_02_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("scene_1_grandmacrasher_02")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("scene_1_grandma_crasher02Restore", 3)
	end


end

---
----------------------------------------------------------
function onTriggerEnter_trigger_goto_scene_2(who)
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_2");
		logicManager:loadScene("data/scenes/scene_2.xml");
	end
end
----------------------------------------------------------
------- scene 2 ------- scene 2 ------- scene 2 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_scene_2()

	player = logicManager:getBot("Player");
	initPos = player:getPos();

	local scene_2_int_push_false_way_platform = logicManager:getObject("scene_2_int_push_false_way_platform");
	local platform_pos = scene_2_int_push_false_way_platform:getPos();

	function onSwitchPressed_scene_2_int_push_false_way(who)
		print(tostring(who) .. " Interruptor scene_2_int_push_false_way pressed");
		scene_2_int_push_false_way_platform:move(Vector(platform_pos.x, (platform_pos.y - 4), platform_pos.z), 1);
	end

	function onSwitchReleased_scene_2_int_push_false_way(who)
		print(tostring(who) .. " Interruptor scene_2_int_push_false_way released");
		scene_2_int_push_false_way_platform:move(Vector(platform_pos.x, platform_pos.y, platform_pos.z), 1);
	end

	function onTriggerEnter_scene_2_lightsout_trigger(who)
		if who == "Player" then	
			print(tostring(who) .. " ha entrado en el trigger de cambio de ambient light");
			logicManager:changeAmbientLight(0.5,0.5,0.5,0.95);
		end
	end		

end


---
----------------------------------------------------------
function onTriggerEnter_trigger_goto_scene_3(who)
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_3");
		logicManager:loadScene("data/scenes/scene_3.xml");
	end
end
----------------------------------------------------------
------- scene 3 ------- scene 3 ------- scene 3 ----------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_scene_3_noenemy()
	onSceneLoad_scene_3()
end

function onSceneLoad_scene_3()

	player = logicManager:getBot("Player");
	initPos = player:getPos();	

	local crasher1 = logicManager:getPrismaticJoint("scene_3_grandmacrasher_01");
	crasher1:setLinearLimit(0.1, 10000000, 10000000);
	--- Sala Presentacion Abuelas---

	function onSwitchPressed_scene_3_int_pull_grandma(who)

		print(tostring(who) .. " Interruptor scene_3_int_pull_grandma pressed");
		local platform = logicManager:getObject("scene_3_int_pull_grandma_platform");
		platform:move(Vector(-2.07411, 0, 19.8556), 6);

	end

	function onSwitchReleased_scene_3_int_pull_grandma(who)

		print(tostring(who) .. " Interruptor scene_3_int_pull_grandma released");
		local platform = logicManager:getObject("scene_3_int_pull_grandma_platform");
		platform:move(Vector(-2.07411, -7, 19.8556), 6);

	end

	function onTimerEnd_scene_3_grandma_crasher01Restore()
		local crasher = logicManager:getPrismaticJoint("scene_3_grandmacrasher_01")
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_scene_3_grandmacrasher_01_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger")
		local crasher = logicManager:getPrismaticJoint("scene_3_grandmacrasher_01")
		crasher:setLinearLimit(1000, 0, 0)

		logicManager:setTimer("scene_3_grandma_crasher01Restore", 3)
	end


	--- Sala Presentacion Agua ---

	function onTriggerExit_scene_3_trigger_pipedown(who)
		print(tostring(who) .. " ha dejado en el trigger de pipedown");
				
		local top = logicManager:getObject("scene_3_pipedown_top");
		local top_pos = top:getPos();		

		if who == "scene_3_pipedown_top" then
			local m_door = logicManager:getHingeJoint("scene_3_puerta_gating_1");
			m_door:setMotor(1.55, 20000);
			logicManager:changeWaterLevel(-1.1,0.25);
			--logicManager:createParticleGroup("ps_jet", top_pos, Quaternion( 0.5, 0.5, -0.5, 0.5));

			logicManager:createParticleGroup("ps_jet", Vector(top_pos.x + 1, top_pos.y + 1, top_pos.z), Quaternion( 0.7071067811865476, 0, -0.7071067811865476, 0));
		end
	
	end

	--- Sala Puzle Agua ---

	function onTriggerExit_scene_3_trigger_pipeout(who)
		print(tostring(who) .. " ha dejado en el trigger de pipeout");
	
		if who == "scene_3_pipeout_top" then
			logicManager:changeWaterLevel(-11.1,0.25);
		end
	
	end

	function onSwitchPressed_scene_3_int_push_final_door(who)

		print(tostring(who) .. " Interruptor scene_3_int_push_final_door pressed");
		local platform = logicManager:getObject("scene_3_final_door");
		platform:move(Vector(-21.466, 5.55807, 113.98), 6);

	end

	function onSwitchReleased_scene_3_int_push_final_door(who)

		print(tostring(who) .. " Interruptor scene_3_int_push_final_doora released");
		local platform = logicManager:getObject("scene_3_final_door");
		platform:move(Vector(-21.466, 1.55807, 113.98), 6);

	end

end


----
----------------------------------------------------------
function onTriggerEnter_trigger_goto_scene_4(who)
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_4");
		logicManager:loadScene("data/scenes/scene_4.xml");
	end
end
----------------------------------------------------------
------- scene 4 ------- scene 4 ------- scene 4 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_scene_4()

	player = logicManager:getBot("Player");
	initPos = player:getPos();

	function onSwitchPressed_interruptor_pulsar_plataforma_01(who)

		print(tostring(who) .. " Interruptor plataforma_01 pulsado");
		local plataforma_ascendente_01 = logicManager:getObject("plataforma_ascendente_01");
		plataforma_ascendente_01:move(Vector(50.3461, 9.77687, 27.29), 3);

	end

	function onSwitchReleased_interruptor_pulsar_plataforma_01(who)

		print(tostring(who) .. " Interruptor plataforma_01 relesed");
		local plataforma_ascendente_01 = logicManager:getObject("plataforma_ascendente_01");
		plataforma_ascendente_01:move(Vector(50.3461, -1.77687, 27.29), 1);

	end


	function onSwitchPressed_interruptor_pulsar_plataforma_02(who)

		print(tostring(who) .. " Interruptor plataforma_02 pulsado");
		local plataforma_ascendente_02 = logicManager:getObject("plataforma_ascendente_02");
		plataforma_ascendente_02:move(Vector(39.4995, 20.4613, 30.4338), 3);

	end

	function onSwitchReleased_interruptor_pulsar_plataforma_02(who)

		print(tostring(who) .. " Interruptor plataforma_02 relesed");
		local plataforma_ascendente_02 = logicManager:getObject("plataforma_ascendente_02");
		plataforma_ascendente_02:move(Vector(39.4995, 10.4613, 30.4338), 1);

	end



	local scene_5_clockround = false;

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
		if who == "scene_5_minutero" then	
			
			scene_5_clockround = true;

		end
	end

	function onTriggerEnter_scene_5_clock_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger scene_5_clock_trigger");
		if who == "scene_5_minutero" then	
			if scene_5_clockround then

				print("Clock Blocked!");
				m_minutero = logicManager:getHingeJoint("scene_5_minutero");
				m_minutero:setLimit(10);
		
				m_atrezzo01 = logicManager:getHingeJoint("atrezzo_plataforma01");
				m_atrezzo01:setMotor(10, 100000);
				m_atrezzo02 = logicManager:getHingeJoint("atrezzo_plataforma02");
				m_atrezzo02:setMotor(10, 100000);
				m_atrezzo03 = logicManager:getHingeJoint("atrezzo_plataforma03");
				m_atrezzo03:setMotor(10, 1000);

				scene_5_clockround = false;
			end
		end
	end


	function onSwitchPressed_scene_5_clock_switch(who)

		print(tostring(who) .. "Interruptor pulsado scene_5_clock_switch");
		m_minutero = logicManager:getHingeJoint("scene_5_minutero");
		m_minutero:setLimit(0);
		m_minutero:setMotor(-1.55, 2000);

		m_atrezzo01 = logicManager:getHingeJoint("atrezzo_plataforma01");
		m_atrezzo01:setMotor(-1000, 100000);
		m_atrezzo02 = logicManager:getHingeJoint("atrezzo_plataforma02");
		m_atrezzo02:setMotor(-1000, 100000);
		m_atrezzo03 = logicManager:getHingeJoint("atrezzo_plataforma03");
		m_atrezzo03:setMotor(-1000, 100000);

	end

	function onTriggerEnter_scene4_elevator_trigger(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then

			print(tostring(who) .. "Player en el trigger");
			local scene4_elevator = logicManager:getObject("scene4_elevator");
			local platform_pos = scene4_elevator:getPos();
			print(platform_pos);
			scene4_elevator:move(Vector(platform_pos.x, (platform_pos.y + 32), platform_pos.z), 3);

		end

	end

end


----
----------------------------------------------------------
function onTriggerEnter_trigger_goto_scene_5(who)
	if who == "Player" then	
		print(tostring(who) .. " ha entrado en el trigger de cambio a escena_5");
		logicManager:loadScene("data/scenes/scene_5.xml");
	end
end


----------------------------------------------------------
------- scene 5 ------- scene 5 ------- scene 5 -------
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_scene_5_noenemy()
	onSceneLoad_scene_5()
end

function onSceneLoad_scene_5()

	player = logicManager:getBot("Player");
	initPos = player:getPos();

	local scene_5_clockround = false;

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
		if who == "scene_5_minutero" then	
			
			scene_5_clockround = true;

		end
	end

	function onTriggerEnter_scene_5_clock_trigger(who)
		print(tostring(who) .. " ha entrado en el trigger scene_5_clock_trigger");
		if who == "scene_5_minutero" then	
			if scene_5_clockround then

				print("Clock Blocked!");
				m_minutero = logicManager:getHingeJoint("scene_5_minutero");
				m_minutero:setLimit(10);
		
				m_atrezzo01 = logicManager:getHingeJoint("atrezzo_plataforma01");
				m_atrezzo01:setMotor(10, 100000);
				m_atrezzo02 = logicManager:getHingeJoint("atrezzo_plataforma02");
				m_atrezzo02:setMotor(10, 100000);
				m_atrezzo03 = logicManager:getHingeJoint("atrezzo_plataforma03");
				m_atrezzo03:setMotor(10, 1000);

				scene_5_clockround = false;
			end
		end
	end


	function onSwitchPressed_scene_5_clock_switch(who)

		print(tostring(who) .. "Interruptor pulsado scene_5_clock_switch");
		m_minutero = logicManager:getHingeJoint("scene_5_minutero");
		m_minutero:setLimit(0);
		m_minutero:setMotor(-1.55, 2000);

		m_atrezzo01 = logicManager:getHingeJoint("atrezzo_plataforma01");
		m_atrezzo01:setMotor(-1000, 100000);
		m_atrezzo02 = logicManager:getHingeJoint("atrezzo_plataforma02");
		m_atrezzo02:setMotor(-1000, 100000);
		m_atrezzo03 = logicManager:getHingeJoint("atrezzo_plataforma03");
		m_atrezzo03:setMotor(-1000, 100000);

	end

end



					-- VICTORY --

function onTriggerEnter_Trigger_Victory(who)
	if who == "Player" then
		print(tostring(who) .. " VICTORIAAAA!!!");
		--logicManager:pushPlayerLegsState("fbp_Victory");
	end
end


                     -- DEAD --

function onPlayerDead()
	local player = logicManager:getBot("Player");
	player:teleportToPos(initPos);
end



