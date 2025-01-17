cmake_minimum_required(VERSION 3.6)
project(Morpheus VERSION 0.1.0)


# Set include directories for other projects
set(MORPHEUS_INCLUDE_DIR  
	"${CMAKE_CURRENT_SOURCE_DIR}/Engine/include" 
	CACHE INTERNAL "MORPHEUS_INCLUDE_DIR")

set(ENTT_INCLUDE_DIR
	"${CMAKE_SOURCE_DIR}/entt/src"
	CACHE INTERNAL "ENTT_INCLUDE_DIR")

function(add_third_party_libraries APP_NAME)
	message(${APP_NAME})
	target_include_directories(${APP_NAME} PRIVATE
		${CMAKE_SOURCE_DIR}/entt/src/
		${CMAKE_SOURCE_DIR}/json/single_include
		${CMAKE_SOURCE_DIR}/mio/single_include}
		${CMAKE_SOURCE_DIR}/cereal/include/)
endfunction()

function(add_msvc_ignores TARGET_NAME)
    target_compile_options(${TARGET_NAME} PRIVATE /wd4127)
    target_compile_options(${TARGET_NAME} PRIVATE /wd4458)
    target_compile_options(${TARGET_NAME} PRIVATE /wd4244)
    target_compile_options(${TARGET_NAME} PRIVATE /wd4267)
    target_compile_options(${TARGET_NAME} PRIVATE /wd26819)
    target_compile_options(${TARGET_NAME} PRIVATE /wd26812)
    target_compile_options(${TARGET_NAME} PRIVATE /wd26495)
    target_compile_options(${TARGET_NAME} PRIVATE /wd4201)
endfunction()

function(add_engine_includes LIB_NAME)
	if(USE_BULLET)
		INCLUDE_DIRECTORIES(
			${LIB_NAME} ${BULLET_PHYSICS_SOURCE_DIR}/src
		)
	endif()

	INCLUDE_DIRECTORIES(
		${LIB_NAME} 
		${DILIGENT_CORE_SOURCE_DIR}/Common/interface
		${DILIGENT_CORE_SOURCE_DIR}/Primitives/interface
		${DILIGENT_CORE_SOURCE_DIR}/Graphics/GraphicsEngine/interface
		${ENTT_INCLUDE_DIR}
	)
endfunction()

function(add_engine_app APP_NAME SOURCE INCLUDE SHADERS ASSETS)

	add_engine_includes(${APP_NAME})

	if (USE_BULLET)
		LINK_LIBRARIES(
			${APP_NAME} BulletDynamics BulletCollision LinearMath
		)
	endif()

    set_source_files_properties(${SHADERS} PROPERTIES VS_TOOL_OVERRIDE "None")
    set(ALL_ASSETS ${ASSETS} ${SHADERS})
    add_target_platform_app(${APP_NAME} "${SOURCE}" "${INCLUDE}" "${ALL_ASSETS}")

    set_source_files_properties(${ALL_ASSETS} PROPERTIES 
        VS_DEPLOYMENT_LOCATION "."
        MACOSX_PACKAGE_LOCATION "Resources" 
	)
	
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(${APP_NAME} PRIVATE Threads::Threads)

    if(PLATFORM_WIN32)
        set_target_properties(${APP_NAME} PROPERTIES 
            VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        )
        copy_required_dlls(${APP_NAME})
        #append_sample_base_win32_source(${APP_NAME})
    elseif(PLATFORM_UNIVERSAL_WINDOWS)
        append_sample_base_uwp_source(${APP_NAME})
		package_required_dlls(${APP_NAME})
    endif()

    target_include_directories(${APP_NAME}
    PRIVATE	
		src
		${CMAKE_SOURCE_DIR}/entt/src
		${CMAKE_SOURCE_DIR}/json/single_include
		${CMAKE_SOURCE_DIR}/im3d
    )

    target_link_libraries(${APP_NAME}
    PRIVATE
        # On Linux we must have Diligent-NativeAppBase go first, otherwise the linker 
        # will fail to resolve Diligent::CreateApplication() function.
        #Diligent-NativeAppBase
        Diligent-BuildSettings
		Morpheus-Engine
		${ASSIMP_LIBRARIES}
    )
    set_common_target_properties(${APP_NAME})

    if(MSVC)
        # Disable MSVC-specific warnings
        add_msvc_ignores(${APP_NAME})
    endif()

    source_group("src" FILES ${SOURCE} ${INCLUDE})
    source_group("assets" FILES ${ALL_ASSETS})	

    if(PLATFORM_WIN32 OR PLATFORM_LINUX)
        # Copy assets to target folder
        add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                "\"$<TARGET_FILE_DIR:${APP_NAME}>\"")
	endif()

    set_target_properties(${APP_NAME} PROPERTIES
        FOLDER Morpheus
    )
	
	target_compile_features(${APP_NAME} PRIVATE cxx_std_17)
endfunction()

if(TARGET Diligent-NativeAppBase AND TARGET Diligent-TextureLoader AND TARGET Diligent-Imgui)
	add_subdirectory(Engine)
endif()
