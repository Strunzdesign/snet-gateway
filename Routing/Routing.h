#ifndef ROUTING_H
#define ROUTING_H

#include <vector>
class SnetServiceMessage;
class ToolHandlerCollection;
class HdlcdClientHandlerCollection;

class Routing {
public:
    Routing(ToolHandlerCollection &a_ToolHandlerCollection, HdlcdClientHandlerCollection &a_HdlcdClientHandlerCollection);
    
    void RouteIncomingSnetPacket(SnetServiceMessage* a_pSnetServiceMessage);

private:
    // Members
    ToolHandlerCollection&        m_ToolHandlerCollection;
    HdlcdClientHandlerCollection& m_HdlcdClientHandlerCollection;
};

#endif // ROUTING_H
