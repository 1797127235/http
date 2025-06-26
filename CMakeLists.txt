# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(HttpServer)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 根据编译器设置不同的编译选项
if(MSVC)
    # Windows Visual Studio 编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
    
    # 禁用某些MSVC警告
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
    
    # Windows特定设置
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    add_compile_definitions(NOMINMAX)
else()
    # Linux/GCC 编译选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
endif()

# 查找线程库
find_package(Threads REQUIRED)

# 创建HTTP服务器可执行文件
add_executable(http_server
    ServerMain.cc           # 主函数文件
    # 注意：其他文件都是.hpp头文件，不需要单独编译
)

# 设置头文件包含目录
target_include_directories(http_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 链接库
target_link_libraries(http_server 
    PRIVATE 
    Threads::Threads
)

# Windows特定库
if(WIN32)
    target_link_libraries(http_server 
        PRIVATE 
        ws2_32      # Windows Socket库
        wsock32     # Windows Socket库
    )
else()
    # Linux特定库
    target_link_libraries(http_server rt)
endif()

# 设置输出目录
set_target_properties(http_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 打印配置信息
message(STATUS "项目名称: HttpServer")
message(STATUS "主要源文件: ServerMain.cc") 
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "编译器: ${CMAKE_CXX_COMPILER_ID}") 