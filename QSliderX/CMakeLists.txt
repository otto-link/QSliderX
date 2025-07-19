project(qsliderx)

# Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_VERBOSE ON)

# --- sources GUI headers need to be added in add_executable, otherwise the moc
# won't parse them
file(GLOB_RECURSE QSX_GUI_INCLUDES
     ${CMAKE_CURRENT_SOURCE_DIR}/include/qsx/*.hpp)

file(GLOB_RECURSE QSX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${QSX_SOURCES} ${QSX_GUI_INCLUDES})

set(QSX_INCLUDE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE)

target_include_directories(${PROJECT_NAME} PRIVATE ${QSX_INCLUDE}
                                                   ${CMAKE_BINARY_DIR}/include)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog Qt6::Core
                                              Qt6::Widgets)
