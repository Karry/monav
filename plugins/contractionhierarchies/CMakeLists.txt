
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

qt5_generate_moc(contractionhierarchiesclient.h  moc_contractionhierarchiesclient.cpp)
qt5_generate_moc(contractionhierarchies.h        moc_contractionhierarchies.cpp)
qt5_generate_moc(chsettingsdialog.h              moc_chsettingsdialog.cpp)

#.ui files  - https://cmake.org/pipermail/cmake/2007-August/015860.html
set(contractionhierarchies_UIS
    chsettingsdialog.ui
)

QT5_WRAP_UI( contractionhierarchies_UIS_H ${contractionhierarchies_UIS} )

set(contractionhierarchies_SRCS

    moc_contractionhierarchiesclient.cpp
    contractionhierarchiesclient.cpp  

    moc_contractionhierarchies.cpp
    contractionhierarchies.cpp  

    moc_chsettingsdialog.cpp
)

add_library(contractionhierarchies STATIC 
    ${contractionhierarchies_SRCS}
    ${contractionhierarchies_UIS_H}
)
target_link_libraries(contractionhierarchies
    Qt5::Core 
)
