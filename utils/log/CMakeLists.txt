
# TODO: add support for NOGUI variant

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${CMAKE_HOME_DIRECTORY}/interfaces 
  ${CMAKE_HOME_DIRECTORY}/utils
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

qt5_generate_moc(log.h          moc_log.cpp)
qt5_generate_moc(logwindow.h    moc_logwindow.cpp)

set(log_UIS
    logwindow.ui
)

QT5_WRAP_UI( log_UIS_H ${log_UIS} )


set(log_SRCS
    log.cpp
    logwindow.cpp

    moc_log.cpp
    moc_logwindow.cpp
)

add_library(log STATIC 
    ${log_SRCS}
    ${log_UIS_H}
)
