ADD_EXECUTABLE ( SceneEditor src/SceneEditor.cpp)

SET_TARGET_PROPERTIES ( SceneEditor PROPERTIES OUTPUT_NAME SceneEditor )
TARGET_LINK_LIBRARIES ( SceneEditor janus )
TARGET_INCLUDE_DIRECTORIES( SceneEditor PUBLIC ../Janus/src)

INSTALL ( TARGETS
  SceneEditor
  DESTINATION
  bin
  )
