
# TODO: add support for NOGUI variant

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/interfaces 
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${ICU_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)


qt5_generate_moc(aboutdialog.h          moc_aboutdialog.cpp)
qt5_generate_moc(pluginmanager.h        moc_pluginmanager.cpp)
qt5_generate_moc(preprocessingwindow.h  moc_preprocessingwindow.cpp)

#.ui files  - https://cmake.org/pipermail/cmake/2007-August/015860.html
set(preprocessor_UIS
    aboutdialog.ui
    preprocessingwindow.ui
)

QT5_WRAP_UI( preprocessor_UIS_H ${preprocessor_UIS} )

set(preprocessor_RCCS
    images.qrc
    speedprofiles.qrc
)
QT5_ADD_RESOURCES(preprocessor_RCC_SRCS ${preprocessor_RCCS})


set(preprocessor_SRCS
    aboutdialog.cpp
    pluginmanager.cpp
    preprocessingwindow.cpp

    moc_aboutdialog.cpp
    moc_pluginmanager.cpp
    moc_preprocessingwindow.cpp

    ${CMAKE_HOME_DIRECTORY}/utils/commandlineparser.cpp
)

add_library(preprocessor STATIC 
    ${preprocessor_SRCS}
    ${preprocessor_UIS_H}
    ${preprocessor_RCC_SRCS}
)
target_link_libraries(preprocessor
    directory
    osmrenderer
    osmimporter
    lzma
    contractionhierarchies
    gpsgrid
    unicodetournamenttrie
    log

    Qt5::Core 
    Qt5::Widgets

    ${BZIP2_LIBRARIES}
    ${LIBMAPNIK_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${ICU_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

# ==================================================================================================
# UI
set(preprocessor_ui_SRCS
    main.cpp
)
add_executable (monav-preprocessor
    ${preprocessor_ui_SRCS}
)
target_link_libraries(monav-preprocessor
    preprocessor
    directory
    osmrenderer
    osmimporter
    testimporter
    lzma
    contractionhierarchies
    gpsgrid
    unicodetournamenttrie
    log

    Qt5::Core 
    Qt5::Widgets

    ${BZIP2_LIBRARIES}
    ${LIBMAPNIK_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${ICU_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

# ==================================================================================================
# CLI
set(preprocessor_cli_SRCS
    console-main.cpp
)
add_executable (monav-preprocessor-cli
    ${preprocessor_cli_SRCS}
)
target_link_libraries(monav-preprocessor-cli
    preprocessor
    directory
    osmrenderer
    osmimporter
    testimporter
    lzma
    contractionhierarchies
    gpsgrid
    unicodetournamenttrie

    Qt5::Core 
    Qt5::Widgets

    ${BZIP2_LIBRARIES}
    ${LIBMAPNIK_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${ICU_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
