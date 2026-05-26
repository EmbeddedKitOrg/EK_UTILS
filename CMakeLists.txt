cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 平台必填，不设默认值
if(NOT PLATFORM)
    message(FATAL_ERROR "PLATFORM is required. Usage: cmake -DPLATFORM=cm4 -B build\n"
                        "Valid values: cm0, cm3, cm4, cm7, cm23, cm33, cm55")
endif()

option(PICOLIBC "使用 picolibc 替代 nano.specs" OFF)
option(OBJ_LIB "编译为对象库而非静态库" OFF)

# 加载交叉编译工具链
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)

project(ek_utils C)

# ========== 源文件收集 ==========
file(GLOB EK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# 第三方源文件（始终编译，#if 守卫控制实际使用）
set(TP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tlsf/tlsf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lwprintf/lwprintf.c
)


set(EK_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tlsf
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lwprintf/inc
)

# picolibc 头文件和库路径
if(PICOLIBC)
    list(APPEND EK_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/picolibc/include
    )
    set(PICOLIBC_LIB_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/picolibc/libs/cortexm/${PLATFORM}
    )
    # 去掉 --specs=nano.specs，改用 picolibc
    string(REPLACE "--specs=nano.specs" ""
        CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

if(OBJ_LIB)
    add_library(ek_utils OBJECT ${EK_SOURCES} ${TP_SOURCES})
else()
    add_library(ek_utils STATIC ${EK_SOURCES} ${TP_SOURCES})
endif()

target_include_directories(ek_utils PUBLIC ${EK_INCLUDE_DIRS})

# 编译优化
target_compile_options(ek_utils PRIVATE
    -Os
    -g
)

# picolibc 预编译库链接（仅静态库模式）
if(PICOLIBC AND NOT OBJ_LIB)
    target_link_directories(ek_utils PUBLIC ${PICOLIBC_LIB_DIR})
    target_link_libraries(ek_utils PUBLIC c nosys m)
endif()
