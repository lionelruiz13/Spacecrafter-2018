
########### install files ###############

INSTALL(FILES 
	atmosphere.vert atmosphere.frag 
	fog.vert fog.frag 
	stars.frag stars.geom stars.vert
	sky_draw.frag sky_draw.vert
	skylineDraw.frag skylineDraw.vert
	skylineTropicDrawTick.frag skylineTropicDrawTick.vert
	skylineMVPDraw.frag skylineMVPDraw.vert
	skygrid.frag skygrid.geom skygrid.vert
	illuminate.frag illuminate.vert
	body_artificial.frag body_artificial.vert
	body_normal.frag body_normal.vert 
	body_sun.frag body_sun.vert
	body_orbit2d.vert body_orbit2d.geom body_orbit2d.frag 
	body_orbit3d.vert body_orbit3d.geom body_orbit3d.frag 
	body_Axis.vert body_Axis.frag
	moon_normal.frag moon_normal.vert
	moon_bump.frag moon_bump.vert
	splash.frag splash.vert
	font_print.frag font_print.vert
	fbo.frag fbo.vert
	imageViewport.frag imageViewport.vert
	#~ imageHorizontal.frag imageHorizontal.vert
	#~ imageDome.frag imageDome.vert
	imageUnified.frag imageUnified.vert
	#~ imageEquatorial.frag imageEquatorial.vert
	meteor.frag meteor.vert
	sfontDraw.frag sfontDraw.vert
	sfontPrint.frag sfontPrint.vert
	sfontHorizontal.frag sfontHorizontal.vert
	nebulaHint.frag nebulaHint.vert
	nebulaTex.frag nebulaTex.geom nebulaTex.vert
	body_trace.vert body_trace.geom body_trace.frag
	body_halo.vert body_halo.frag
	bodyHints.vert bodyHints.frag
	body_trail.vert body_trail.geom body_trail.frag
	sun_big_halo.frag sun_big_halo.geom sun_big_halo.vert 
	constellationArt.frag constellationArt.geom constellationArt.vert
	constellationBoundary.frag constellationBoundary.vert
	constellationLines.frag constellationLines.vert
	star_pointer.frag star_pointer.geom star_pointer.vert 
	starLines.frag starLines.geom starLines.vert
	object_base_pointer.frag object_base_pointer.geom object_base_pointer.vert 
	landscape2T.vert landscape2T.geom landscape2T.frag
	# milky3d.vert milky3d.geom milky3d.frag
	viewportShape.frag viewportShape.vert 
	body_bump.frag body_bump.vert 
	body_ringed.vert body_ringed.frag 
	ring_planet.vert ring_planet.frag 
	body_night.frag body_night.vert
	colorInverse.frag colorInverse.vert
	milkyway.frag milkyway.vert
	person.frag person.geom person.vert
	screenFader.frag screenFader.vert

	DESTINATION share/${PACKAGE}/shaders
	)

