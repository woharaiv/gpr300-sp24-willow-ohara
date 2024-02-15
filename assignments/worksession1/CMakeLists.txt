file(
 GLOB_RECURSE WORKSESSION1_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION1_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this worksession1's asset folder to bin when it is built
add_custom_target(copyAssetsWS1 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION1_INC} DESTINATION include/worksession1)
add_executable(worksession1 ${WORKSESSION1_SRC} ${WORKSESSION1_INC})
target_link_libraries(worksession1 PUBLIC core IMGUI assimp)
target_include_directories(worksession1 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when worksession1 is built
add_dependencies(worksession1 copyAssetsWS1)