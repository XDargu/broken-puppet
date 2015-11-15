SLB.using(SLB)

-- Funciones útiles de LUA
function print(s)
	logicManager:print(tostring(s));
end
function error(s)
	logicManager:print("ERROR: " .. tostring(s));
end

bossRopeTimesThrowed = 0;
bossSecuence = false;

_print = print
clearCoroutines()

function onSceneLoad_my_file()
	
	logicManager:setFogAttributes(1000, -10000);
	--logicManager:changeWaterLevel(7.58, 0.25);
	--onSceneLoad_scene_final_boss()
	--onSceneLoad_scene_4()
	--onSceneLoad_scene_3()
	--onSceneLoad_scene_2()
	--onSceneLoad_scene_1()	
end


-- Escena del menú principal
function onSceneLoad_scene_menu()
	-- Parpadeo de la lámpara
	logicManager:setTimer("menu_lamp_parp", 5)
end

function onTimerEnd_menu_lamp_parp()
	startCoroutine("lamp_parp", lamp_parp)	
end

function lamp_parp()
	local parp_time = 0.02
	local lamp_parp = logicManager:getObject("lampara_menu_parpadeo");		

	lamp_parp:setEmissive(false)
	waitTime(parp_time)
	lamp_parp:setEmissive(true)

	waitTime(0.5)
	lamp_parp:setEmissive(false)
	waitTime(parp_time)
	lamp_parp:setEmissive(true)
	waitTime(parp_time)
	lamp_parp:setEmissive(false)
	waitTime(parp_time)
	lamp_parp:setEmissive(true)
	
	logicManager:setTimer("menu_lamp_parp", 15)
end


