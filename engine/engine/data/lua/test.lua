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

function onSceneLoad_scene_1()
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
	local cable = logicManager:getObject("cable_int_sc1_door")	
	cable:setEmissive(false)

	-- Cerrar puerta
	local sc1_door = logicManager:getHingeJoint("sc1_hinge_door");
	sc1_door:setLimit(0.1)

	function onSwitchPressed_int_sc1_door(who)
		-- Encender cable
		cable:setEmissive(true)
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
	-- ******** PRUEBA PUZZLES FINALES ESCENA 3 **********

	-- Subir el nivel del agua
	function onTriggerExit_sc3_trigger_tuber(who)
		print(tostring(who) .. " ha dejado en el trigger de tuber");
	
		if who == "tapa_agua_puzle" then
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
	-- ******** PRUEBA PUZZLES FINALES ESCENA 4 **********


	-- *** Ascensor ***

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

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 4 ********
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

