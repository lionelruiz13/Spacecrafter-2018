ADD_SUBDIRECTORY( planetsephems ) 
ADD_SUBDIRECTORY( iniparser ) 


INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/planetsephems ${CMAKE_SOURCE_DIR}/src/iniparser )


set(SRCS
	anchor_creator_cor.cpp
	anchor_manager.cpp
	anchor_point_body.cpp
	anchor_point_observatory.cpp	
	anchor_point_orbit.cpp	
	anchor_point.cpp
	app_command_color.cpp
	app_command_interface.cpp
	app_settings.cpp
	app.cpp
	atmosphere.cpp
	audio.cpp
	axis.cpp
	body_artificial.cpp
	body_bigbody.cpp
	body_color.cpp
	body_decor.cpp
	body_moon.cpp
	body_smallbody.cpp
	body_sun.cpp
	body_trace.cpp
	body.cpp
	call_system.cpp
	cardinals.cpp
	checkkeys.cpp
	constellation_mgr.cpp
	constellation.cpp
	core_executor.cpp
	core_io.cpp
	core.cpp
	CPUInfo.cpp
	dso3d.cpp
	event_handler.cpp
	external_mplayer.cpp
	file_path.cpp
	geodesic_grid.cpp
	glutils.cpp
	grid.cpp
	halo.cpp
	hints.cpp
	hip_star_mgr.cpp
	hip_star_wrapper.cpp
	hip_star.cpp
	ia.cpp
	illuminate_mgr.cpp
	illuminate.cpp
	image_mgr.cpp
	image.cpp
	init_parser.cpp
	io.cpp
	joypad_controller.cpp
	landscape.cpp
	log.cpp
	main.cpp
	mCity_mgr.cpp
	md5.cpp
	media.cpp
	meteor_mgr.cpp
	meteor.cpp
	milkyway.cpp
	mkfifo.cpp
	model3D.cpp
	navigator.cpp
	nebula_mgr.cpp
	nebula.cpp
	object_base.cpp
	object.cpp
	objl_mgr.cpp
	objl.cpp
	observer.cpp
	ojm_mgr.cpp
	ojm.cpp
	ojml.cpp
	oort.cpp
	orbit_2d.cpp
	orbit_3d.cpp
	orbit_creator_cor.cpp
	orbit_plot.cpp
	orbit.cpp
	perf_debug.cpp
	projector.cpp
	ring.cpp
	s_font.cpp
	s_texture.cpp
	save_screen.cpp
	screenFader.cpp
	script_mgr.cpp
	script.cpp
	sdl_facade.cpp
	shader.cpp
	signals.cpp
	sky_draw.cpp
	sky_localizer.cpp
	skybright.cpp
	skygrid_mgr.cpp
	skygrid.cpp
	skylight.cpp
	skyline_mgr.cpp
	skyline.cpp
	skyperson.cpp
	solarsystem.cpp
	space_date.cpp
	sphere_geometry.cpp
	starLines.cpp
	starManager.cpp
	starNavigator.cpp
	stateGL.cpp
	string_array.cpp
	text_mgr.cpp
	text.cpp
	time_mgr.cpp
	tone_reproductor.cpp
	trail.cpp
	translator.cpp
	tully.cpp
	ubo_cam.cpp
	ui_tui.cpp
	ui_tuiconf.cpp
	ui.cpp
	utility.cpp
	video_player.cpp
	viewport.cpp
	vr360.cpp
	zone_array.cpp
	)
    
set(HEADERS
	anchor_creator_cor.hpp
	anchor_manager.hpp
	anchor_point_body.hpp
	anchor_point_observatory.hpp
	anchor_point_orbit.hpp
	anchor_point.hpp
	app_command_color.hpp
	app_command_interface.hpp
	app_settings.hpp
	app.hpp
	atmosphere.hpp
	audio.hpp
	axis.hpp
	backup_mgr.hpp
	body_artificial.hpp
	body_bigbody.hpp
	body_color.hpp
	body_decor.hpp
	body_moon.hpp
	body_smallbody.hpp
	body_sun.hpp
	body_trace.hpp
	body.hpp
	call_system.cpp
	callback_helpers.hpp
	callbacks.hpp
	cardinals.hpp
	checkkeys.hpp
	clock.hpp
	cmutex.hpp
	constellation_mgr.hpp
	constellation.hpp
	core_executor.hpp
	core_io.hpp
	core.hpp
	CPUInfo.hpp
	dso3d.hpp
	event.hpp
	event_handler.hpp
	event_manager.hpp
	external_mplayer.hpp
	fader.hpp
	file_path.hpp
	fmath.hpp
	geodesic_grid.hpp
	gettext.hpp
	glutils.hpp
	grid.hpp
	halo.hpp
	hints.hpp
	hip_star_mgr.hpp
	hip_star_wrapper.hpp
	hip_star.hpp
	ia.hpp
	illuminate_mgr.hpp
	illuminate.hpp
	image_mgr.hpp
	image.hpp
	init_parser.hpp
	intrusive_ptr.hpp
	io.hpp
	landscape.hpp
	log.hpp
	matrix4.hpp
	mCity_mgr.hpp
	md5.hpp
	media.hpp
	meteor_mgr.hpp
	meteor.hpp
	milkyway.hpp
	mkfifo.hpp
	model3D.hpp
	navigator.hpp
	nebula_mgr.hpp
	nebula.hpp
	object_base.hpp
	object_type.hpp
	object.hpp
	objl_mgr.hpp
	objl.hpp
	observer.hpp
	ojm_mgr.hpp
	ojm.hpp
	ojml.hpp
	oort.hpp
	orbit_2d.hpp
	orbit_3d.hpp
	orbit_creator_cor.hpp
	orbit_plot.hpp
	orbit.hpp
	perf_debug.hpp
	projector.hpp
	ring.hpp
	s_font.hpp
	s_texture.hpp
	save_screen.hpp
	scalable.hpp
	screenFader.hpp
	script_mgr.hpp
	script.hpp
	sdl_facade.hpp
	shader.hpp
	signals.hpp
	sky_draw.hpp
	sky_localizer.hpp
	skybright.hpp
	skygrid_mgr.hpp
	skygrid.hpp
	skylight.hpp
	skyline_mgr.hpp
	skyline.hpp
	skyperson.hpp
	solarsystem.hpp
	solve.hpp
	space_date.hpp
	spacecrafter.hpp
	sphere_geometry.hpp
	starLines.hpp
	starManager.hpp
	starNavigator.hpp
	stateGL.hpp
	string_array.hpp
	text_mgr.hpp
	text.hpp
	ThreadPool.hpp
	time_mgr.hpp
	tone_reproductor.hpp
	trail.hpp
	translations.hpp
	translator.hpp
	tully.hpp
	ubo_cam.hpp
	ui_tui.hpp
	ui.hpp
	utility.hpp
	vecmath.hpp
	vector2.hpp
	vector3.hpp
	vector4.hpp
	video_player.hpp
	viewport.hpp
	vr360.hpp
	zone_array.hpp
	zone_data.hpp 
    )

set (CMAKE_CXX_STANDARD 17)


# création de spacecrafter
add_executable(spacecrafter ${SRCS} ${HEADERS})
target_link_libraries(spacecrafter iniparser planetsephems ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${SDL2_MIXER_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_NET_LIBRARY} ${GLEW_LIBRARY} )
INSTALL(TARGETS spacecrafter DESTINATION bin)
