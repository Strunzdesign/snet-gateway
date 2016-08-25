#ifndef HDLCD_CLIENT_HANDLER_H
#define HDLCD_CLIENT_HANDLER_H

#include <memory>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "HdlcdPacketData.h"
class HdlcdAccessClient;
class Routing;

class HdlcdClientHandler {
public:
    HdlcdClientHandler(boost::asio::io_service& a_IOService, const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName, Routing* a_pRouting);
    void Send(const HdlcdPacketData& a_HdlcdPacketData, std::function<void()> a_OnSendDoneCallback = std::function<void()>());
    
private:
    // Helpers
    void ResolveDestination();
    
    // Members
    boost::asio::io_service& m_IOService;
    const std::string m_DestinationName;
    const std::string m_TcpPort;
    const std::string m_SerialPortName;
    
    // Resolver
    boost::asio::ip::tcp::resolver m_Resolver;
    boost::asio::deadline_timer m_ConnectionRetryTimer;
    
    // The connection to the HDLC Daemon
    std::shared_ptr<HdlcdAccessClient> m_HdlcdAccessClient;
    
    Routing* m_pRouting;
};

#endif // HDLCD_CLIENT_HANDLER_H
