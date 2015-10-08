SLB.using(SLB)

-- Funciones útiles de LUA
function print(s)
	logicManager:print(tostring(s));
end
function error(s)
	logicManager:print("ERROR: " .. tostring(s));
end



_print = print
clearCoroutines()

----------------------------------------------------------
-- scene_boss --- scene scene_boss --- scene scene_boss --
----------------------------------------------------------
----------------------------------------------------------

function onSceneLoad_my_file()
	onSceneLoad_scene_final_boss()
	onSceneLoad_scene_4()
	onSceneLoad_scene_3()
	onSceneLoad_scene_2()
	onSceneLoad_scene_1()	
end

function onSceneLoad_scene_final_boss()

	player = logicManager:getBot("Player");
	initPos = player:getPosition();
	respawnPos = player:getPosition();
	
	function onTriggerEnter_PitTrigger(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then
			--logicManager:pushPlayerLegsState("fbp_WakeUp");
			logicManager:pushPlayerLegsState("fbp_WakeUpTeleport");
			player:teleportToPos(respawnPos);
		end

	end

	function onTriggerFirstEnter_scb_trigger_bandas(who)
		logicManager:setBands(true)
		logicManager:setMediumShotActive(true)

		-- Bajar luz ambiental
		logicManager:changeAmbientLight(0.7, 0.7, 0.7, 0.1);
	end

	function onTimerEnd_boss_init_animation()
		logicManager:setCanMove(true)
		logicManager:setCanThrow(true)
		logicManager:setCanPull(true)
		logicManager:setCanCancel(true)
		logicManager:setCanTense(true)
		logicManager:setBands(false)
		logicManager:releaseCameraLock()
		logicManager:resetPlayerCamera()

		respawnPos = player:getPosition();
	end

	function onTriggerFirstEnter_scb_trigger_block_control(who)
		local boss = logicManager:getObject("Boss")	
		boss:riseUpBoss()

		logicManager:setCanMove(false)
		logicManager:setCanThrow(false)
		logicManager:setCanPull(false)
		logicManager:setCanCancel(false)
		logicManager:setCanTense(false)
		logicManager:pushPlayerLegsState("fbp_Idle");
		local boss = logicManager:getBot("Boss")
		logicManager:lockCameraOnPosition(Vector(0, 8, 0))
		logicManager:setTimer("boss_init_animation", 21)
		
		startCoroutine("bossShake", bossShake)
	end

	function bossShake()
		-- Tiempos
		local first_arm = 2.72
		local second_arm = 6.79
		local get_up = 9.8
		local first_arm_2 = 14.1
		local second_arm_2 = 14.8
		local scream = 20.44

		local elapsed = 0

		logicManager:shakeCamera(0.03)
		waitTime(first_arm)
		elapsed = first_arm

		logicManager:shakeCamera(0.1) -- Primer brazo
		waitTime(0.5)
		elapsed = elapsed + 0.5
		logicManager:shakeCamera(0.03)
		waitTime(second_arm - elapsed)
		elapsed = second_arm

		logicManager:shakeCamera(0.1) -- Segundo brazo
		waitTime(0.5)
		elapsed = elapsed + 0.5
		logicManager:shakeCamera(0.03)
		waitTime(get_up - elapsed)
		elapsed = get_up

		logicManager:shakeCamera(0.12) -- Levantarse
		waitTime(0.5)
		elapsed = elapsed + 0.5
		logicManager:stopShakeCamera()
		waitTime(first_arm_2 - elapsed)
		elapsed = first_arm_2

		logicManager:shakeCamera(0.03) -- Pequeño movimiento del primer brazo
		waitTime(0.5)
		elapsed = elapsed + 0.5
		logicManager:stopShakeCamera()
		waitTime(second_arm_2 - elapsed)
		elapsed = second_arm_2

		logicManager:shakeCamera(0.03) -- Pequeño movimiento del segundo brazo
		waitTime(0.5)
		elapsed = elapsed + 0.5
		logicManager:stopShakeCamera()
		waitTime(scream - elapsed)
		elapsed = scream

		logicManager:shakeCamera(0.15) -- Grito
		waitTime(1)
		logicManager:stopShakeCamera()
	end
end


function onSceneLoad_scene_1()
	player = logicManager:getBot("Player");
	initPos = player:getPosition();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 1 **********

	-- Iluminación inicial
	logicManager:changeAmbientLight(0.6, 0.6, 0.6, 1);

	function onTriggerEnter_sc1_trigger_change_light(who)
		logicManager:changeAmbientLight(1, 1, 1, 0.75);
	end

	-- Machacaabuelas
	local crasher = logicManager:getPrismaticJoint("scene1_grandmacrasher_joint1");
	crasher:setLinearLimit(0.1, 10000000, 10000000);
	
	function onTimerEnd_scene_1_grandma_crasher01Restore()
		crasher:setLinearLimit(0.1, 10000000, 10000000)
		print("Timer ends")
	end


	function onTriggerEnter_scene1_grandmacrasher_trigger1(who)
		print(tostring(who) .. " ha entrado en el trigger")
		crasher:setLinearLimit(1000, 0, 0)

		--local crasher_body = logicManager:getObject("machacabuelas")		
		--crasher_body:applyForce(Vector(0, -10, 0))

		logicManager:setTimer("scene_1_grandma_crasher01Restore", 3)
	end
		

	-- Apagar cable
	local sc1_cable = logicManager:getObject("cable_int_sc1_door")	
	sc1_cable:setEmissive(false)

	-- Cerrar puerta
	local sc1_door = logicManager:getHingeJoint("sc1_hinge_door");
	sc1_door:setLimit(0.1)

	function onSwitchPressed_int_sc1_door(who)
		-- Encender cable
		sc1_cable:setEmissive(true)
		local o_cable = logicManager:getObject("sc1_cable");
		logicManager:playEventAtPosition("LIGHT_ON", o_cable:getPos());

		-- Abrir puerta
		sc1_door:setLimit(90)
		sc1_door:setMotor(-1.55, 2000000);
		local door = logicManager:getObject("sc1_hinge_door");
		logicManager:playEventAtPosition("WOODEN_DOOR", door:getPos());
	end

	function onSwitchReleased_int_sc1_door(who)
		-- Apagar cable
		sc1_cable:setEmissive(false)
		local o_cable = logicManager:getObject("sc1_cable");
		logicManager:playEventAtPosition("LIGHT_OFF", o_cable:getPos());

		-- Cerrar puerta
		sc1_door:setMotor(0, 0);
		sc1_door:setLimit(0.1)
	end

	-- *** Ascensor *** 
	-- *** Interruptor del ascensor *** 
	-- Motor
	local hinge_int_asc = logicManager:getHingeJoint("hinge_ascensor_brazo");
	hinge_int_asc:setMotor(-1.55, 1000);
	
	-- Bloquear puertas	
	local sc1_hinge_asc1 = logicManager:getHingeJoint("sc1_hinge_asc1");
	local sc1_hinge_asc2 = logicManager:getHingeJoint("sc1_hinge_asc2");
	local sc1_hinge_asc3 = logicManager:getHingeJoint("sc1_hinge_asc3");
	local sc1_hinge_asc4 = logicManager:getHingeJoint("sc1_hinge_asc4");

	sc1_hinge_asc1:setLimit(0.1)
	sc1_hinge_asc2:setLimit(0.1)
	sc1_hinge_asc3:setLimit(0.1)
	sc1_hinge_asc4:setLimit(0.1)

	local int_ascensor_pulsado = false

	function onTriggerEnter_sc1_trigger_int_asc(who)
		if (who == "int_ascensor_brazo") then
			if int_ascensor_pulsado == false then
				hinge_int_asc:setMotor(1.55, 1000);
				-- Abrir puertas delanteras
				sc1_hinge_asc1:setLimit(90)
				sc1_hinge_asc2:setLimit(90)

				sc1_hinge_asc1:setMotor(1.55, 3000);
				sc1_hinge_asc2:setMotor(-1.55, 3000);

				int_ascensor_pulsado = true
			end
		end
	end
	
	-- Subida del ascensor

	local ascensor_usado = false

	function onTriggerEnter_escena1_ascensor_fin(who)
		logicManager:setCanMove(true)
		logicManager:setCanThrow(true)
		logicManager:setCanPull(true)
		logicManager:setCanCancel(true)
		logicManager:setCanTense(true)
		logicManager:pushPlayerLegsState("fbp_Idle");
		
		-- Cerrar puertas traseras
		sc1_hinge_asc3:setMotor(0, 0);
		sc1_hinge_asc3:setMotor(0, 0);
		sc1_hinge_asc1:setLimit(0.1)
		sc1_hinge_asc2:setLimit(0.1)

		-- Abrir puertas traseras
		sc1_hinge_asc3:setLimit(90)
		sc1_hinge_asc4:setLimit(90)

		sc1_hinge_asc3:setMotor(-1.55, 3000);
		sc1_hinge_asc4:setMotor(1.55, 3000);
	end

	function onTriggerEnter_escena1_ascensor(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then

			if ascensor_usado == false then
				ascensor_usado = true
				local scene1_elevator = logicManager:getObject("ascensor_desvan_cabina");
				local platform_pos = scene1_elevator:getPos();
				scene1_elevator:move(Vector(platform_pos.x, (platform_pos.y + 23), platform_pos.z), 5);

				logicManager:setCanMove(false)
				logicManager:setCanThrow(false)
				logicManager:setCanPull(false)
				logicManager:setCanCancel(false)
				logicManager:setCanTense(false)
				logicManager:pushPlayerLegsState("fbp_Idle");

				-- Cerrar puertas delanteras
				-- Abrir puertas delanteras
				sc1_hinge_asc1:setLimit(0.01)
				sc1_hinge_asc2:setLimit(0.01)

				sc1_hinge_asc1:setMotor(0,0);
				sc1_hinge_asc2:setMotor(0,0);

				-- Sonido de ascensor
				logicManager:playEvent("ELEVATOR")
			end

		end

	end

	-- Salir de la escena

	function onTriggerEnter_sc_trigger_fin(who)
		logicManager:loadScene("data/scenes/scene_2.xml");
	end

	-- Narrador 

	function onTriggerFirstEnter_scene1_trigger_narr1(who)
		logicManager:playSubtitles("SCENE103");
	end

	function onTriggerFirstEnter_scene1_trigger_narr2(who)
		logicManager:playSubtitles("SCENE104");
	end
	

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 1 ********
end

function onSceneLoad_scene_2()

	player = logicManager:getBot("Player");
	initPos = player:getPosition();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 2 **********

	-- Apagar cable
	local sc2_cable = logicManager:getObject("sc2_cable_plat")	
	sc2_cable:setEmissive(false)

	-- Cadenas
	local sc2_hinge_caida1 = logicManager:getHingeJoint("sc2_hinge_caida1");
	local sc2_hinge_caida2 = logicManager:getHingeJoint("sc2_hinge_caida2");
	local sc2_hinge_caida3 = logicManager:getHingeJoint("sc2_hinge_caida3");
	local sc2_hinge_caida4 = logicManager:getHingeJoint("sc2_hinge_caida4");
	local sc2_hinge_caida5 = logicManager:getHingeJoint("sc2_hinge_caida5");
	local sc2_hinge_caida6 = logicManager:getHingeJoint("sc2_hinge_caida6");
	local sc2_hinge_caida7 = logicManager:getHingeJoint("sc2_hinge_caida7");
	local sc2_hinge_caida8 = logicManager:getHingeJoint("sc2_hinge_caida8");
	
	-- Limitar cadenas al inicio
	sc2_hinge_caida1:setLimit(0.1)
	sc2_hinge_caida2:setLimit(0.1)
	sc2_hinge_caida3:setLimit(0.1)
	sc2_hinge_caida4:setLimit(0.1)
	sc2_hinge_caida5:setLimit(0.1)
	sc2_hinge_caida6:setLimit(0.1)
	sc2_hinge_caida7:setLimit(0.1)
	sc2_hinge_caida8:setLimit(0.1)

	-- Caída
	function onTriggerFirstEnter_trigger_caida(who)
		-- Quitar límites a cadenas
		sc2_hinge_caida1:setLimit(90)
		sc2_hinge_caida2:setLimit(90)
		sc2_hinge_caida3:setLimit(90)
		sc2_hinge_caida4:setLimit(90)
		sc2_hinge_caida5:setLimit(90)
		sc2_hinge_caida6:setLimit(90)
		sc2_hinge_caida7:setLimit(90)
		sc2_hinge_caida8:setLimit(90)

		-- Romper fixed joints
		local sc2_fixed_1 = logicManager:getFixedJoint("sc2_fixed_caida1");
		sc2_fixed_1:breakJoint();

		local sc2_fixed_2 = logicManager:getFixedJoint("sc2_fixed_caida2");
		sc2_fixed_2:breakJoint();

		-- Quitar luces
		logicManager:changeAmbientLight(0.4,0.4,0.4,0.95);
	end

	-- Plataforma elevadora
	local sc2_plataforma_elevadora = logicManager:getObject("sc2_plataforma_elevadora")
	local sc2_plataforma_elevadora_orig = sc2_plataforma_elevadora:getPos()
	sc2_plataforma_elevadora:move(Vector(sc2_plataforma_elevadora_orig.x, sc2_plataforma_elevadora_orig.y + 4.06, sc2_plataforma_elevadora_orig.z), 3);

	function onSwitchPressed_sc2_int_pelev(who)

		print(tostring(who) .. " Interruptor subir plataforma");
		sc2_plataforma_elevadora:move(sc2_plataforma_elevadora_orig, 3);
		sc2_cable:setEmissive(true)

	end

	function onSwitchReleased_sc2_int_pelev(who)

		print(tostring(who) .. " Interruptor bajar plataforma");
		sc2_plataforma_elevadora:move(Vector(sc2_plataforma_elevadora_orig.x, sc2_plataforma_elevadora_orig.y + 4.06, sc2_plataforma_elevadora_orig.z), 3);
		sc2_cable:setEmissive(false)
	end
	
	-- Salir de la escena
	function onTriggerEnter_sc2_trigger_fin(who)
		logicManager:loadScene("data/scenes/scene_3.xml");
	end

	-- Narrador

	function onTriggerFirstEnter_scene2_trigger_narr2(who)
		logicManager:playSubtitles("SCENE202");
	end

	function onTriggerFirstEnter_scene2_trigger_narr3(who)
		logicManager:playSubtitles("SCENE203");
	end

	function onTriggerFirstEnter_scene2_trigger_narr4(who)
		logicManager:setTimer("timerSc2Narr4", 1);
	end

	function onTimerEnd_timerSc2Narr4()
		logicManager:playSubtitles("SCENE204");
	end

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 2 ********
end

function onSceneLoad_scene_3()

	player = logicManager:getBot("Player");
	initPos = player:getPosition();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 3 **********

	-- Subir el nivel del agua
	function onTriggerExit_sc3_trigger_tuber(who)
		print(tostring(who) .. " ha dejado en el trigger de tuber");
	
		if (who == "tapa_agua_puzle") then
			logicManager:changeWaterLevel(7.58, 0.25);
			logicManager:createPrefab("running_water", Vector(-16.88, 11.135, 69.9094), Quaternion( 0.7071067811865476, 0, 0.7071067811865476, 0));
		end
	
	end

	-- Subir el agua si no se ha subido aún
	function onTriggerFirstEnter_sc3_trigger_rise_water(who)
		logicManager:changeWaterLevel(7.58, 0.5);
	end

	-- Bajar el nivel del agua
	function onTriggerExit_sc3_trigger_desag(who)
		print(tostring(who) .. " ha dejado en el trigger de desag");
	
		if who == "tapa_desague_puzle" then
			logicManager:changeWaterLevel(-3.3, 0.25);
		end
	
	end

	-- Plataforma elevadora

	-- Apagar cable
	local sc3_cable_fin = logicManager:getObject("cable_int_sc3_door")	
	sc3_cable_fin:setEmissive(false)
	
	-- Obtener plataforma	 
	local sc3_plataforma_elevadora = logicManager:getObject("sc3_plataforma_cajon")
	local sc3_plataforma_elevadora_orig = sc3_plataforma_elevadora:getPos()

	function onSwitchPressed_sc3_int_final(who)
		sc3_plataforma_elevadora:move(Vector(sc3_plataforma_elevadora_orig.x, sc3_plataforma_elevadora_orig.y + 4, sc3_plataforma_elevadora_orig.z), 3);
		sc3_cable_fin:setEmissive(true)
	end

	function onSwitchReleased_sc3_int_final(who)
		sc3_plataforma_elevadora:move(sc3_plataforma_elevadora_orig, 3);
		sc3_cable_fin:setEmissive(false)
	end
	
	-- Plataforma elevada
	-- Apagar cable
	local sc3_cable_inicio = logicManager:getObject("cable_int_sc3_platform")	
	sc3_cable_inicio:setEmissive(false)

	local sc3_plataform = logicManager:getPrismaticJoint("sc3_int_prismatic_inicio");
	sc3_plataform:setLinearLimit(1000, 10000000, 10000000)

	function onSwitchPressed_sc3_int_inicio(who)
		sc3_plataform:setLinearLimit(0.1, 10000000, 10000000)
		sc3_cable_inicio:setEmissive(true)
	end

	function onSwitchReleased_sc3_int_inicio(who)
		sc3_plataform:setLinearLimit(1000, 10000000, 10000000)
		sc3_cable_inicio:setEmissive(false)
	end

	-- Cerrar puerta
	function onTriggerEnter_sc3_trigger_close_door(who)
		
	end	

	-- Salir de la escena
	function onTriggerEnter_sc3_trigger_fin(who)
		logicManager:loadScene("data/scenes/scene_4.xml");
	end

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 3 ********
end


function onSceneLoad_scene_4()

	player = logicManager:getBot("Player");
	initPos = player:getPosition();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 4 **********


	-- *** Ascensor *** 
	-- *** Interruptor del ascensor *** 
	-- Motor
	local sc4_hinge_int_asc = logicManager:getHingeJoint("sc4_hinge_ascensor_brazo");
	sc4_hinge_int_asc:setMotor(-1.55, 1000);
	
	-- Bloquear puertas	
	local sc4_hinge_asc_1 = logicManager:getHingeJoint("sc4_hinge_asc_1");
	local sc4_hinge_asc_2 = logicManager:getHingeJoint("sc4_hinge_asc_2");
	local sc4_hinge_asc_3 = logicManager:getHingeJoint("sc4_hinge_asc_3");
	local sc4_hinge_asc_4 = logicManager:getHingeJoint("sc4_hinge_asc_4");

	sc4_hinge_asc_1:setLimit(0.1)
	sc4_hinge_asc_2:setLimit(0.1)
	sc4_hinge_asc_3:setLimit(0.1)
	sc4_hinge_asc_4:setLimit(0.1)

	local sc4_int_ascensor_pulsado = false

	function onTriggerEnter_sc4_trigger_int_asc(who)
		if (who == "sc4_int_ascensor_brazo") then
			if sc4_int_ascensor_pulsado == false then
				sc4_hinge_int_asc:setMotor(1.55, 1000);
				-- Abrir puertas delanteras
				sc4_hinge_asc_1:setLimit(90)
				sc4_hinge_asc_2:setLimit(90)

				sc4_hinge_asc_1:setMotor(-1.55, 3000);
				sc4_hinge_asc_2:setMotor(1.55, 3000);
				sc4_int_ascensor_pulsado = true
			end
		end
	end
	
	-- Subida del ascensor

	function onTriggerEnter_trigger_asc_crematorio_fin(who)
		logicManager:setCanMove(true)
		logicManager:setCanThrow(true)
		logicManager:setCanPull(true)
		logicManager:setCanCancel(true)
		logicManager:setCanTense(true)
		logicManager:pushPlayerLegsState("fbp_Idle");
		
		-- Cerrar puertas traseras
		sc4_hinge_asc_1:setMotor(0, 0);
		sc4_hinge_asc_2:setMotor(0, 0);
		sc4_hinge_asc_1:setLimit(0.1)
		sc4_hinge_asc_2:setLimit(0.1)

		-- Abrir puertas traseras
		sc4_hinge_asc_3:setLimit(90)
		sc4_hinge_asc_4:setLimit(90)

		sc4_hinge_asc_3:setMotor(-1.55, 3000);
		sc4_hinge_asc_4:setMotor(1.55, 3000);
	end

	function onTriggerFirstEnter_trigger_asc_crematorio(who)
		print(tostring(who) .. "Entrado en el trigger");
		local scene4_elevator = logicManager:getObject("ascensor_crematorio_cabina");
		local platform_pos = scene4_elevator:getPos();
		scene4_elevator:move(Vector(platform_pos.x, (platform_pos.y + 23), platform_pos.z), 5);

		logicManager:setCanMove(false)
		logicManager:setCanThrow(false)
		logicManager:setCanPull(false)
		logicManager:setCanCancel(false)
		logicManager:setCanTense(false)
		logicManager:pushPlayerLegsState("fbp_Idle");

		-- Cerrar puertas delanteras
		-- Abrir puertas delanteras
		sc4_hinge_asc_1:setLimit(0.01)
		sc4_hinge_asc_2:setLimit(0.01)

		sc4_hinge_asc_1:setMotor(0,0);
		sc4_hinge_asc_2:setMotor(0,0);

		-- Sonido de ascensor
		logicManager:playEvent("ELEVATOR")
	end


	-- Reloj
	-- Apagar cable
	local sc4_cable_reloj = logicManager:getObject("sc4_cable_reloj")	
	sc4_cable_reloj:setEmissive(false)

	-- Mover engranajes de dentro
	sc4_hinge_eje_1 = logicManager:getHingeJoint("sc4_hinge_eje_1");
	sc4_hinge_eje_2 = logicManager:getHingeJoint("sc4_hinge_eje_2");
	sc4_hinge_eje_3 = logicManager:getHingeJoint("sc4_hinge_eje_3");
	sc4_hinge_eje_4 = logicManager:getHingeJoint("sc4_hinge_eje_4");
	sc4_hinge_eje_5 = logicManager:getHingeJoint("sc4_hinge_eje_5");

	sc4_hinge_eje_1:setLimit(0)
	sc4_hinge_eje_2:setLimit(0)
	sc4_hinge_eje_3:setLimit(0)
	sc4_hinge_eje_4:setLimit(0)
	sc4_hinge_eje_5:setLimit(0)

	sc4_hinge_eje_1:setMotor(1.55, 800000)
	sc4_hinge_eje_2:setMotor(-1.55, 800000)
	sc4_hinge_eje_3:setMotor(1.55, 800000)
	sc4_hinge_eje_4:setMotor(-1.55, 800000)
	sc4_hinge_eje_5:setMotor(1.55, 800000)


	sc4_reloj_hinge_1 = logicManager:getHingeJoint("sc4_reloj_hinge_1");
	sc4_reloj_hinge_2 = logicManager:getHingeJoint("sc4_reloj_hinge_2");
	sc4_aguja_hinge_2 = logicManager:getHingeJoint("HingeMinutero");
	sc4_atrezzo_hinge_1 = logicManager:getHingeJoint("sc4_atrezzo_hinge_1");
	sc4_atrezzo_hinge_2 = logicManager:getHingeJoint("sc4_atrezzo_hinge_2");
	sc4_atrezzo_hinge_3 = logicManager:getHingeJoint("sc4_atrezzo_hinge_3");
	
	-- Cerrar puertas
	sc4_reloj_hinge_1:setLimit(40)
	sc4_reloj_hinge_2:setLimit(40)
	sc4_reloj_hinge_1:setMotor(-1.55, 800000)
	sc4_reloj_hinge_2:setMotor(1.55, 800000)

	-- Bloquear aguja
	sc4_aguja_hinge_2:setLimit(0.01)

	-- Bloquear atrezzos
	sc4_atrezzo_hinge_1:setMotor(-1.55, 800000)
	sc4_atrezzo_hinge_2:setMotor(-1.55, 800000)
	sc4_atrezzo_hinge_3:setMotor(-1.55, 800000)

	local vuelta_completa = false
	local sonido_tictac = false

	-- Interruptor
	function onSwitchPressed_sc4_int_reloj(who)
		if (vuelta_completa == false) then
			-- Abrir puertas
			sc4_reloj_hinge_1:setMotor(1.55, 800000)
			sc4_reloj_hinge_2:setMotor(-1.55, 800000)

			-- Abrir atrezzos
			sc4_atrezzo_hinge_1:setMotor(1.55, 800000)
			sc4_atrezzo_hinge_2:setMotor(1.55, 800000)
			sc4_atrezzo_hinge_3:setMotor(1.55, 800000)

			-- Desbloquear aguja y moverla
			sc4_aguja_hinge_2:setLimit(0)
			sc4_aguja_hinge_2:setMotor(1.55, 15000)

			-- Sonido de tic tac
			sonido_tictac = true
			local o_aguja = logicManager:getObject("minutero");
			logicManager:playEventAtPosition("TICTAC", o_aguja:getPos());
			logicManager:setTimer("scene_4_tictac", 0.5)

			-- Encender cable
			sc4_cable_reloj:setEmissive(true)
		end
	end

	-- Timers para el tic tac. Por alguna razón, un timer que se llama a si mismo falla
	function onTimerEnd_scene_4_tictac()		
		if (sonido_tictac == true) then
			local o_aguja = logicManager:getObject("minutero");
			logicManager:playEventAtPosition("TICTAC", o_aguja:getPos());
			logicManager:setTimer("scene_4_tictac2", 0.5)
		end		
	end

	function onTimerEnd_scene_4_tictac2()
		if (sonido_tictac == true) then
			local o_aguja = logicManager:getObject("minutero");
			logicManager:playEventAtPosition("TICTAC", o_aguja:getPos());
			logicManager:setTimer("scene_4_tictac", 0.5)
		end		
	end

	-- Aguja
	

	function onTriggerEnter_sc4_trigger_aguja_alto(who)
		if (who == "minutero") then
			if (vuelta_completa == true) then
				-- Volver a colocar los atrezzos
				sc4_atrezzo_hinge_1:setMotor(-1.55, 800000)
				sc4_atrezzo_hinge_2:setMotor(-1.55, 800000)
				sc4_atrezzo_hinge_3:setMotor(-1.55, 800000)

				-- Volver a cerrar las puertas
				sc4_reloj_hinge_1:setMotor(-1.55, 800000)
				sc4_reloj_hinge_2:setMotor(1.55, 800000)

				-- Bloquear aguja
				sc4_aguja_hinge_2:setLimit(0.01)
				sc4_aguja_hinge_2:setMotor(0, 0)

				-- Reiniciar variable de vuelta completa
				vuelta_completa = false

				-- Reinicar variable de sonido
				sonido_tictac = false

				-- Apagar cable
				sc4_cable_reloj:setEmissive(false)
			end
		end
	end

	function onTriggerEnter_sc4_trigger_aguja_bajo(who)
		if (who == "minutero") then
			vuelta_completa = true
		end
	end

	-- Narrador

	function onTriggerFirstEnter_sc4_trigger_narr_1(who)
		
	end

	function onTriggerFirstEnter_sc4_trigger_narr_2(who)
		
	end

	-- Machacaabuelas del crematorio
	-- Obtener los joints
	sc4_prismatic_crasher_1 = logicManager:getPrismaticJoint("sc4_prismatic_crasher_1");
	sc4_prismatic_crasher_2 = logicManager:getPrismaticJoint("sc4_prismatic_crasher_2");
	sc4_prismatic_crasher_3 = logicManager:getPrismaticJoint("sc4_prismatic_crasher_3");
	sc4_prismatic_crasher_4 = logicManager:getPrismaticJoint("sc4_prismatic_crasher_4");

	-- Activar sus límites
	sc4_prismatic_crasher_1:setLinearLimit(0.1, 10000000, 10000000);
	sc4_prismatic_crasher_2:setLinearLimit(0.1, 10000000, 10000000);
	sc4_prismatic_crasher_3:setLinearLimit(0.1, 10000000, 10000000);
	sc4_prismatic_crasher_4:setLinearLimit(0.1, 10000000, 10000000);

	-- Activar sus temporizadores
	logicManager:setTimer("scene_4_crasher_1_down", 1)
	logicManager:setTimer("scene_4_crasher_2_down", 2)
	logicManager:setTimer("scene_4_crasher_3_down", 3)
	logicManager:setTimer("scene_4_crasher_4_down", 4)

	-- Crasher 1
	function onTimerEnd_scene_4_crasher_1_up()
		sc4_prismatic_crasher_1:setLinearLimit(0.1, 10000000, 10000000);
		logicManager:setTimer("scene_4_crasher_1_down", 4)
	end

	function onTimerEnd_scene_4_crasher_1_down()
		sc4_prismatic_crasher_1:setLinearLimit(1000, 0, 0);
		logicManager:setTimer("scene_4_crasher_1_up", 2)

		-- Sonido
		local o_crasher = logicManager:getObject("sc4_base_");
		logicManager:playEventAtPosition("COMPRESOR_1", o_crasher:getPos());
	end

	-- Crasher 2 (roto)
	function onTimerEnd_scene_4_crasher_2_up()
		sc4_prismatic_crasher_2:setLinearLimit(0.1, 10000000, 10000000);
		logicManager:setTimer("scene_4_crasher_2_down", 3)
	end

	function onTimerEnd_scene_4_crasher_2_down()
		sc4_prismatic_crasher_2:setLinearLimit(3, 0, 0);
		logicManager:setTimer("scene_4_crasher_2_up", 2)
	end

	-- Crasher 3
	function onTimerEnd_scene_4_crasher_3_up()
		sc4_prismatic_crasher_3:setLinearLimit(0.1, 10000000, 10000000);
		logicManager:setTimer("scene_4_crasher_3_down", 3.4)		
	end

	function onTimerEnd_scene_4_crasher_3_down()
		sc4_prismatic_crasher_3:setLinearLimit(1000, 0, 0);
		logicManager:setTimer("scene_4_crasher_3_up", 1.6)

		-- Sonido
		local o_crasher = logicManager:getObject("sc4_base_3");
		logicManager:playEventAtPosition("COMPRESOR_3", o_crasher:getPos());
	end

	-- Crasher 4
	function onTimerEnd_scene_4_crasher_4_up()
		sc4_prismatic_crasher_4:setLinearLimit(0.1, 10000000, 10000000);
		logicManager:setTimer("scene_4_crasher_4_down", 2.8)
	end

	function onTimerEnd_scene_4_crasher_4_down()
		sc4_prismatic_crasher_4:setLinearLimit(1000, 0, 0);
		logicManager:setTimer("scene_4_crasher_4_up", 2)

		-- Sonido
		local o_crasher = logicManager:getObject("sc4_base_4");
		logicManager:playEventAtPosition("COMPRESOR_4", o_crasher:getPos());
	end


	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 4 ********
end

                     -- DEAD --

function onPlayerDead()
	local player = logicManager:getBot("Player");
	player:teleportToPos(initPos);
end

					-- VICTORY --

function onTriggerEnter_Trigger_Victory(who)
	if who == "Player" then
		print(tostring(who) .. " VICTORIAAAA!!!");
		--logicManager:pushPlayerLegsState("fbp_Victory");
	end
end
