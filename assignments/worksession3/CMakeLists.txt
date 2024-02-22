file(
 GLOB_RECURSE WORKSESSION3_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION3_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this worksession3's asset folder to bin when it is built
add_custom_target(copyAssetsWS3 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION3_INC} DESTINATION include/worksession3)
add_executable(worksession3 ${WORKSESSION3_SRC} ${WORKSESSION3_INC})
target_link_libraries(worksession3 PUBLIC core IMGUI assimp)
target_include_directories(worksession3 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when worksession3 is built
add_dependencies(worksession3 copyAssetsWS3)