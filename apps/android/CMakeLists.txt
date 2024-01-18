add_library(bim SHARED main.cpp)
target_link_libraries(bim
  PRIVATE
  bim_app
  bim_axmol_jni
)
