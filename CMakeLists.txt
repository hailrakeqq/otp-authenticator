cmake_minimum_required(VERSION 3.12)

project(otp-authenticator)

find_package(OpenCV REQUIRED)
find_package(OpenSSL REQUIRED)

add_subdirectory(json)
include_directories(${CMAKE_SOURCE_DIR}/json)

add_executable(otp-authenticator src/main.cpp)

target_include_directories(otp-authenticator PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(otp-authenticator PRIVATE OpenSSL::Crypto)
target_link_libraries(otp-authenticator PRIVATE ${OpenCV_LIBRARIES})
target_link_libraries(otp-authenticator PRIVATE nlohmann_json::nlohmann_json)