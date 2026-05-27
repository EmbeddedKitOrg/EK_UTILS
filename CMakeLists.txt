cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PICOLIBC "使用 picolibc 替代 nano.specs" OFF)
option(OBJ_LIB "编译为对象库而非静态库" OFF)

# 仅在独立构建时强制工具链和平台参数（作为子模块时由父项目控制）
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    if(NOT PLATFORM)
        message(FATAL_ERROR "PLATFORM is required. Usage: cmake -DPLATFORM=cm4 -B build\n"
                            "Valid values: cm0, cm3, cm4, cm7, cm23, cm33, cm55")
    endif()
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake)
endif()

project(ek_utils C)

# picolibc 全局链接标志调整
if(PICOLIBC AND CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    string(REPLACE "--specs=nano.specs" ""
        CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()

# ========== 源文件收集 ==========
file(GLOB EK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set(TP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tlsf/tlsf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lwprintf/lwprintf.c
)

set(EK_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tlsf
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/lwprintf/inc
)

if(PICOLIBC)
    list(APPEND EK_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/picolibc/include
    )
    set(PICOLIBC_LIB_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/picolibc/libs/cortexm/${PLATFORM}
    )
endif()

# ========== 库目标 ==========
if(OBJ_LIB)
    add_library(ek_utils OBJECT ${EK_SOURCES} ${TP_SOURCES})
else()
    add_library(ek_utils STATIC ${EK_SOURCES} ${TP_SOURCES})
endif()

target_include_directories(ek_utils PUBLIC ${EK_INCLUDE_DIRS})
target_compile_options(ek_utils PRIVATE -Os -g)

if(PICOLIBC AND NOT OBJ_LIB)
    target_link_directories(ek_utils PUBLIC ${PICOLIBC_LIB_DIR})
    target_link_libraries(ek_utils PUBLIC c nosys m)
endif()
