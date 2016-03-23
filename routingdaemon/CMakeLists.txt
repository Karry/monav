
file(GLOB DAEMON_PROTO_FILES "signals.proto")

protobuf_generate_cpp(DAEMON_PROTO_SOURCES DAEMON_PROTO_HEADERS ${DAEMON_PROTO_FILES})

include_directories(
  ${CMAKE_HOME_DIRECTORY} # ".."
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_HOME_DIRECTORY}/utils/qtservice-2.6_1-opensource/src/
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
)

qt5_generate_moc(routingdaemon.h      moc_routingdaemon.cpp)
qt5_generate_moc(routingserver.h      moc_routingserver.cpp)

# ==================================================================================================
# daemon

set(daemon_SRCS

    routingdaemon.cpp
    moc_routingdaemon.cpp

    ${DAEMON_PROTO_SOURCES}
)

add_executable (monav-daemon
		${daemon_SRCS}
)

target_link_libraries(monav-daemon
    contractionhierarchies
    gpsgrid
    lzma
    directory
    qtservice

    Qt5::Core 
    Qt5::Network
    Qt5::Widgets
    ${PROTOBUF_LIBRARIES}
)

# ==================================================================================================
# server

set(server_SRCS

    routingserver.cpp
    moc_routingserver.cpp

    ${DAEMON_PROTO_SOURCES}
)

add_executable (monav-server
		${server_SRCS}
)

target_link_libraries(monav-server
    contractionhierarchies
    gpsgrid
    lzma
    directory
    qtservice

    Qt5::Core 
    Qt5::Network
    Qt5::Widgets
    ${PROTOBUF_LIBRARIES}
)