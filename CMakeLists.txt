cmake_minimum_required(VERSION 3.0.0)
project(IEW VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_definitions(-DLIBUS_NO_SSL)
add_definitions(-DWITH_LIBDEFLATE=1)
# add_definitions(-DELPP_DEBUG_ERRORS)
add_definitions(-DELPP_NO_DEFAULT_LOG_FILE)

include_directories("src/")
include_directories("src/Application/")
include_directories("External/uWebSockets/src")
include_directories("External/uWebSockets/uSockets/src")

include(FetchContent)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.9.1)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
  FetchContent_Populate(json)
   add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(easyloggingpp
  GIT_REPOSITORY https://github.com/amrayn/easyloggingpp.git
  GIT_TAG v9.97.0)

FetchContent_GetProperties(easyloggingpp)
if(NOT easyloggingpp_POPULATED)
  	FetchContent_Populate(easyloggingpp)
	add_subdirectory(${easyloggingpp_SOURCE_DIR} ${easyloggingpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

include_directories("${easyloggingpp_SOURCE_DIR}/src")

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE USOCKETS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/External/uWebSockets/uSockets/src/*.c)
file(GLOB_RECURSE LOGGING LIST_DIRECTORIES true ${easyloggingpp_SOURCE_DIR}/src/*.cc)

add_executable(IEW ${SOURCES} ${USOCKETS} ${LOGGING})

target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json)

SET(LIBPATH /usr/lib /usr/local/lib /lib/x86_64-linux-gnu) 

find_library(Z_LIB NAMES z PATHS ${LIBPATH})
if(Z_LIB)
	message(STATUS "z found at ${Z_LIB}")
else(Z_LIB)
	message( FATAL_ERROR "z not found. Please install z")
endif(Z_LIB)

target_link_libraries(${PROJECT_NAME} ${Z_LIB})

find_library(DEFLATE_LIB NAMES deflate PATHS ${LIBPATH})
if(DEFLATE_LIB)
	message(STATUS "libdeflate found at ${DEFLATE_LIB}")
else(DEFLATE_LIB)
	message( FATAL_ERROR "libdeflate not found. Please install libdeflate")
endif(DEFLATE_LIB)

target_link_libraries(${PROJECT_NAME} ${DEFLATE_LIB})

find_library(FCGI_LIB NAMES fcgi PATHS ${LIBPATH})
if(FCGI_LIB)
	message(STATUS "fcgi found at ${FCGI_LIB}. fcgi support is compiled in.")
else(FCGI_LIB)
	message( FATAL_ERROR "fcgi not found. Please install fcgi")
endif(FCGI_LIB)

target_link_libraries(${PROJECT_NAME} ${FCGI_LIB})

find_library(CURL_LIB NAMES curl PATHS ${LIBPATH})
if(CURL_LIB)
	message(STATUS "curl found at ${CURL_LIB}. curl support is compiled in.")
else(CURL_LIB)
	message( FATAL_ERROR "curl not found. Please install curl")
endif(CURL_LIB)

target_link_libraries(${PROJECT_NAME} ${CURL_LIB})

find_library(MYSQL_LIB NAMES mysqlcppconn8 PATHS ${LIBPATH})
if(MYSQL_LIB)
	message(STATUS "MySQL Connector 8.0 found at ${MYSQL_LIB}")
else(MYSQL_LIB)
	message( FATAL_ERROR "MySQL Connector 8.0 not found. Please install MySQL Connector 8.0")
endif(MYSQL_LIB)

target_link_libraries(${PROJECT_NAME} ${MYSQL_LIB})

include_directories("/usr/include/mysql-cppconn-8/mysqlx/")

find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
