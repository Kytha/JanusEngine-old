SET ( JANUS_SOURCE
    src/Application.cpp
  )

SET ( JANUS_HEADERS
    src/Core.h
    src/Application.h
  )


# Visual Studio source folders
#SOURCE_GROUP (core REGULAR_EXPRESSION src/core/.*)
#SOURCE_GROUP (shapes REGULAR_EXPRESSION src/shapes/.*)

###########################################################################

ADD_LIBRARY ( janus STATIC
  ${JANUS_SOURCE}
  ${JANUS_HEADERS}
  )

ADD_SUBDIRECTORY(vendors/glfw)
TARGET_INCLUDE_DIRECTORIES( janus PUBLIC vendors/glfw/include)
TARGET_LINK_LIBRARIES( janus glfw)

#IF (WIN32)
  # Avoid a name clash when building on Visual Studio
  #SET_TARGET_PROPERTIES ( janus PROPERTIES OUTPUT_NAME libjanus )
#ENDIF()

INSTALL ( TARGETS
  janus
  DESTINATION
  lib
  )
