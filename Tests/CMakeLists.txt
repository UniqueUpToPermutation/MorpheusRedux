cmake_minimum_required(VERSION 3.6)

if(TARGET Morpheus-Engine)
	add_custom_target(MorpheusTests)

	add_subdirectory(PBR)
	add_subdirectory(AsyncTest)
	add_subdirectory(SpriteBatchTest)
	add_subdirectory(Im3dTest)
	add_subdirectory(Im3dGizmo)
	add_subdirectory(RawTextureTest)
	add_subdirectory(HelloWorld)
	add_subdirectory(EmbeddedGeoTest)
	add_subdirectory(RaytraceTest)

	if (USE_PBRT)
		add_subdirectory(Pbrt-Test)
	endif()
endif()