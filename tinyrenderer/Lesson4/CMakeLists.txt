cmake_minimum_required(VERSION 3.10.1)

project(tinyrenderer)

set(HEADERS
    tgaimage.h
    model.h
    geometry.h
)

set(SOURCES
    main.cpp
    model.cpp
    tgaimage.cpp
    geometry.cpp
)

add_executable(tinyrenderer ${HEADERS} ${SOURCES})
