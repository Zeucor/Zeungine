# Add Zeungine library
add_library(zeungine ${ZG_LIBRARY_TYPE} ${ZG_SOURCES})
include_directories(include)
include_directories(${ZG_INC_INSTALL_PREFIX_ABS})
message(STATUS "ZG_LIBRARIES: ${ZG_LIBRARIES}")
target_link_libraries(zeungine ${ZG_LIBRARIES})
