# TODO: explore qtservice situation, it is merget to Qt already?
# TODO: add support for windows

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} # path with generated files
    ${Qt5Core_INCLUDE_DIRS}  
)

qt5_generate_moc(src/qtservice.cpp        qtservice.moc)
qt5_generate_moc(src/qtservice_unix.cpp   qtservice_unix.moc)
qt5_generate_moc(src/qtunixserversocket.h moc_qtunixserversocket.cpp)
qt5_generate_moc(src/qtunixsocket.h       moc_qtunixsocket.cpp)

set(qtservice_SRCS

    src/qtservice.cpp
    qtservice.moc

    src/qtservice_unix.cpp
    qtservice_unix.moc

    #src/qtservice_win.cpp

    src/qtunixserversocket.cpp
    moc_qtunixserversocket.cpp

    src/qtunixsocket.cpp
    moc_qtunixsocket.cpp
)

add_library(qtservice STATIC 
    ${qtservice_SRCS}
)
target_link_libraries(qtservice
    Qt5::Core 
    Qt5::Network
)
