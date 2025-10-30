add_library(cancans_elements
  stroke.cpp
  # сюда будут добавляться другие элементы...
)

target_include_directories(cancans_elements
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_features(cancans_elements PUBLIC cxx_std_20)
