add_subdirectory(BeeNuked)

add_library(fm_cores INTERFACE)
target_link_libraries(fm_cores INTERFACE libbeenuked)