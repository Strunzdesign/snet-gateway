#include "HdlcdClientHandlerCollection.h"
#include "HdlcdClientHandler.h"
#include <assert.h>

HdlcdClientHandlerCollection::HdlcdClientHandlerCollection(boost::asio::io_service& a_IOService): m_IOService(a_IOService), m_pRoutingEntity(NULL) {
}

void HdlcdClientHandlerCollection::RegisterRoutingEntity(Routing* a_pRoutingEntity) {
    assert(a_pRoutingEntity);
    m_pRoutingEntity = a_pRoutingEntity;
}

void HdlcdClientHandlerCollection::CreateHdlcdClientHandler(const std::string& a_DestinationName, const std::string& a_TcpPort, const std::string& a_SerialPortName) {
    // Create new HDLCd client entity
    assert(m_pRoutingEntity);
    auto l_NewClientHandler = std::make_shared<HdlcdClientHandler>(m_IOService, a_DestinationName, a_TcpPort, a_SerialPortName, m_pRoutingEntity);
    m_HdlcdClientHandlerVector.push_back(l_NewClientHandler);
}

void HdlcdClientHandlerCollection::Send(const HdlcdPacketData& a_HdlcdPacketData, std::function<void()> a_OnSendDoneCallback) {
    for (auto l_HdlcdClientHandlerIterator = m_HdlcdClientHandlerVector.begin(); l_HdlcdClientHandlerIterator != m_HdlcdClientHandlerVector.end(); ++l_HdlcdClientHandlerIterator) {
        (*l_HdlcdClientHandlerIterator)->Send(a_HdlcdPacketData);
    } // for
}
