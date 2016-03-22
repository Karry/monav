
include_directories(
  ${CMAKE_HOME_DIRECTORY}
)


set(lzma_SRCS

    LzFind.c
    LzmaDec.c
    LzmaEnc.c
)

add_library(lzma STATIC 
    ${lzma_SRCS}
)
