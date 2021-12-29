CMAKE_MINIMUM_REQUIRED ( VERSION 3.12 )

PROJECT (janus LANGUAGES CXX C)


#OPTION(JANUS_FLOAT_AS_DOUBLE "Use 64-bit floats" OFF)

IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++20")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
ENDIF()

IF(MSVC)
  ADD_DEFINITIONS (/D _CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS (/D YY_NO_UNISTD_H)
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES RELEASE)
  ADD_DEFINITIONS (-DNDEBUG)
ENDIF()
###########################################################################
# Helios

SET ( JANUS_SOURCE
    Janus/src/Application.cpp
  )

SET ( JANUS_HEADERS
    Janus/src/Core.h
    Janus/src/Application.h
  )


# Visual Studio source folders
#SOURCE_GROUP (core REGULAR_EXPRESSION src/core/.*)
#SOURCE_GROUP (shapes REGULAR_EXPRESSION src/shapes/.*)

###########################################################################

ADD_LIBRARY ( janus STATIC
  ${JANUS_SOURCE}
  ${JANUS_HEADERS}
  )

IF (WIN32)
  # Avoid a name clash when building on Visual Studio
  SET_TARGET_PROPERTIES ( janus PROPERTIES OUTPUT_NAME libjanus )
ENDIF()

SET(ALL_JANUS_LIBS
  janus
)

ADD_EXECUTABLE ( SceneEditor SceneEditor/src/SceneEditor.cpp)

SET_TARGET_PROPERTIES ( SceneEditor PROPERTIES OUTPUT_NAME SceneEditor )
TARGET_LINK_LIBRARIES ( SceneEditor ${ALL_JANUS_LIBS} )
TARGET_INCLUDE_DIRECTORIES( SceneEditor PUBLIC Janus/src)

INSTALL ( TARGETS
  SceneEditor
  DESTINATION
  bin
  )

INSTALL ( TARGETS
  janus
  DESTINATION
  lib
  )