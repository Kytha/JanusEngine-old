ADD_EXECUTABLE ( SceneEditor src/SceneEditor.cpp)

SET_TARGET_PROPERTIES ( SceneEditor PROPERTIES OUTPUT_NAME SceneEditor )
TARGET_LINK_LIBRARIES ( SceneEditor janus )
TARGET_INCLUDE_DIRECTORIES( SceneEditor PUBLIC ../Janus/src)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/assets
)

add_dependencies(SceneEditor copy_assets)

INSTALL ( TARGETS
  SceneEditor
  DESTINATION
  bin
  )
