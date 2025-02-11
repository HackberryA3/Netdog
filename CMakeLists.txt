cmake_minimum_required(VERSION 3.0)
project(netdog CXX)

add_executable(netdog netdog.cpp)

target_compile_options(netdog PUBLIC -O2 -Wall)
target_compile_features(netdog PUBLIC cxx_std_23)
target_link_libraries(netdog PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:ws2_32.lib>
	$<$<CXX_COMPILER_ID:GNU>:lws2_32>
)

add_executable(nc netdog.cpp)

target_compile_options(nc PUBLIC -O2 -Wall)
target_compile_features(nc PUBLIC cxx_std_23)
target_link_libraries(nc PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:ws2_32.lib>
	$<$<CXX_COMPILER_ID:GNU>:lws2_32>
)
