
# TODO: add support for NOGUI variant

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_STATICPLUGIN)

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/interfaces 
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)


qt5_generate_moc(chsettingsdialog.h              moc_chsettingsdialog.cpp)

#.ui files  - https://cmake.org/pipermail/cmake/2007-August/015860.html
set(osmimporter_UIS
    chsettingsdialog.ui
)

QT5_WRAP_UI( osmimporter_UIS_H ${osmimporter_UIS} )

set(osmimporter_SRCS

    moc_osmimporterclient.cpp
    osmimporterclient.cpp  

    moc_osmimporter.cpp
    osmimporter.cpp  

    moc_chsettingsdialog.cpp
)

add_library(osmimporter STATIC 
    ${osmimporter_SRCS}
    ${osmimporter_UIS_H}
)
target_link_libraries(osmimporter
    Qt5::Core 
)