-- Escena del boss
function onSceneLoad_scene_final_boss()

	bossSecuence = false;
	player = logicManager:getBot("Player");
	initPos = player:getPosition();
	respawnPos = player:getPosition();
	mBoss = logicManager:getObject("Boss");

	-- Colocar collider del escenario
	local scene_collider = logicManager:getObject("escenario_pared_collider");		
	scene_collider:setPos(Vector(1000, 1000, 1000));

	function onTriggerFirstEnter_sc5_trigger_narr1(who)
		logicManager:playSubtitles("NARRSC501");
	end
	
	function onTriggerEnter_PitTrigger(who)
		print(tostring(who) .. "Entrado en el trigger");

		if who == "Player" then
			--logicManager:pushPlayerLegsState("fbp_WakeUp");
			logicManager:pushPlayerLegsState("fbp_WakeUpTeleport");
			player:teleportToPos(respawnPos);

			if (player:getLife(0) <= 0) then
				player:setLife(100);
			end
		end

	end

	function onTriggerFirstEnter_scb_trigger_bandas(who)
		--logicManager:setMediumShotActive(true)

		logicManager:setFogAttributes(1000, -10);

		-- Bajar luz ambiental
		logicManager:changeAmbientLight(0.7, 0.7, 0.7, 0.1);

		-- Evitar que pueda trastear con cuerdas

		logicManager:setCanThrow(false)
		logicManager:setCanPull(false)
		logicManager:setCanCancel(false)
		logicManager:setCanTense(false)
	end

	function onTimerEnd_boss_init_animation()
		bossSecuence = false;
		logicManager:setCanMove(true)
		logicManager:setCanThrow(true)
		logicManager:setCanPull(true)
		logicManager:setCanCancel(true)
		logicManager:setCanTense(true)
		logicManager:setBands(false)
		logicManager:releaseCameraLock()
		logicManager:resetPlayerCamera()

		-- Activar GUI
		logicManager:drawGUI(true)

		local antiRain = logicManager:getObject("anti_rain_collider_boss");		
		antiRain:setPos(Vector(5000, 5000, 5000));

		respawnPos = player:getPosition();
		initPos = respawnPos;
	end


	function end_speak()
		-- Reiniciamos cuerdas tiradas
		bossRopeTimesThrowed = 0;
		bossSecuence = true;

		local substitute = logicManager:getBot("Substitute");		
		logicManager:lockCameraOnBotBone(substitute, 4);

		-- Reactivamos cuerdas
		logicManager:setCanThrow(true)
		--logicManager:setCanPull(true)
		--logicManager:setCanCancel(true)
		logicManager:setCanTense(true)			
	end

	function onTriggerFirstEnter_scb_trigger_block_control(who)
	bossSecuence = false;
		local substite = logicManager:getObject("Substitute")	
		substite:initLittleTalk();

		logicManager:setCanMove(false)
		logicManager:pushPlayerLegsState("fbp_Idle");
		
		startCoroutine("playCinematic", playCinematic);
		
		-- Obtenemos la posición de la sustituta y le apuntamos
		--local substitute = logicManager:getBot("Substitute");
		
		--logicManager:lockCameraOnBotBone(substitute, 4);

		-- Zoom in con FOV
		--logicManager:changeFov(6, 3);

	end

	function playCinematic()
		-- Bandas cinemáticas
		logicManager:setBands(true)
		-- Desactivar niebla
		logicManager:setFogAttributes(1000, -1000);

		-- Cancelar cuerdas
		logicManager:cancelAllStrings()

		-- Desactivar GUI
		logicManager:drawGUI(false)

		local player_start_pos = player:getPosition();
		print("asdasd1\n")
		local player_cinematic_pos_obj = logicManager:getObject("player_cinematic_place")
		print("asdasd2\n")
		local cinematic_pos = player_cinematic_pos_obj:getPos();
		print("asdasd3\n")

		local Camera01 = logicManager:getObject("Camera01");
		local Camera002 = logicManager:getObject("Camera002");
		logicManager:playAnimation("Camera_01_sustituta", Camera01)
		logicManager:playAnimation("Camera_002_sustituta", Camera002)

		waitTime(1)
		-- Iniciar cinemática en dos cámaras		
						
		logicManager:changeCamera("Camera01")				
		player:teleportToPos(cinematic_pos);
		waitTime(6)

		logicManager:changeCamera("Camera002")

		waitTime(7)

		logicManager:changeCamera("Camera01")

		waitTime(15.77)
		--player:teleportToPos(player_start_pos);

		-- Fin cinemática
		end_speak()

	end

	function onSubstituteHanged()
		local boss = logicManager:getObject("Boss")	
		boss:riseUpBoss()
		logicManager:setMediumShotActive(true)
		logicManager:changeCamera("PlayerCamera");

		-- Activar niebla
		logicManager:setFogAttributes(1000, -5);

		-- Colocar anti rain collider
		local antiRain = logicManager:getObject("anti_rain_collider_boss");		
		local playerPos = player:getPosition();
		antiRain:setPos(playerPos);
		
		local boss = logicManager:getBot("Boss")
		logicManager:lockCameraOnPosition(Vector(0, 8, 0), true)
		logicManager:setTimer("boss_init_animation", 28)
		
		startCoroutine("bossShake", bossShake)
		startCoroutine("bossInitialRain",  bossInitialRain)
		
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
		local trono = logicManager:getObject("boss_trono_sustituta"); -- eliminar trono
		trono:setRender(false);
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

		-- Colocar collider del escenario
		local scene_collider = logicManager:getObject("escenario_pared_collider");		
		scene_collider:setPos(Vector(0, 0, 0));
	end


	function bossInitialRain()
		-- Tiempos
		local first_arm = 0.72
		local second_arm = 4.79
		local get_up = 7.8
		local first_arm_2 = 12.1
		local second_arm_2 = 12.8
		local scream = 18.44

		local elapsed = 0
		
		waitTime(first_arm)
		elapsed = first_arm
		mBoss:initialRain(15);	
		waitTime(0.5)

		elapsed = elapsed + 0.5
		waitTime(second_arm - elapsed)
		elapsed = second_arm
		mBoss:initialRain(20);	
		waitTime(0.5)
		
		elapsed = elapsed + 0.5
		waitTime(get_up - elapsed)
		elapsed = get_up
		--mBoss:initialRain(30);	
		waitTime(0.5)

		elapsed = elapsed + 0.5
		waitTime(first_arm_2 - elapsed)
		elapsed = first_arm_2
		mBoss:initialRain(1);	
		waitTime(0.5)

		elapsed = elapsed + 0.5
		waitTime(second_arm_2 - elapsed)
		elapsed = second_arm_2
		mBoss:initialRain(1);	
		waitTime(0.5)

		waitTime(0.5)
		elapsed = elapsed + 0.5
		waitTime(scream - elapsed)
		elapsed = scream
		mBoss:initialRain(120);	
		waitTime(0.5)


		waitTime(1.8)
		local objToLook = mBoss:firstBombBoss();

		waitTime(1.6)
		logicManager:lockCameraOnObject(objToLook);
	end

