SET ( GLAD_SOURCE
    src/glad.c
  )

SET ( GLAD_HEADERS
    include/glad/glad.h
    include/KHR/khrplatform.h
  )

ADD_LIBRARY ( Glad STATIC
  ${GLAD_SOURCE}
  ${GLAD_HEADERS}
  )

TARGET_INCLUDE_DIRECTORIES(Glad PUBLIC include)
