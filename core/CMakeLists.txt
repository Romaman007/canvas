add_subdirectory(elements)

add_library(cancans_core
  camera.cpp
  scene.cpp
)

target_include_directories(cancans_core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cancans_core
  PUBLIC cancans_elements
)

target_compile_features(cancans_core PUBLIC cxx_std_20)
