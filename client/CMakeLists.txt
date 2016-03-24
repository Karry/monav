
include_directories(
  ${CMAKE_HOME_DIRECTORY} # ".."
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Xml_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
)

if (Qt5Positioning_FOUND)
  include_directories(${Qt5Positioning_INCLUDE_DIRS})
else()
  ADD_DEFINITIONS(-DNOQTMOBILE) # TODO: check it
endif()

qt5_generate_moc(addressdialog.h            moc_addressdialog.cpp)
qt5_generate_moc(bookmarksdialog.h          moc_bookmarksdialog.cpp)
qt5_generate_moc(fullscreenexitbutton.h     moc_fullscreenexitbutton.cpp)
qt5_generate_moc(generalsettingsdialog.h    moc_generalsettingsdialog.cpp)
qt5_generate_moc(gpsdialog.h                moc_gpsdialog.cpp)
qt5_generate_moc(gpsdpositioninfosource.h   moc_gpsdpositioninfosource.cpp)
qt5_generate_moc(logger.h                   moc_logger.cpp)
qt5_generate_moc(mainwindow.h               moc_mainwindow.cpp)
qt5_generate_moc(mapdata.h                  moc_mapdata.cpp)
qt5_generate_moc(mapmoduleswidget.h         moc_mapmoduleswidget.cpp)
qt5_generate_moc(mappackageswidget.h        moc_mappackageswidget.cpp)
qt5_generate_moc(overlaywidget.h            moc_overlaywidget.cpp)
qt5_generate_moc(paintwidget.h              moc_paintwidget.cpp)
qt5_generate_moc(placechooser.h             moc_placechooser.cpp)
qt5_generate_moc(routedescriptiondialog.h   moc_routedescriptiondialog.cpp)
qt5_generate_moc(routinglogic.h             moc_routinglogic.cpp)
qt5_generate_moc(scrollarea.h               moc_scrollarea.cpp)
qt5_generate_moc(serverinputdialog.h        moc_serverinputdialog.cpp)
qt5_generate_moc(serverlogic.h              moc_serverlogic.cpp)
qt5_generate_moc(streetchooser.h            moc_streetchooser.cpp)
qt5_generate_moc(worldmapchooser.h          moc_worldmapchooser.cpp)

set(client_UIS

    addressdialog.ui
    bookmarksdialog.ui
    generalsettingsdialog.ui
    gpsdialog.ui
    mainwindow.ui
    mapmoduleswidget.ui
    mappackageswidget.ui
    paintwidget.ui
    placechooser.ui
    routedescriptiondialog.ui
    serverinputdialog.ui
    streetchooser.ui
)

QT5_WRAP_UI( client_UIS_H ${client_UIS} )

set(client_RCCS
    images.qrc
)
QT5_ADD_RESOURCES(client_RCC_SRCS ${client_RCCS})

set(client_SRCS

    addressdialog.cpp
    bookmarksdialog.cpp
    generalsettingsdialog.cpp
    globalsettings.cpp
    gpsdialog.cpp
    gpsdpositioninfosource.cpp
    json.cpp
    logger.cpp
    main.cpp
    mainwindow.cpp
    mapdata.cpp
    mapmoduleswidget.cpp
    mappackageswidget.cpp
    overlaywidget.cpp
    paintwidget.cpp
    placechooser.cpp
    routedescriptiondialog.cpp
    routinglogic.cpp
    scrollarea.cpp
    serverinputdialog.cpp
    serverlogic.cpp
    streetchooser.cpp
    worldmapchooser.cpp

    moc_addressdialog.cpp
    moc_bookmarksdialog.cpp
    moc_fullscreenexitbutton.cpp
    moc_generalsettingsdialog.cpp
    moc_gpsdialog.cpp
    moc_gpsdpositioninfosource.cpp
    moc_json.cpp
    moc_logger.cpp
    moc_mainwindow.cpp
    moc_mapdata.cpp
    moc_mapmoduleswidget.cpp
    moc_mappackageswidget.cpp
    moc_overlaywidget.cpp
    moc_paintwidget.cpp
    moc_placechooser.cpp
    moc_routedescriptiondialog.cpp
    moc_routinglogic.cpp
    moc_scrollarea.cpp
    moc_serverinputdialog.cpp
    moc_serverlogic.cpp
    moc_streetchooser.cpp
    moc_worldmapchooser.cpp

)

add_executable (monav-client
		${client_SRCS}
    ${client_UIS_H}
    ${client_RCC_SRCS}
)

target_link_libraries(monav-client
    directory
    contractionhierarchies
    gpsgrid
    unicodetournamenttrie
    osmrenderer

    Qt5::Core 
    Qt5::Network
    Qt5::Widgets
    Qt5::Xml
    Qt5::Svg

    ${LIBMAPNIK_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)
if (Qt5Positioning_FOUND)
    target_link_libraries(monav-client Qt5::Positioning)
endif()

