set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost REQUIRED COMPONENTS system signals program_options regex)
include_directories(${Boost_INCLUDE_DIR})

find_package(Threads)

add_executable(snet-gateway
main.cpp
HdlcdClient/HdlcdClientHandler.cpp
HdlcdClient/HdlcdClientHandlerCollection.cpp
Routing/Routing.cpp
ToolServer/AddressLease.cpp
ToolServer/AddressPool.cpp
ToolServer/AddressService.cpp
ToolServer/PublishSubscribeService.cpp
ToolServer/ToolFrameGenerator.cpp
ToolServer/ToolFrameParser.cpp
ToolServer/ToolHandler.cpp
ToolServer/ToolHandlerCollection.cpp
)

if(WIN32)
    set(ADDITIONAL_LIBRARIES wsock32 ws2_32)
else()
    set(ADDITIONAL_LIBRARIES "")
endif()

target_link_libraries(snet-gateway
${Boost_LIBRARIES}
${CMAKE_THREAD_LIBS_INIT}
${ADDITIONAL_LIBRARIES}
)

install(TARGETS snet-gateway RUNTIME DESTINATION bin)