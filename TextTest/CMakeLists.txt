# TextTest

file(
	GLOB SOURCES "./*.cpp"
)

file(GLOB PRIVATE_HEADERS "./*.h" "./*.hpp")

add_executable(TextTest ${SOURCES} ${PRIVATE_HEADERS})

# 仅在windows下使用传递utf-8编码选项
if(WIN32)
	target_compile_options(TextTest PRIVATE "/utf-8")
endif()


target_link_libraries(TextTest PRIVATE LunarTextIO)
