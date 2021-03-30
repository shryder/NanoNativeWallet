# CMakeList.txt : CMake project for NanoNativeWallet, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Find libraries
find_package(CURL CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(unofficial-curlpp CONFIG REQUIRED)
find_package(SDL2 REQUIRED)

# OpenGL
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Include directories
include_directories(${Boost_INCLUDE_DIR})

# Visual Studio only I think
set(CMAKE_CXX_STANDARD 17)

# Define executable
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "*.h" "*.cpp" "*.c")
add_executable(NanoNativeWallet ${SRC_FILES})
target_compile_definitions(NanoNativeWallet PRIVATE -DED25519_CUSTOMHASH)

target_link_libraries(NanoNativeWallet PRIVATE ${D3D11_LIBRARY} ${D3DX11_LIBRARY})

target_link_libraries(NanoNativeWallet PRIVATE CURL::libcurl)
target_link_libraries(NanoNativeWallet PRIVATE cryptopp-static)
target_link_libraries(NanoNativeWallet PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(NanoNativeWallet PRIVATE optimized ${Boost_LIBRARY_DIRS}/../../lib/boost_system-vc140-mt.lib debug ${Boost_LIBRARY_DIRS}/boost_system-vc140-mt.lib)
target_link_libraries(NanoNativeWallet PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(NanoNativeWallet PRIVATE unofficial::curlpp::curlpp)
target_link_libraries(NanoNativeWallet PRIVATE SDL2::SDL2 SDL2::SDL2main)

# OpenGL
target_link_libraries(NanoNativeWallet PRIVATE glfw)
target_link_libraries(NanoNativeWallet PRIVATE GLEW::GLEW)
