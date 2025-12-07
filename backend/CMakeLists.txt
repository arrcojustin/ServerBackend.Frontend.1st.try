cmake_minimum_required(VERSION 3.10)
project(BackendServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# multi-config 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release)

# g3log
set(G3LOG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/g3log/src")
file(GLOB G3LOG_SOURCES "${G3LOG_SRC_DIR}/*.cpp")

set(PLATFORM_LIBS "")

if(WIN32)
    list(FILTER G3LOG_SOURCES EXCLUDE REGEX ".*crashhandler_unix.cpp")
    set(PLATFORM_LIBS ws2_32)
elseif(UNIX)
    list(FILTER G3LOG_SOURCES EXCLUDE REGEX ".*crashhandler_windows.cpp")
    set(PLATFORM_LIBS pthread)
endif()

if(NOT G3LOG_SOURCES)
    message(WARNING "g3log source files not found in ${G3LOG_SRC_DIR}")
endif()

add_library(g3log_lib SHARED ${G3LOG_SOURCES})

target_include_directories(g3log_lib PUBLIC
    ${G3LOG_SRC_DIR}
)

target_link_libraries(g3log_lib PRIVATE
    ${PLATFORM_LIBS}
)

# 强制 MSVC 导出所有符号以生成 import lib（只在 Windows 下）
if(WIN32)
  set_target_properties(g3log_lib PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
  )
else()
  set_target_properties(g3log_lib PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  )
endif()

add_executable(${PROJECT_NAME} src/main.cpp)

# 保证库先构建
add_dependencies(${PROJECT_NAME} g3log_lib)

 if (UNIX)   
    target_link_libraries(${PROJECT_NAME} PRIVATE
        g3log_lib
        "ws2_32" 
        "m"
        "pthread"
        ${PLATFORM_LIBS}
    )
 else()
    target_link_libraries(${PROJECT_NAME} PRIVATE
        g3log_lib
        "ws2_32" 
        ${PLATFORM_LIBS}
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${G3LOG_SRC_DIR}
)