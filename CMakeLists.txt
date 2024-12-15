cmake_minimum_required(VERSION 3.30)
project(AbstractNexus VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor/fenster)
include_directories(vendor/fenster/include)
include_directories(include)

add_library(abstractnexus
        STATIC
        src/IEntity.cpp
        src/IScene.cpp
        src/IGame.cpp
        src/modules/fenster/Fenster.cpp
)

target_link_libraries(abstractnexus fenster)

function(create_test TEST_NAME TEST_SOURCE TEST_ARGS)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} abstractnexus)
    if(UNIX AND NOT APPLE)
        target_link_libraries(${TEST_NAME} ${X11_LIBRARIES})
    endif()
    add_test(NAME ${TEST_NAME} COMMAND $<TARGET_FILE_DIR:${TEST_NAME}>/${TEST_NAME}${TEST_EXT} ${TEST_ARGS})
endfunction()

include(CTest)
enable_testing()
create_test(FensterWindow tests/FensterWindow.cpp "")