
# TODO: add support for NOGUI variant

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_STATICPLUGIN)
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

file(GLOB OSM_PROTO_FILES 
    "${CMAKE_HOME_DIRECTORY}/utils/osm/fileformat.proto"
    "${CMAKE_HOME_DIRECTORY}/utils/osm/osmformat.proto"
)

protobuf_generate_cpp(OSM_PROTO_SOURCES OSM_PROTO_HEADERS ${OSM_PROTO_FILES})

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/interfaces 
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${LIBXML2_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIRS}
)

qt5_generate_moc(highwaytypewidget.h      moc_highwaytypewidget.cpp)
qt5_generate_moc(nodemodificatorwidget.h  moc_nodemodificatorwidget.cpp)
qt5_generate_moc(oisettingsdialog.h       moc_oisettingsdialog.cpp)
qt5_generate_moc(osmimporter.h            moc_osmimporter.cpp)
qt5_generate_moc(speedprofiledialog.h     moc_speedprofiledialog.cpp)
qt5_generate_moc(waymodificatorwidget.h   moc_waymodificatorwidget.cpp)

set(osmimporter_UIS
    highwaytypewidget.ui
    nodemodificatorwidget.ui
    oisettingsdialog.ui
    speedprofiledialog.ui
    waymodificatorwidget.ui
)

QT5_WRAP_UI( osmimporter_UIS_H ${osmimporter_UIS} )

set(osmimporter_RCCS
    speedprofiles.qrc
)
QT5_ADD_RESOURCES(osmimporter_RCC_SRCS ${osmimporter_RCCS})


set(osmimporter_SRCS
    highwaytypewidget.cpp
    nodemodificatorwidget.cpp
    oisettingsdialog.cpp
    osmimporter.cpp
    speedprofiledialog.cpp
    types.cpp
    waymodificatorwidget.cpp

    moc_highwaytypewidget.cpp
    moc_nodemodificatorwidget.cpp
    moc_oisettingsdialog.cpp
    moc_osmimporter.cpp
    moc_speedprofiledialog.cpp
    moc_waymodificatorwidget.cpp
)

add_library(osmimporter STATIC 
    ${OSM_PROTO_SOURCES}
    ${osmimporter_SRCS}
    ${osmimporter_UIS_H}
    ${osmimporter_RCC_SRCS}
)
target_link_libraries(osmimporter
    Qt5::Core 
    Qt5::Widgets

    ${LIBXML2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)
