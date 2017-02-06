if(MSVC)
    set(Boost_USE_STATIC_LIBS ON)     
    find_package(Boost REQUIRED COMPONENTS system signals program_options date_time regex)
else()
    set(Boost_USE_STATIC_LIBS OFF) 
    set(Boost_USE_MULTITHREADED ON)  
    set(Boost_USE_STATIC_RUNTIME OFF) 
    find_package(Boost REQUIRED COMPONENTS system signals program_options regex)
endif()
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

add_executable(${PROJECT_NAME}
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

target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${ADDITIONAL_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
