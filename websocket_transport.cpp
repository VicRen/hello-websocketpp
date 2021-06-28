#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include <websocketpp/client.hpp>
#include <utility>
#include <json.hpp>

#include "websocket_transport.h"

using namespace lightspeed::protoo;

namespace lightspeed {
    namespace rtc {
        // WebsocketTransport
        WebsocketTransport::WebsocketTransport(std::string uri) : url(std::move(uri)), closed(false), m_status(kConnecting) {
            m_endpoint.set_access_channels(websocketpp::log::alevel::all);
            m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_header);
            m_endpoint.clear_access_channels(websocketpp::log::alevel::frame_payload);
            m_endpoint.set_error_channels(websocketpp::log::elevel::all);

            m_endpoint.init_asio();
            m_endpoint.start_perpetual();

            m_endpoint.set_pong_timeout(1000);
            m_endpoint.set_open_handshake_timeout(4000);

            m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
        }

        WebsocketTransport::~WebsocketTransport() {
            m_endpoint.stop_perpetual();

            if (m_status == kOpen) {
                websocketpp::lib::error_code ec;
                m_endpoint.close(m_hdl, websocketpp::close::status::going_away, "", ec);
                if (ec) {
                    std::cout << "> Error closing connection " << ": " << ec.message() << std::endl;
                }
            }

            m_thread->join();
        }

        void WebsocketTransport::connect(IWebsocketTransport::IListener *listener) {
            std::cout << __FUNCTION__ << " url: " << url << std::endl;
            this->listener = listener;
#ifdef PROTOO_ENABLE_SSL
            m_endpoint.set_tls_init_handler(websocketpp::lib::bind(
                    &WebsocketTransport::on_tls_init,
                    this,
                    websocketpp::lib::placeholders::_1));
#endif
            websocketpp::lib::error_code ec;

            client::connection_ptr con = m_endpoint.get_connection(url, ec);
            if (ec) {
//	            RTC_LOG(LS_INFO) << __FUNCTION__ << "> Connect initialization error: " << ec.message();
                return;
            }

            m_hdl = con->get_handle();


            con->set_open_handler(websocketpp::lib::bind(
                    &WebsocketTransport::on_open,
                    this,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_fail_handler(websocketpp::lib::bind(
                    &WebsocketTransport::on_fail,
                    this,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_close_handler(websocketpp::lib::bind(
                    &WebsocketTransport::on_close,
                    this,
                    websocketpp::lib::placeholders::_1
            ));
            con->set_message_handler(websocketpp::lib::bind(
                    &WebsocketTransport::on_message,
                    this,
                    websocketpp::lib::placeholders::_1,
                    websocketpp::lib::placeholders::_2
            ));
            con->set_pong_timeout_handler(websocketpp::lib::bind(
                    &WebsocketTransport::on_pong_timeout,
                    this,
                    websocketpp::lib::placeholders::_1,
                    websocketpp::lib::placeholders::_2));

            con->add_subprotocol("protoo");
            m_endpoint.connect(con);
        }

        void WebsocketTransport::close() {
            std::cout << __FUNCTION__ << std::endl;
            closed = true;
            websocketpp::lib::error_code ec;
            m_endpoint.close(m_hdl, websocketpp::close::status::going_away, "", ec);
            if (ec) {
                std::cout << "> Error initiating close: " << ec.message() << std::endl;
            }
        }

        void WebsocketTransport::sendMessage(const std::string &message) {
            websocketpp::lib::error_code ec;
//                std::cout << "WebsocketTransport::sendMessage: " << message << std::endl;
            m_endpoint.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
            if (ec) {
                std::cout << "> Error sending message: " << ec.message() << std::endl;
                return;
            }
        }

        void WebsocketTransport::on_open(websocketpp::connection_hdl hdl) {
//	        RTC_LOG(LS_INFO) << __FUNCTION__;
            m_status = kOpen;
            if (closed) {
                return;
            }
            connected = true;
            if (listener != nullptr) {
                listener->onOpen();
            }
            std::thread th([&, hdl] () {
                auto con = m_endpoint.get_con_from_hdl(hdl);
                while (con->get_state() == websocketpp::session::state::open && !closed) {
//                    std::cout << "Sending heartbeat" << std::endl;
                    websocketpp::lib::error_code ec;
                    m_endpoint.ping(hdl, "", ec);
                    if (ec) {
                        std::cout << "Ping error: " << ec << std::endl;
                        break;
                    }

                    // Make sure this is longer than the pong timeout (currently 1000ms) so we
                    // don't send another too quickly.
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }
            });
            th.detach();
        };

        void WebsocketTransport::on_fail(websocketpp::connection_hdl hdl) {
            std::cout << __FUNCTION__ << std::endl;
            m_status = kFailed;
            if (closed) {
                return;
            }
            closed = true;
            if (listener != nullptr) {
                listener->onFail();
            }
        }

        void WebsocketTransport::on_close(websocketpp::connection_hdl hdl) {
//            RTC_LOG(LS_INFO) << __FUNCTION__;
            m_status = kClosed;
            if (closed) {
                return;
            }
            closed = true;
            connected = false;
            if (listener != nullptr) {
                listener->onClose();
            }
        }

        void WebsocketTransport::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
            if (closed) {
                return;
            }
            auto data = msg->get_payload();
//            std::cout << "WebsocketTransport::on_message: " << data << std::endl;
            if (listener != nullptr) {
                listener->onMessage(data);
            }
        }

#ifdef PROTOO_ENABLE_SSL
        context_ptr WebsocketTransport::on_tls_init(websocketpp::connection_hdl) {
            context_ptr ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(
                    boost::asio::ssl::context::tlsv12);

            boost::system::error_code ec;
            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::no_sslv3 |
                             boost::asio::ssl::context::single_dh_use, ec);
            if (ec) {
                std::cout << "on_tls_init: " << ec.message() << std::endl;
                return nullptr;
            }
            // TODO: cert verify
//                    ctx->set_default_verify_paths();
//                    ctx->set_verify_mode(boost::asio::ssl::verify_peer);

            return ctx;
        }
#endif

        void WebsocketTransport::on_pong_timeout(websocketpp::connection_hdl hdl, std::string payload) {
            if (m_timeouts < 2) {
                m_timeouts++;
                return;
            }
            closed = true;
//            std::cout << "PONG TIMEOUT!" << std::endl;

            if (listener) {
                listener->onDisconnected();
            }
        }
    } // namespace protoo
} // namespace lightspeed
