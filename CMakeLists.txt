cmake_minimum_required (VERSION 2.8.11)
project(Zmey CXX)

# Zmey
set(ZmeyHeaders
    "include/Zmey/Allocator.h"
    "include/Zmey/Config.h"
    "include/Zmey/LogHandler.h"
    "Source/Memory/DefaultAllocator.h"
    "Source/Memory/LinearAllocator.h"
    "Source/Memory/STLAllocator.h"
)

set(ZmeySource
    "Source/EngineLoop.cpp"
    "Source/Logging.cpp"
    "Source/Memory/MemoryManagement.cpp"
)

add_library(Zmey ${ZmeySource})
target_include_directories(Zmey PUBLIC "include")

# Gift of the sanctum
set(GSHeaders "")

set(GSSource
    "Games/GiftOfTheSanctum/Source/main.cpp"
)

add_executable(GiftOfTheSanctum ${GSSource})
target_include_directories(GiftOfTheSanctum PUBLIC "include")
target_link_libraries(GiftOfTheSanctum Zmey)
