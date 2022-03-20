cmake_minimum_required(VERSION 3.14)

project(avr)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Build type")

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

# Organisation of Visual Studio solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER AutoMoc)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP AutoMoc)
set(CMAKE_UtilityFolderName "CMakeUtilityTargets")
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${CMAKE_UtilityFolderName})

# ----------------------------------------------------------------------------------------------------------------------
# User defined variables

set(QT_ROOT CACHE PATH "Qt library path")

if (NOT QT_ROOT)
message(FATAL_ERROR "You need to provide the correct path to all specified libraries and sources."
        "It is assumed that libraries are"
        "compartable with CMake, and `find_package(<Path>)` can correctly process configuration."     
        "`<Lib>Config.cmake` must be under that path."
        "Sources must have CMakeLists.txt"
)
endif()

# ----------------------------------------------------------------------------------------------------------------------
# QT #
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH ${QT_ROOT})

find_package(Qt6
    COMPONENTS 
        Widgets 
        Core
        Gui
        PrintSupport
    REQUIRED
)

add_executable(SerialPort src/main.cpp)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SerialPort)

target_compile_definitions(SerialPort PRIVATE 
    _USE_MATH_DEFINES
    _UNICODE
    _NOMINMAX
    NO_OPTIX
    $<$<CONFIG:Debug>:_DEBUG>
)

target_sources(SerialPort PRIVATE
    src/BaseDevice.cpp
    src/qprange.cpp
    src/SerialPort.cpp
    
    src/BaseDevice.h
    src/qprange.h
    src/SerialPort.h
    src/crosslib.h
)

get_target_property(SerialPort_SOURCES SerialPort SOURCES)
source_group("Source" FILES ${SerialPort_SOURCES})

target_link_libraries(SerialPort PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::PrintSupport
    Qt6::Widgets
)

if(WIN32)
    add_custom_command(
        TARGET SerialPort POST_BUILD                     
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt6::Core>
            $<TARGET_FILE:Qt6::Gui>
            $<TARGET_FILE:Qt6::PrintSupport>
            $<TARGET_FILE:Qt6::Widgets>
            $<TARGET_FILE_DIR:SerialPort>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${QT_ROOT}/plugins/platforms
            $<TARGET_FILE_DIR:SerialPort>/platforms
    ) 
endif()


