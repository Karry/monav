
# TODO: add support for NOGUI variant

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_STATICPLUGIN)

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/interfaces 
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

qt5_generate_moc(ggdialog.h      moc_ggdialog.cpp)
qt5_generate_moc(gpsgridclient.h moc_gpsgridclient.cpp)
qt5_generate_moc(gpsgrid.h       moc_gpsgrid.cpp)

#.ui files  - https://cmake.org/pipermail/cmake/2007-August/015860.html
set(gpsgrid_UIS
    ggdialog.ui
)

QT5_WRAP_UI( gpsgrid_UIS_H ${gpsgrid_UIS} )

set(gpsgrid_SRCS

    ggdialog.cpp
    moc_ggdialog.cpp

    gpsgridclient.cpp
    moc_gpsgridclient.cpp

    gpsgrid.cpp
    moc_gpsgrid.cpp
)

add_library(gpsgrid STATIC 
    ${gpsgrid_SRCS}
    ${gpsgrid_UIS_H}
)
target_link_libraries(gpsgrid
    Qt5::Core 
)