end


function onSceneLoad_scene_1()
bossSecuence = false;
	player = logicManager:getBot("Player");
	initPos = player:getPosition();

	-- Diálogo inicial
	logicManager:playSubtitles("NARRSC101");

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
				
				logicManager:setFogAttributes(1000, -1000);

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
		logicManager:playSubtitles("NARRSC103");
	end

	function onTriggerFirstEnter_scene1_trigger_narr2(who)
		logicManager:playSubtitles("NARRSC104");
	end
	

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 1 ********
end

function onSceneLoad_scene_2()
bossSecuence = false;
	player = logicManager:getBot("Player");
	initPos = player:getPosition();

	-- ******** PRUEBA PUZZLES FINALES ESCENA 2 **********
	-- Hacer que al empezar el player mire al sitio correcto
	logicManager:cameraLookAtPosition(Vector(initPos.x, initPos.y + 1.7, initPos.z - 100))

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
	function onTriggerFirstEnter_scene2_trigger_narr1(who)
		logicManager:playSubtitles("NARRSC201");
	end

	function onTriggerFirstEnter_scene2_trigger_narr2(who)
		logicManager:playSubtitles("NARRSC202");
	end

	function onTriggerFirstEnter_scene2_trigger_narr3(who)
		logicManager:playSubtitles("NARRSC203");
	end

	function onTriggerFirstEnter_scene2_trigger_narr4(who)
		logicManager:setTimer("timerSc2Narr4", 1);
	end

	function onTimerEnd_timerSc2Narr4()
		logicManager:playSubtitles("NARRSC204");
	end

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 2 ********
end

function onSceneLoad_scene_3()
bossSecuence = false;
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
	local water_level_down = false
	function onTriggerExit_sc3_trigger_desag(who)
		print(tostring(who) .. " ha dejado en el trigger de desag");
	
		if who == "tapa_desague_puzle" then
			if water_level_down == false then
				logicManager:changeWaterLevel(-3.3, 0.25);
				logicManager:setTimer("sc3_timer_kath_dialog", 5)

				-- Partícula de desagüe
				logicManager:createParticleGroup("ps_twister", Vector(-0.0129995, -3.188, 83.822), Quaternion(-0.71, 0, 0, 0.71))

				water_level_down = true
			end
			
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

	-- Narrador
	function onTriggerFirstEnter_sc3_trigger_narr_1(who)
		logicManager:playSubtitles("NARRSC301");
	end

	function onTimerEnd_sc3_timer_kath_dialog(who)
		logicManager:playSubtitles("KATHSC301");
	end

	-- Salir de la escena
	function onTriggerEnter_sc3_trigger_fin(who)
		logicManager:loadScene("data/scenes/scene_4.xml");
	end

	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 3 ********
end


