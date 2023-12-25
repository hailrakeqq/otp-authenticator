cmake_minimum_required(VERSION 3.12)

project(otp-authenticator)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -g")
endif()

find_package(OpenCV REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(json)
include_directories(${CMAKE_SOURCE_DIR}/json)

set(SRC 
    src/crypt.cpp
    src/main.cpp
    src/settings.cpp)

add_executable(otp-authenticator ${SRC})

target_include_directories(otp-authenticator PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(otp-authenticator PRIVATE OpenSSL::Crypto)
target_link_libraries(otp-authenticator PRIVATE ${OpenCV_LIBRARIES})
target_link_libraries(otp-authenticator PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(otp-authenticator PRIVATE ncurses)