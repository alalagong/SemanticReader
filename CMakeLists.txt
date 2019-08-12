cmake_minimum_required(VERSION 3.5.1)
project(read_yaml)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
find_package(YAML-CPP REQUIRED)

include_directories(
    ${PROJECT_SOURCE_DIR}/include 
    ${YAML_CPP_INCLUDE_DIR}
    ThirdParty/CocCoMaskAPI/include
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(read_yaml src/read_semantic.cpp)
target_link_libraries(read_yaml ${OpenCV_LIBS} ${YAML_CPP_LIBRARIES} ${PROJECT_SOURCE_DIR}/ThirdParty/CocCoMaskAPI/lib/libCoCoMaskAPI.so)