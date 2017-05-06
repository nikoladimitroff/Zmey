cmake_minimum_required (VERSION 2.8.11)
project(Zmey CXX)

# Zmey
set(ZmeyHeaders
    "include/Zmey/Config.h"
    "include/Zmey/EngineLoop.h"
    "include/Zmey/LogHandler.h"
    "include/Zmey/Logging.h"
    "include/Zmey/Memory/Allocator.h"
    "include/Zmey/Memory/LinearAllocator.h"
    "include/Zmey/Memory/MemoryManagement.h"
    "include/Zmey/Memory/STLAllocator.h"
    "include/Zmey/Tasks/BlockingQueue.h"
    "include/Zmey/Tasks/TaskSystem.h"
)

set(ZmeySource
    "Source/EngineLoop.cpp"
    "Source/Logging.cpp"
    "Source/Memory/MemoryManagement.cpp"
)

add_library(Zmey ${ZmeyHeaders} ${ZmeySource})
target_include_directories(Zmey PUBLIC "include")

# Gift of the sanctum
set(GSHeaders "")

set(GSSource
    "Games/GiftOfTheSanctum/Source/main.cpp"
)

add_executable(GiftOfTheSanctum ${GSSource})
target_include_directories(GiftOfTheSanctum PUBLIC "include")
target_link_libraries(GiftOfTheSanctum Zmey)
