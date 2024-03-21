file(
 GLOB_RECURSE WORKSESSION4_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION4_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this worksession4's asset folder to bin when it is built
add_custom_target(copyAssetsWS4 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION4_INC} DESTINATION include/worksession4)
add_executable(worksession4 ${WORKSESSION4_SRC} ${WORKSESSION4_INC})
target_link_libraries(worksession4 PUBLIC core IMGUI assimp)
target_include_directories(worksession4 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when worksession4 is built
add_dependencies(worksession4 copyAssetsWS4)