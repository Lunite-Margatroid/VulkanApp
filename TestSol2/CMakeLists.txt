# TestSol2
file(
	GLOB SOURCES "./*.cpp"
)

add_executable(TestSol2 ${SOURCES})

if(WIN32)
	target_compile_options(TestSol2 PRIVATE "/utf-8")
endif()
	
if(WIN32)
	target_link_libraries(TestSol2 
		PRIVATE
			debug "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}d.lib"
			optimized "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}.lib"
	)	
	message(STATUS "Link ${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}d.lib")
elseif(UNIX)
	target_link_libraries(TestSol2 
		PRIVATE
			debug "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}d.so"
			optimized "${LUA_INSTALLED_DIR}/lib/${LUA_LIB_NAME}.so"
			
	)
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

target_include_directories(TestSol2
	PRIVATE
		"${LUA_INSTALLED_DIR}/include"
		"${SOL2_INSTALLED_DIR}/include"
)