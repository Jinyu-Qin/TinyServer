# 指定cmake的最小版本
cmake_minimum_required(VERSION 3.16)

# 设置项目名称
# 会生成TinyServer_SOURCE_DIR和TinyServer_BINARY_DIR两个变量（等价于PROJECT_SOURCE_DIR和PROJECT_BINARY_DIR）
project(TinyServer)
# 可执行文件的名称
set(BINARY_NAME tinyserver)

# 设置编译模式
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

# 设置C++语言标准
set(CMAKE_CXX_STANDARD 11)

# 设置可执行文件输出到bin目录
# CMAKE_SOURCE_DIR是CMakeLists.txt所在的目录（等价于PROJECT_SOURCE_DIR）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 设置编译宏选项
add_definitions(
    -D_GNU_SOURCE
)

# 设置包含的目录
include_directories(
    ${PROJECT_SOURCE_DIR}/app/inc
    ${PROJECT_SOURCE_DIR}/base/inc
    ${PROJECT_SOURCE_DIR}/net/inc
)

# 设置编译类型
aux_source_directory(${PROJECT_SOURCE_DIR}/app APP_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/base BASE_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/net NET_SOURCE)
add_executable(${BINARY_NAME} ${APP_SOURCE} ${BASE_SOURCE} ${NET_SOURCE})

# 设置链接库
target_link_libraries(${BINARY_NAME} glog)

# 设置编译和链接的一些flag
# 添加pthread支持
set_target_properties(${BINARY_NAME} PROPERTIES
    COMPILE_FLAGS "-pthread"
    LINK_FLAGS "-pthread"
)

# 打印信息
# message("PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
# message("Sources: " ${APP_SOURCE} ${BASE_SOURCE} ${NET_SOURCE})
