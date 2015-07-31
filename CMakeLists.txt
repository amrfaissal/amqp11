cmake_minimum_required(VERSION 2.8.4)

project(amqp11)

add_subdirectory(src)

set(SOURCES src/AMQP.cpp src/AMQPBase.cpp src/AMQPException.cpp src/AMQPMessage.cpp src/AMQPExchange.cpp src/AMQPQueue.cpp)

