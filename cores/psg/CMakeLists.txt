add_subdirectory(BeePSG)

add_library(psg_cores INTERFACE)
target_link_libraries(psg_cores INTERFACE libbeepsg)