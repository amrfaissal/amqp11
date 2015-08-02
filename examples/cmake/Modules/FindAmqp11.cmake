get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} PATH)

find_library (AMQP11_LIBRARY NAMES amqp11 HINTS ${PARENT_DIR}/lib NO_DEFAULT_PATH)
find_path (AMQP11_HEADERS amqp11.hpp ${PARENT_DIR}/include)

message (STATUS "===+> ${AMQP11_HEADERS}")

if (${AMQP11_LIBRARY} MATCHES "NOTFOUND")
  set (Amqp11_FOUND FALSE CACHE INTERNAL "")
  message (STATUS "Amqp11 library not found.")
  unset (AMQP11_LIBRARY)
else()
  set (Amqp11_FOUND TRUE CACHE INTERNAL "")
  message (STATUS "Found Amqp11 library: ${AMQP11_LIBRARY}")
endif()

set(CMAKE_REQUIRED_INCLUDES ${AMQP11_HEADERS})
