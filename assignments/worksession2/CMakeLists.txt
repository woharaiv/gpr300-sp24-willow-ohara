file(
 GLOB_RECURSE WORKSESSION2_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION2_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this worksession2's asset folder to bin when it is built
add_custom_target(copyAssetsWS2 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION2_INC} DESTINATION include/worksession2)
add_executable(worksession2 ${WORKSESSION2_SRC} ${WORKSESSION2_INC})
target_link_libraries(worksession2 PUBLIC core IMGUI assimp)
target_include_directories(worksession2 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when worksession2 is built
add_dependencies(worksession2 copyAssetsWS2)