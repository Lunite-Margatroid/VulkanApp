# TestSol2
file(
	GLOB SOURCES "./*.cpp"
)

add_executable(TestSol2 ${SOURCES})

if(WIN32)
	target_compile_options(TestSol2 PRIVATE "/utf-8")
endif()

file(STRINGS "${LUA_INSTALLED_DIR}/include/lua.h" _lua_major_line REGEX "^#define LUA_VERSION_MAJOR")
file(STRINGS "${LUA_INSTALLED_DIR}/include/lua.h" _lua_minor_line REGEX "^#define LUA_VERSION_MINOR")
string(REGEX MATCH "[0-9]+" _lua_major "${_lua_major_line}")
string(REGEX MATCH "[0-9]+" _lua_minor "${_lua_minor_line}")
set(LUA_LIB_NAME "lua${_lua_major}${_lua_minor}")
	
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