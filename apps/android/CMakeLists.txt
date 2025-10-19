add_library(bim SHARED main.cpp)
target_link_libraries(bim
  PRIVATE
  bim_axmol_app
  bim_axmol_jni
)
post_build_strip(bim)
