cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(PROJECT_NAME "AMQP client library for C++11")
project( ${PROJECT_NAME} )

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")


# RabbitMQ C library dependency
find_package(Rabbitmq REQUIRED)
if (Rabbitmq_FOUND)
  set (Rabbitmq_INCLUDEDIR "/usr/local/include")
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (Rabbitmq_LIBDIR "/usr/local/lib64")
  else()
      set (Rabbitmq_LIBDIR "/usr/local/lib")
  endif()

  include_directories(SYSTEM ${Rabbitmq_INCLUDEDIR})
  link_directories(${Rabbitmq_LIBDIR})
else()
  message (FATAL_ERROR "Need rabbitmq-c library to continue build.")
  return()
endif()

# Boost dependency
set (BOOST_INCLUDEDIR "/usr/include")
set (BOOST_LIBRARYDIR "/usr/lib")
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (BOOST_ALL_DYN_LINK OFF)

find_package(Boost 1.54.0 REQUIRED COMPONENTS system)
if (Boost_FOUND)
  include_directories(SYSTEM ${BOOST_INCLUDEDIR})
  link_directories(${BOOST_LIBRARYDIR})
else()
  message (FATAL_ERROR "Need Boost library 1.54.0+ to continue build.")
  return()
endif()

set (INCLUDE_DIRS include)
set (SOURCES src/amqp11.cc src/amqp_base.cc src/amqp_exception.cc
            src/amqp_message.cc src/amqp_exchange.cc
            src/amqp_queue.cc)

include_directories(${INCLUDE_DIRS})
add_library(amqp11_d SHARED ${SOURCES})
add_library(amqp11_s STATIC ${SOURCES})

set_target_properties(amqp11_d PROPERTIES OUTPUT_NAME "amqp11")
set_target_properties(amqp11_d PROPERTIES VERSION 1.0 SOVERSION 1)

set_target_properties(amqp11_s PROPERTIES OUTPUT_NAME "amqp11")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

install(TARGETS amqp11_d amqp11_s
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)

install(FILES amqp11.hpp DESTINATION include/amqp11)
