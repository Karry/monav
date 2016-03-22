
ADD_DEFINITIONS(${QT_DEFINITIONS})

include_directories(
  ${CMAKE_HOME_DIRECTORY}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}
)

qt5_generate_moc(directoryunpacker.h  moc_directoryunpacker.cpp)

set(directory_SRCS

    directorypacker.cpp

    directoryunpacker.cpp
    moc_directoryunpacker.cpp
)

add_library(directory STATIC 
    ${directory_SRCS}
)

target_link_libraries(directory
    lzma
    Qt5::Core 
)