function onSceneLoad_scene_4()
bossSecuence = false;
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

		-- Narración
		logicManager:playSubtitles("KATHSC402");

		-- Música
		logicManager:playEvent("MUSIC_ESC")	
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
			sc4_reloj_hinge_1:setLimit(25)
			sc4_reloj_hinge_2:setLimit(60)
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
				sc4_reloj_hinge_1:setLimit(40)
				sc4_reloj_hinge_2:setLimit(40)
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

	-- Narrador

	function onTriggerFirstEnter_sc4_trigger_narr_1(who)
		logicManager:playSubtitles("KATHSC401");
		local prefab_generator = logicManager:getObject("prefabGenerator");
		prefab_generator:generatePrefabs(20);		
	end

	function onTriggerFirstEnter_sc4_trigger_narr_2(who)
		logicManager:playSubtitles("NARRSC402");		
	end

	-- Salir de la escena
	function onTriggerEnter_sc4_trigger_end(who)
		logicManager:loadScene("data/scenes/scene_final_boss.xml");		
	end


	-- ****** FIN PRUEBA PUZZLES FINALES ESCENA 4 ********
end

                     -- DEAD --

function onPlayerDead(phrase, subtitle_guid, time)
	if (phrase >= 0) then
		logicManager:setTimer("resetPlayerPos", time)	
		logicManager:playSubtitles(subtitle_guid);		
		logicManager:playEventParameter("NARR_DEATH", "phrase", phrase);
	else
		local bot_player = logicManager:getBot("Player");
		bot_player:setLife(100);
		bot_player:teleportToPos(initPos);
		logicManager:pushPlayerLegsState("fbp_Idle");
	end
end

function onTimerEnd_resetPlayerPos()
	local bot_player = logicManager:getBot("Player");
	bot_player:setLife(100);
	bot_player:teleportToPos(initPos);
	logicManager:pushPlayerLegsState("fbp_Idle");
end

					-- VICTORY --

function onTriggerEnter_Trigger_Victory(who)
	if who == "Player" then
		print(tostring(who) .. " VICTORIAAAA!!!");
		--logicManager:pushPlayerLegsState("fbp_Victory");
	end
end


			--- Try Rope Substitute ---

function onTimerEnd_wait_for_lanch()
	logicManager:setCanThrow(true)
	-- debemos llamar a una funcion que apunte a la sustituta
	local hang_place = logicManager:getObject("substitute_hang_place")
	logicManager:lockCameraOnPosition(hang_place:getPos(), false)		
end

function onBossRopeThrow()
	if bossSecuence == true then
		if bossRopeTimesThrowed == 1 then			
			-- Linea a borrar en cuanto se asegure el impacto en la prota
			--local substitute = logicManager:getBot("Substitute");		
			--logicManager:lockCameraOnBotBone(substitute, 4);
			bossSecuence = false
			logicManager:setCanThrow(false)
		else
			if bossRopeTimesThrowed == 0 then						
				bossRopeTimesThrowed = 1
				-- debemos llamar a una funcion que apunte al techo
				
				-- obtenemos a la sustitute
				local mSubstitute = logicManager:getObject("Substitute");		
				mSubstitute:hitSubstitute();
				logicManager:setTimer("wait_for_lanch", 1)		
				logicManager:setCanThrow(false)
			end
		end
	end
end

function onExtraButtonPressed()
	startCoroutine("extraCoroutine", extraCoroutine);
end

cam_val = 0
function extraCoroutine()
	
	if (cam_val == 0) then
		local camera = logicManager:getObject("Camara_Trailer");
	
		logicManager:changeCamera("Camara_Trailer")
		waitTime(2)
		logicManager:playAnimation("sc2_cinematica_trailer1", camera)
		waitTime(17)
		logicManager:changeCamera("PlayerCamera")
	end
	if (cam_val == 1) then
		local camera = logicManager:getObject("Camara_Trailer");
	
		logicManager:changeCamera("Camara_Trailer")
		waitTime(2)
		logicManager:playAnimation("sc2_cinematica_trailer2", camera)
		waitTime(14)
		logicManager:changeCamera("PlayerCamera")
		cam_val = -1
	end
	cam_val = cam_val + 1

end