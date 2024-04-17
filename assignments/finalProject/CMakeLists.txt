file(
 GLOB_RECURSE FINALPROJECT_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE FINALPROJECT_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this finalProject's asset folder to bin when it is built
add_custom_target(copyAssetsFinal ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${FINALPROJECT_INC} DESTINATION include/finalProject)
add_executable(finalProject ${FINALPROJECT_SRC} ${FINALPROJECT_INC} "Particle.cpp" "Particle.h" "ParticleContact.cpp" "ParticleContact.h")
target_link_libraries(finalProject PUBLIC core IMGUI assimp)
target_include_directories(finalProject PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when finalProject is built
add_dependencies(finalProject copyAssetsFinal)