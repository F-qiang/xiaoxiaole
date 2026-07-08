cmake_minimum_required(VERSION 3.20)

project(xiaoxiaole)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED COCOS2DX_ROOT)
    message(FATAL_ERROR "请通过 -DCOCOS2DX_ROOT 指定 Cocos2d-x 4.0 根目录，例如 D:/App/cocos2d-x-4.0")
endif()

set(COCOS2DX_ROOT_PATH ${COCOS2DX_ROOT})
set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)
include(CocosBuildSet)

add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)

set(GAME_SOURCE
    Classes/AppDelegate.cpp
    Classes/BoardRenderer.cpp
    Classes/GameBoard.cpp
    Classes/GameScene.cpp
    proj.win32/main.cpp
)

set(GAME_HEADER
    Classes/AppDelegate.h
    Classes/BoardRenderer.h
    Classes/GameBoard.h
    Classes/GameScene.h
    proj.win32/main.h
)

set(GAME_RES_FOLDER
    "${CMAKE_CURRENT_SOURCE_DIR}/resourses"
)

set(_existing_res_folders)
foreach(_dir ${GAME_RES_FOLDER})
    if(EXISTS "${_dir}")
        list(APPEND _existing_res_folders "${_dir}")
    endif()
endforeach()

if(APPLE OR WINDOWS)
    if(_existing_res_folders)
        cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${_existing_res_folders})
    endif()
endif()

set(all_code_files
    ${GAME_HEADER}
    ${GAME_SOURCE}
)

if(NOT ANDROID)
    add_executable(${PROJECT_NAME} ${all_code_files})
else()
    add_library(${PROJECT_NAME} SHARED ${all_code_files})
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
    target_link_libraries(${PROJECT_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

target_link_libraries(${PROJECT_NAME} cocos2d)

target_include_directories(${PROJECT_NAME}
    PRIVATE Classes
    PRIVATE proj.win32
    PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
)

setup_cocos_app_config(${PROJECT_NAME})

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

if(WINDOWS)
    cocos_copy_target_dll(${PROJECT_NAME})
endif()

if(LINUX OR WINDOWS)
    if(_existing_res_folders)
        cocos_get_resource_path(APP_RES_DIR ${PROJECT_NAME})
        cocos_copy_target_res(${PROJECT_NAME} LINK_TO ${APP_RES_DIR} FOLDERS ${_existing_res_folders})
    endif()
endif()
