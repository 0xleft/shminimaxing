cmake_minimum_required(VERSION 3.30)
project(shminimaxing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release) # todo
add_library(shminimaxing SHARED src/library.cpp
                                src/game.cpp
                                src/search.cpp
                                src/saved_states.cpp
)

add_executable(tests src/game.cpp
                     src/tests.cpp
                    src/search.cpp
                     src/saved_states.cpp
)

if(WIN32)
    set_target_properties(
            shminimaxing
            PROPERTIES
            OUTPUT_NAME "shminimaxing"
            PREFIX ""
    )
elseif(UNIX AND CMAKE_SYSTEM_NAME MATCHES "Linux")
    set_target_properties(
            shminimaxing
            PROPERTIES
            OUTPUT_NAME "shminimaxing"
            PREFIX "lib"
    )
endif()



enable_testing()

add_test(NAME tests COMMAND tests)

find_package(Java REQUIRED)
find_package(JNI REQUIRED)
target_link_libraries(shminimaxing PUBLIC JNI::JNI)
target_include_directories(shminimaxing PUBLIC ${JNI_INCLUDE_DIRS})