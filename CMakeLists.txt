cmake_minimum_required(VERSION 3.21)

include(FetchContent)

project(avr)

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

FetchContent_Declare(
  fftw
  URL      http://www.fftw.org/fftw-3.3.10.tar.gz
  URL_HASH MD5=8ccbf6a5ea78a16dbc3e1306e234cc5c
)

FetchContent_MakeAvailable(fftw)


add_executable(SerialPort src/main.cpp)
add_dependencies(SerialPort fftw3)  

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SerialPort)

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

target_include_directories(SerialPort PRIVATE               # добавление директорий с библиотеками в проект
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${fftw_SOURCE_DIR}/api)

target_link_libraries(SerialPort PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::PrintSupport
    Qt6::Widgets
    fftw3
)



if(WIN32)
    add_custom_command(
        TARGET SerialPort POST_BUILD                     
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:SerialPort> $<TARGET_FILE_DIR:SerialPort>
        COMMAND_EXPAND_LISTS
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${QT_ROOT}/plugins/platforms
            $<TARGET_FILE_DIR:SerialPort>/platforms
    ) 
endif()


