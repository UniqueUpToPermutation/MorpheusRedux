cmake_minimum_required (VERSION 3.6)

project(Embedded-Geo-Test CXX)

set(SOURCE
    main.cpp
)

set(SHADERS
)

set(ASSETS
)

add_engine_app("Embedded-Geo-Test" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
add_test(NAME Embedded-Geo-Test COMMAND Embedded-Geo-Test)
add_dependencies(MorpheusTests Embedded-Geo-Test)