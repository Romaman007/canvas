add_executable(cancans
  main.cpp
  canvas_view.cpp
  canvas_view.hpp

)

target_link_libraries(cancans
  PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
    cancans_core
    cancans_render
)

set_target_properties(cancans PROPERTIES
  WIN32_EXECUTABLE TRUE   # чтобы на Windows не открывался консольный терминал
)
