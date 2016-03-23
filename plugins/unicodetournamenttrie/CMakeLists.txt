
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
  ${Qt5Widgets_INCLUDE_DIRS}
)

qt5_generate_moc(unicodetournamenttrieclient.h  moc_unicodetournamenttrieclient.cpp)
qt5_generate_moc(unicodetournamenttrie.h        moc_unicodetournamenttrie.cpp)
qt5_generate_moc(uttsettingsdialog.h            moc_uttsettingsdialog.cpp)

set(unicodetournamenttrie_UIS
    uttsettingsdialog.ui
)

QT5_WRAP_UI( unicodetournamenttrie_UIS_H ${unicodetournamenttrie_UIS} )

set(unicodetournamenttrie_SRCS
    unicodetournamenttrieclient.cpp
    unicodetournamenttrie.cpp
    uttsettingsdialog.cpp

    moc_unicodetournamenttrieclient.cpp
    moc_unicodetournamenttrie.cpp
    moc_uttsettingsdialog.cpp
)

add_library(unicodetournamenttrie STATIC 
    ${unicodetournamenttrie_SRCS}
    ${unicodetournamenttrie_UIS_H}
)
target_link_libraries(unicodetournamenttrie
    Qt5::Core 
    Qt5::Widgets
)
