list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

cmake_minimum_required(VERSION 3.10)
project(librxui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

function(configure target)
    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
        target_compile_options(${target} PRIVATE -Werror)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        target_compile_features(${target} PRIVATE cxx_std_11)
    endif ()
endfunction()

add_library(rxui
        src/rxui.cpp src/rxui.h
        )
configure(rxui)

add_library(rxui-efl
        src/rxui-efl.cpp src/rxui-efl.h
        src/rxui-widgets-gtk.h
        )
configure(rxui-efl)
target_compile_definitions(rxui-efl PUBLIC RXUI_BACKEND_EFL)
find_package(Elementary REQUIRED)
if (ELEMENTARY_FOUND AND NOT TARGET Elementary::efl)
    add_library(Elementary::efl INTERFACE IMPORTED)
    set_target_properties(Elementary::efl PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES
            "${ELEMENTARY_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES
            "${ELEMENTARY_LIBRARIES}"
            INTERFACE_COMPILE_DEFINITIONS
            "${ELEMENTARY_DEFINITIONS_STR}"
            )
endif ()
target_link_libraries(rxui-efl
        PUBLIC
        rxui
        Elementary::efl
        )

add_library(rxui-gtk
        src/rxui-gtk.cpp src/rxui-gtk.h
        src/rxui-widgets-gtk.h
        )
configure(rxui-gtk)
target_compile_definitions(rxui-gtk PUBLIC RXUI_BACKEND_GTK)
find_package(GTK3 REQUIRED)
target_link_libraries(rxui-gtk
        PUBLIC
        rxui
        GTK3::gtk
        )

add_library(rxui-qt
        src/rxui-qt.cpp src/rxui-qt.h
        src/rxui-widgets-gtk.h
        )
configure(rxui-qt)
target_compile_definitions(rxui-qt PUBLIC RXUI_BACKEND_QT)
find_package(Qt5 REQUIRED COMPONENTS Widgets)
target_link_libraries(rxui-qt
        PUBLIC
        rxui
        Qt5::Widgets
        )

function(test name files)
    add_executable(${name}-efl test/7guis/00-common.h test/7guis/00-common-efl.h ${files})
    configure(${name}-efl)
    target_link_libraries(${name}-efl rxui-efl)

    add_executable(${name}-gtk test/7guis/00-common.h test/7guis/00-common-gtk.h ${files})
    configure(${name}-gtk)
    target_link_libraries(${name}-gtk rxui-gtk)

    add_executable(${name}-qt test/7guis/00-common.h test/7guis/00-common-qt.h ${files})
    configure(${name}-qt)
    target_link_libraries(${name}-qt rxui-qt)
endfunction()

test(rxui-7guis-01-counter test/7guis/01-counter.cpp)
test(rxui-7guis-02-tempconv test/7guis/02-tempconv.cpp)
test(rxui-7guis-03-flight test/7guis/03-flight.cpp)
#test(rxui-7guis-04-timer test/7guis/04-timer.cpp)
test(rxui-7guis-05-crud test/7guis/05-crud.cpp)
#test(rxui-7guis-06-circles test/7guis/06-circles.cpp)
#test(rxui-7guis-07-cells. test/7guis/07-cells.cpp)
