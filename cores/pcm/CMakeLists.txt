add_subdirectory(BeePCM)

add_library(pcm_cores INTERFACE)
target_link_libraries(pcm_cores INTERFACE libbeepcm)