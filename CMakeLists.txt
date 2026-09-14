cmake_minimum_required(VERSION 3.25)
project(SleapyCamera LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SleapyCamera WIN32 app/main.cpp)

target_compile_definitions(SleapyCamera PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(SleapyCamera PRIVATE
    d3d11
    dxgi
    d3dcompiler
    user32
    gdi32
    shell32
    ole32
    mf
    mfplat
    mfreadwrite
    mfuuid
)

if(MSVC)
    target_compile_options(SleapyCamera PRIVATE /W4 /permissive-)
endif()
