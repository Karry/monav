
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_STATICPLUGIN)

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/interfaces 
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
)

qt5_generate_moc(testimporter.h  moc_testimporter.cpp)

set(testimporter_SRCS
    testimporter.cpp
    moc_testimporter.cpp
)

add_library(testimporter STATIC 
    ${testimporter_SRCS}
)

target_link_libraries(testimporter
    Qt5::Core 
)
