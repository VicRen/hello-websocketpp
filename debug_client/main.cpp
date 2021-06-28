#include <thread>

#include "websocket_transport.h"

using namespace lightspeed::rtc;

class Listener : public WebsocketTransport::IListener {
public:
    void onOpen() override {
        std::cout << __FUNCTION__ << std::endl;
    }

    void onFail() override {
        std::cout << __FUNCTION__ << std::endl;
    }

    void onMessage(const std::string &string) override {
        std::cout << __FUNCTION__ << " message: " << string << std::endl;
    }

    void onDisconnected() override {
        std::cout << __FUNCTION__ << std::endl;
    }

    void onClose() override {
        std::cout << __FUNCTION__ << std::endl;
    }

};

int main() {
    auto transport = new WebsocketTransport("ws://127.0.0.1:9012");
    transport->connect(new Listener());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    transport->sendMessage("testing message");

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

//    transport->close();
    delete transport;
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    std::this_thread::sleep_for(std::chrono::milliseconds(500000));
    return 0;
}