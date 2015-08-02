AMQP Client library for C++11
------

## Introduction
amqp11 is a small and simple AMQP client library for C++11 of the [RabbitMQ](http://www.rabbitmq.com)

## Latest Stable Version
The latest stable version of the amqp11 library can be found at:
- <https://github.com/AmrFaissal/amqp11/releases/latest>

## Getting started
### Building and Installing
#### Dependencies :
- [CMake v2.6 or better](http://www.cmake.org/)
- A C++11 compiler (GCC 4.8+, clang)
- [rabbitmq-c](https://github.com/alanxz/rabbitmq-c/releases/latest) library
- [Boost library](http://www.boost.org) version 1.54.0+

After downloading and extracting the source from a tarball to a directory.
([see above][Latest Stable Version]), the commands to build amqp11 on most systems are:

    mkdir build && cd build
    cmake ..
    cmake --build .

It is also possible to point the CMake GUI tool at the CMakeLists.txt in the root of
the source tree and generate build projects or IDE workspace.

Installing the library and optionally specifying a prefix can be done with:

    cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
    cmake --build . --target install

More information on CMake can be found on its [FAQ](http://www.cmake.org/Wiki/CMake_FAQ).

## Running the examples
Arrange for a RabbitMQ or other AMQP server to be running on `localhost` at TCP port number `5672`.

Build the examples:

    cd examples && mkdir build && cd build
    cmake ..
    cmake --build .

In one terminal, run:

    $./example_publish

In another terminal,

    $./example_consume

You should see output similar to the following in the listener's terminal window:

    Received data: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmodtempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum. |1: Tag=1 / Content-type:text/plain / Encoding:  / Mode=2

## Using `amqp11`

Please see the `examples` directory for short examples of the use of the `amqp11` library.
