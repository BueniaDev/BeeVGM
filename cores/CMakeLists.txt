add_subdirectory(psg)
add_subdirectory(fm)
add_subdirectory(pcm)

add_library(emu_cores INTERFACE)
target_link_libraries(emu_cores INTERFACE psg_cores fm_cores pcm_cores)