cmake_minimum_required(VERSION 3.5)
project(resource_management VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++11 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型，如果未指定则默认为 Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
  src/main.cpp
  src/resource_indexer.cpp
  src/resource_node.cpp
  src/resource_registry.cpp
)

set(LIB_SOURCES
  src/resource_indexer.cpp
  src/resource_node.cpp
  src/resource_registry.cpp
)

# 创建主可执行文件
add_executable(resource_demo ${SOURCES})

add_executable(test_ResourceNode test/test_ResourceNode.cpp ${LIB_SOURCES})

add_executable(test_ResourceRegistry test/test_ResourceRegistry.cpp ${LIB_SOURCES})

add_executable(test_ResourceIndexer test/test_ResourceIndexer.cpp ${LIB_SOURCES})

add_executable(test_Indexed test/test_Indexed.cpp ${LIB_SOURCES})