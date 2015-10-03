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

	function onTriggerFirstEnter_scb_trigger_bandas(who)
		logicManager:setBands(true)
	end

	function onTimerEnd_boss_init_animation()
		logicManager:setCanMove(true)
		logicManager:setCanThrow(true)
		logicManager:setCanPull(true)
		logicManager:setCanCancel(true)
		logicManager:setCanTense(true)
		logicManager:setBands(false)
		logicManager:releaseCameraLock()
	end

	function onTriggerFirstEnter_scb_trigger_block_control(who)
		logicManager:setCanMove(false)
		logicManager:setCanThrow(false)
		logicManager:setCanPull(false)
		logicManager:setCanCancel(false)
		logicManager:setCanTense(false)
		logicManager:pushPlayerLegsState("fbp_Idle");
		local boss = logicManager:getBot("Boss")
		logicManager:lockCameraOnPosition(Vector(0, 8, 0))
		logicManager:setTimer("boss_init_animation", 21)
	end

	onSceneLoad_scene_4()
	onSceneLoad_scene_3()
	onSceneLoad_scene_2()
	onSceneLoad_scene_1()
end


function onSceneLoad_scene_1()

	player = logicManager:getBot("Player");
	initPos = player:getPos();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 1 **********

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
		-- Abrir puerta
		sc1_door:setLimit(90)
		sc1_door:setMotor(-1.55, 2000000);
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
	initPos = player:getPos();

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
	
	-- Limitar cadenas al inicio
	sc2_hinge_caida1:setLimit(0.1)
	sc2_hinge_caida2:setLimit(0.1)
	sc2_hinge_caida3:setLimit(0.1)
	sc2_hinge_caida4:setLimit(0.1)
	sc2_hinge_caida5:setLimit(0.1)
	sc2_hinge_caida6:setLimit(0.1)
	sc2_hinge_caida7:setLimit(0.1)

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

		-- Romper fixed joints
		local sc2_fixed_1 = logicManager:getFixedJoint("sc2_fixed_caida1");
		sc2_fixed_1:breakJoint();

		local sc2_fixed_2 = logicManager:getFixedJoint("sc2_fixed_caida2");
		sc2_fixed_2:breakJoint();

		-- Quitar luces
		logicManager:changeAmbientLight(0.2,0.2,0.2,0.95);
	end

	-- Plataforma elevadora
	local sc2_plataforma_elevadora = logicManager:getObject("sc2_plataforma_elevadora")
	local sc2_plataforma_elevadora_orig = sc2_plataforma_elevadora:getPos()

	function onSwitchPressed_sc2_int_pelev(who)

		print(tostring(who) .. " Interruptor subir plataforma");
		sc2_plataforma_elevadora:move(Vector(sc2_plataforma_elevadora_orig.x, sc2_plataforma_elevadora_orig.y + 4.06, sc2_plataforma_elevadora_orig.z), 3);
		sc2_cable:setEmissive(true)

	end

	function onSwitchReleased_sc2_int_pelev(who)

		print(tostring(who) .. " Interruptor bajar plataforma");
		sc2_plataforma_elevadora:move(sc2_plataforma_elevadora_orig, 3);
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
	initPos = player:getPos();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 3 **********

	-- Subir el nivel del agua
	function onTriggerExit_sc3_trigger_tuber(who)
		print(tostring(who) .. " ha dejado en el trigger de tuber");
	
		if (who == "tapa_agua_puzle") then
			logicManager:changeWaterLevel(7.58, 0.25);
			logicManager:createParticleGroup("ps_jet", Vector(16.23, 11.14, 69.9), Quaternion( 0.7071067811865476, 0, 0.7071067811865476, 0));
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
	initPos = player:getPos();

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
	end


	-- Reloj
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
	end

	-- Crasher 4
	function onTimerEnd_scene_4_crasher_4_up()
		sc4_prismatic_crasher_4:setLinearLimit(0.1, 10000000, 10000000);
		logicManager:setTimer("scene_4_crasher_4_down", 2.8)
	end

	function onTimerEnd_scene_4_crasher_4_down()
		sc4_prismatic_crasher_4:setLinearLimit(1000, 0, 0);
		logicManager:setTimer("scene_4_crasher_4_up", 2)
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


























-- ******************** MILESTONES VIEJOS (TODO COMENTADO) ***********************

--[[
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

--]]

