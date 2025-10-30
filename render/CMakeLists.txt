add_library(cancans_render
  renderer.cpp
)

target_include_directories(cancans_render
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# связь с ядром
target_link_libraries(cancans_render
  PRIVATE cancans_core
)

target_compile_features(cancans_render PUBLIC cxx_std_20)
