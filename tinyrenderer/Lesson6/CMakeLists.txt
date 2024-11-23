cmake_minimum_required(VERSION 3.10.1)

project(tinyrenderer)

set(HEADERS
    tgaimage.h
    model.h
    geometry.h
    camera.h
    tr_gl.h
)

set(SOURCES
    main.cpp
    model.cpp
    tgaimage.cpp
    geometry.cpp
    camera.cpp
    tr_gl.cpp
)

add_executable(tinyrenderer ${HEADERS} ${SOURCES})
