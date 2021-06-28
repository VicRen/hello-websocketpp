#ifndef LIGHTSPEED_PROTOO_WEBSOCKET_TRANSPORT_H
#define LIGHTSPEED_PROTOO_WEBSOCKET_TRANSPORT_H

#include "thread"

#ifdef PROTOO_ENABLE_SSL
#define OPENSSL_IS_BORINGSSL
#include "websocketpp/config/asio_client.hpp"
#else

#include <websocketpp/config/asio_no_tls_client.hpp>

#endif

#include "websocketpp/client.hpp"
#include "websocketpp/common/thread.hpp"
#include "websocketpp/common/memory.hpp"

#include "iwebsocket_transport.h"

namespace lightspeed {
	namespace rtc {
#ifdef PROTOO_ENABLE_SSL
		typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
		typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
#else
		typedef websocketpp::client<websocketpp::config::asio_client> client;
#endif
        enum Status{
            kConnecting = 0,
            kOpen,
            kFailed,
            kClosed,
        };

		class WebsocketTransport : public protoo::IWebsocketTransport {
		public:
			explicit WebsocketTransport(std::string uri);

			~WebsocketTransport() override;

			void on_open(websocketpp::connection_hdl hdl);

			void on_fail(websocketpp::connection_hdl hdl);

			void on_close(websocketpp::connection_hdl hdl);

			void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);

			void on_pong_timeout(websocketpp::connection_hdl hdl, std::string payload);

			void connect(IWebsocketTransport::IListener *listener) override;

			void sendMessage(const std::string &message) override;

			void close() override;

			bool isClosed() override { return closed; }

		private:
#ifdef PROTOO_ENABLE_SSL
			context_ptr on_tls_init(websocketpp::connection_hdl);
#endif
			std::string url;
			bool closed;
			bool connected;
			client m_endpoint;
			uint8_t m_timeouts = 0;
			websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
			websocketpp::connection_hdl m_hdl;
			IWebsocketTransport::IListener *listener;
            Status m_status;
		};
	} // namespace protoo
} // namespace lightspeed

#endif // LIGHTSPEED_PROTOO_WEBSOCKET_TRANSPORT_H
