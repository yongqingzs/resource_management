cmake_minimum_required(VERSION 3.10)
project(resource_management VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++11 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置构建类型，如果未指定则默认为 Debug
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
  # 为MSVC编译器添加UTF-8编码支持
  add_compile_options(/utf-8)

  # 警告级别和错误处理
  add_compile_options(/W4)       # 设置警告级别为4 (类似于-Wall -Wextra)
  add_compile_options(/WX)       # 将警告视为错误 (类似于-Werror)
  add_compile_options(/permissive-) # 增强的标准符合性
  
  # 调试信息设置
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /RTC1") # 完整调试信息
  
  # 优化设置
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /GL") # 最大优化
  
  # 运行时库
  # 多线程动态库 (/MD 和 /MDd) 是默认值，可以明确设置
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

# 设置编译选项
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(LIB_SOURCES
  src/resource_indexer.cpp
  src/resource_node.cpp
  src/resource_registry.cpp
)

# 创建主可执行文件
add_executable(test_ResourceNode test/test_ResourceNode.cpp ${LIB_SOURCES})

add_executable(test_ResourceRegistry test/test_ResourceRegistry.cpp ${LIB_SOURCES})

add_executable(test_ResourceIndexer test/test_ResourceIndexer.cpp ${LIB_SOURCES})

add_executable(test_Indexed test/test_Indexed.cpp ${LIB_SOURCES})

add_executable(test_Struct test/test_Struct.cpp ${LIB_SOURCES})