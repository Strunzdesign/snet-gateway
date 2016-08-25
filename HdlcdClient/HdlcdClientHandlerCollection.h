#ifndef HDLCD_CLIENT_HANDLER_COLLECTION_H
#define HDLCD_CLIENT_HANDLER_COLLECTION_H

#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "HdlcdPacketData.h"
class HdlcdClientHandler;
class Routing;

class HdlcdClientHandlerCollection {
public:
    HdlcdClientHandlerCollection(boost::asio::io_service& a_IOService);
    void CreateHdlcdClientHandler(const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName);
    void RegisterRoutingEntity(Routing* a_pRoutingEntity);

    void Send(const HdlcdPacketData& a_HdlcdPacketData, std::function<void()> a_OnSendDoneCallback = std::function<void()>());
    
private:
    // Members
    boost::asio::io_service& m_IOService;
    std::vector<std::shared_ptr<HdlcdClientHandler>> m_HdlcdClientHandlerVector;
    Routing* m_pRoutingEntity;
};

#endif // HDLCD_CLIENT_HANDLER_COLLECTION_H
