
ADD_DEFINITIONS(-DMAPNIK_VERSION_MAJOR=${MAPNIK_VERSION_MAJOR})

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_STATICPLUGIN)
ADD_DEFINITIONS(${LIBMAPNIK_DEFINITIONS})

file(GLOB OSM_PROTO_FILES "${CMAKE_HOME_DIRECTORY}/utils/osm/fileformat.proto")
file(GLOB OSM_PROTO_FILES "${CMAKE_HOME_DIRECTORY}/utils/osm/osmformat.proto")

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
  ${LIBMAPNIK_INCLUDE_DIRS}
  ${LIBXML2_INCLUDE_DIR}
)

qt5_generate_moc(brsettingsdialog.h     moc_brsettingsdialog.cpp)
qt5_generate_moc(mapnikrendererclient.h moc_mapnikrendererclient.cpp)
qt5_generate_moc(mapnikrenderer.h       moc_mapnikrenderer.cpp)
qt5_generate_moc(mrsettingsdialog.h     moc_mrsettingsdialog.cpp)
qt5_generate_moc(orsettingsdialog.h     moc_orsettingsdialog.cpp)
qt5_generate_moc(osmrendererclient.h    moc_osmrendererclient.cpp)
qt5_generate_moc(osmrenderer.h          moc_osmrenderer.cpp)
qt5_generate_moc(osmrsettingsdialog.h   moc_osmrsettingsdialog.cpp)
qt5_generate_moc(qrsettingsdialog.h     moc_qrsettingsdialog.cpp)
qt5_generate_moc(qtilerendererclient.h  moc_qtilerendererclient.cpp)
qt5_generate_moc(qtilerenderer.h        moc_qtilerenderer.cpp)
qt5_generate_moc(rendererbase.h         moc_rendererbase.cpp)
qt5_generate_moc(tile-write.h           moc_tile-write.cpp)

#.ui files  - https://cmake.org/pipermail/cmake/2007-August/015860.html
set(osmrenderer_UIS
    brsettingsdialog.ui
    mrsettingsdialog.ui
    orsettingsdialog.ui
    osmrsettingsdialog.ui
    qrsettingsdialog.ui
)

QT5_WRAP_UI( osmrenderer_UIS_H ${osmrenderer_UIS} )

set(osmrenderer_SRCS
    brsettingsdialog.cpp
    mapnikrendererclient.cpp
    mapnikrenderer.cpp
    mrsettingsdialog.cpp
    orsettingsdialog.cpp
    osmrendererclient.cpp
    osmrenderer.cpp
    osmrsettingsdialog.cpp
    qrsettingsdialog.cpp
    qtilerendererclient.cpp
    qtilerenderer.cpp
    rendererbase.cpp
    tile-write.cpp

    moc_brsettingsdialog.cpp
    moc_mapnikrendererclient.cpp
    moc_mapnikrenderer.cpp
    moc_mrsettingsdialog.cpp
    moc_orsettingsdialog.cpp
    moc_osmrendererclient.cpp
    moc_osmrenderer.cpp
    moc_osmrsettingsdialog.cpp
    moc_qrsettingsdialog.cpp
    moc_qtilerendererclient.cpp
    moc_qtilerenderer.cpp
    moc_rendererbase.cpp
    moc_tile-write.cpp

)

add_library(osmrenderer STATIC 
    ${osmrenderer_SRCS}    
    ${OSM_PROTO_SOURCES}
    ${osmrenderer_UIS_H}
)
target_link_libraries(osmrenderer
    Qt5::Core 
    ${LIBMAPNIK_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)
