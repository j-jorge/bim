add_library(bm_net INTERFACE)
target_include_directories(bm_net INTERFACE main/include)
target_link_libraries(bm_net INTERFACE iscool_net)
