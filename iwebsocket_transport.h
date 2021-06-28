#ifndef PROTOOCLIENT_IWEBSOCKET_TRANSPORT_H
#define PROTOOCLIENT_IWEBSOCKET_TRANSPORT_H

#include <string>

namespace lightspeed {
    namespace protoo {
        class IWebsocketTransport {
        public:
            class IListener {
            public:
                virtual ~IListener() = default;

                virtual void onOpen() = 0;

                virtual void onFail() = 0;

                virtual void onMessage(const std::string &) = 0;

                virtual void onDisconnected() = 0;

                virtual void onClose() = 0;
            };

            virtual ~IWebsocketTransport() = default;

            virtual void connect(IListener *listener) = 0;

            virtual void sendMessage(const std::string &message) = 0;

            virtual void close() = 0;

            virtual bool isClosed() = 0;
        };
    } // namespace protoo
} // namespace lightspeed

#endif //PROTOOCLIENT_IWEBSOCKET_TRANSPORT_H
