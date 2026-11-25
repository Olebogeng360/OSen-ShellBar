cmake_minimum_required(VERSION 3.25)
project(CustomShellBar VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(
    CustomShellBar
    WIN32
    src/main.cpp
    src/ShellSafety.cpp
    src/AppBarManager.cpp
    src/WindowTracker.cpp
)
target_include_directories(CustomShellBar PRIVATE src)
target_compile_definitions(CustomShellBar PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(CustomShellBar PRIVATE /W4 /permissive- /EHsc /utf-8)
target_link_libraries(
    CustomShellBar
    PRIVATE
    dwmapi
    shell32
    psapi
)
add_executable(CustomShellBarWatchdog WIN32 watchdog/main.cpp)
target_compile_definitions(CustomShellBarWatchdog PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_options(CustomShellBarWatchdog PRIVATE /W4 /permissive- /EHsc /utf-8)
target_link_libraries(CustomShellBarWatchdog PRIVATE shell32)
install(TARGETS CustomShellBar CustomShellBarWatchdog RUNTIME DESTINATION .)
