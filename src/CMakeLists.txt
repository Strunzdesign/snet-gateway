if(UNIX OR MINGW)
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
endif()
find_package(Boost REQUIRED COMPONENTS system signals program_options regex date_time)
include_directories(${Boost_INCLUDE_DIR})
include_directories(
    "HdlcdClient"
    "Routing"
    "GatewayAccessServer"
    "GatewayAccessServer/AddressService"
    "GatewayAccessServer/PublishSubscribeService"
    "GatewayAccessServer/GatewayAccessProtocol"
    "GatewayAccessServer/GatewayAccessProtocol/CommandResponseFrames"
    "GatewayAccessServer/GatewayAccessProtocol/GatewayAccessFrames"
)
    
find_package(Threads)

add_executable(snet-gateway
    main-snet-gateway.cpp
    HdlcdClient/HdlcdClientHandler.cpp
    HdlcdClient/HdlcdClientHandlerCollection.cpp
    Routing/Routing.cpp
    GatewayAccessServer/AddressService/AddressLease.cpp
    GatewayAccessServer/AddressService/AddressPool.cpp
    GatewayAccessServer/AddressService/AddressService.cpp
    GatewayAccessServer/PublishSubscribeService/PublishSubscribeService.cpp
    GatewayAccessServer/GatewayAccessProtocol/GatewayAccessProtocol.cpp
    GatewayAccessServer/GatewayAccessServerHandler.cpp
    GatewayAccessServer/GatewayAccessServerHandlerCollection.cpp
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
