cmake_minimum_required(VERSION 3.10)
project(LOB_Simulator)

# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable and its source files
add_executable(lob_sim
    main.cpp
    OrderBook.cpp
    Metrics.cpp
    Utils.cpp
)

# You can add include directories if needed, though not necessary with this flat structure
# target_include_directories(lob_sim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Optional: Add compiler flags for optimization and warnings
target_compile_options(lob_sim PRIVATE -O3 -Wall -Wextra)
