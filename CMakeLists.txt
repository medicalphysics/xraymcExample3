cmake_minimum_required(VERSION 3.20)

project(xraymcExample3 VERSION 0.0.1 DESCRIPTION "xraymc example 3" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Fetch xraymc library
# xraymc is rather lightweight and is designed to be downloaded at 
# configure time by cmake's FetchContent module

# Optional, make xraymc download material database from IAEA for you
set(XRAYMCLIB_EPICS_DOWNLOAD "ON")
# Optional, include option to save png files
set(XRAYMCLIB_USE_LOADPNG "ON")

# Include CMakes FetchContent module
include(FetchContent)
# Specify where to fint xraymc
FetchContent_Declare(
    libxraymc # name of cmake target (you can chose)
    GIT_REPOSITORY https://github.com/medicalphysics/XRayMClib.git # Git address
    GIT_TAG serializer # Git branch name (master for stable version) 
)
FetchContent_MakeAvailable(libxraymc)


# Create an executable application
add_executable(xraymcexample3 
	src/main.cpp
    src/icrp110phantomreader.cpp
)

target_include_directories(xraymcexample3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(xraymcexample3 PRIVATE libxraymc)
# Adding data from xraymcCarm
xraymclib_add_physics_list(xraymcexample3)
xraymclib_add_icrp145_organ_data(xraymcexample3)

# Adding phantom data
set(DATA_FILES 
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP110/AF/AF.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP110/AF/AF_media.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP110/AF/AF_organs.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP110/AM/AM.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP110/AM/AM_media.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP110/AM/AM_organs.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP145/MRCP_AF/MRCP_AF.ele"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP145/MRCP_AF/MRCP_AF.node"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP145/MRCP_AF/MRCP_AF_media.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP145/MRCP_AM/MRCP_AM.ele"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP145/MRCP_AM/MRCP_AM.node"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ICRP145/MRCP_AM/MRCP_AM_media.dat"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/STL/carm.stl"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/STL/table.stl"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/STL/blanket.stl"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/STL/ceilingShield.stl"
)

# Copying data files to the build directory after building of the example
foreach(datafile ${DATA_FILES})
    cmake_path(SET datapath "${datafile}")        
    cmake_path(GET datapath FILENAME datafilename)
    add_custom_command(TARGET xraymcexample3 POST_BUILD			
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${datafile} ${datafilename}
			WORKING_DIRECTORY $<TARGET_FILE_DIR:xraymcexample3>
			)
endforeach()



