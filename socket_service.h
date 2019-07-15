#ifndef SOCKET_SERVICE_H
#define SOCKET_SERVICE_H

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio.hpp>

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <regex>

#include "socket.h"
#include "log.h"

namespace net
{

template <typename SOCKET_PROCESSOR>
class socket_service
{
private:
    typedef std::shared_ptr<SOCKET_PROCESSOR> processor_t;
    typedef std::unordered_map<int, processor_t> sockets_t;
    sockets_t m_sockets;
    std::mutex m_mutex;
    std::string m_address;

    boost::asio::io_service                         m_service;
    std::shared_ptr<boost::asio::ip::tcp::endpoint> m_endpoint_ptr;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor_ptr;

    std::function<void(int)> on_accept;
    std::function<void(std::string, int)> on_connect;
    std::function<void(int)> on_close;
    std::function<void(int, const std::vector<unsigned char>&)> on_receive;
    std::function<void(int)> on_send;
    std::function<void(int)> on_wait;

private:
    static inline void parse_address(const std::string& address, std::string& ip, uint16_t& port)
    {
        static std::regex pattern("^[0-9]+$");
        
        auto pos = address.find(":");
        if (pos != std::string::npos) {
            ip = address.substr(0, pos);
            port = atoi(address.substr(pos + 1, address.size() - pos - 1).c_str());
        } else {
            if (std::regex_match(address, pattern)) {
                ip = "0.0.0.0";
                port = atoi(address.c_str());
            } else {
                ip = address;
                port = 80;
            }
        }
    }

public:
    void listen(const std::string& address)
    {
        if (address.empty()) return;
        m_address = address;

        std::string ip;
        uint16_t port;
        parse_address(address, ip, port);

        m_endpoint_ptr = std::make_shared<boost::asio::ip::tcp::endpoint>(boost::asio::ip::address_v4::from_string(ip), port);
        m_acceptor_ptr = std::make_shared<boost::asio::ip::tcp::acceptor>(m_service, *m_endpoint_ptr);
    }

    void accept()
    {
        if (m_address.empty()) return;

        auto socket_ptr = std::make_shared<SOCKET_PROCESSOR>(m_service, "");
        add_socket_into_list(socket_ptr);

        m_acceptor_ptr->async_accept(socket_ptr->sock(), std::bind(&socket_service::accept_handler, this, socket_ptr->get_number(), std::placeholders::_1));
    }

    void connect(const std::string& address, size_t timeout_milliseconds)
    {
        std::string ip;
        uint16_t port;
        parse_address(address, ip, port);

        auto socket_ptr = std::make_shared<SOCKET_PROCESSOR>(m_service, address);
        add_socket_into_list(socket_ptr);

        socket_ptr->deadline(m_service).expires_from_now(static_cast<boost::posix_time::time_duration>(boost::posix_time::milliseconds(timeout_milliseconds)));
        socket_ptr->deadline(m_service).async_wait(std::bind(&socket_service::wait_handler, this, socket_ptr->get_number(), std::placeholders::_1));

        socket_ptr->sock().async_connect(socket_ptr->endpoint(ip, port), std::bind(&socket_service::connect_handler, this, socket_ptr->get_number(), std::placeholders::_1));
    }

    void run()
    {
        accept();

        while (true) {
            try {
                m_service.run();
                break;
            } catch (const net::close_handler_exception& e) {
                close_handler(e.socket_number());
            } catch (const boost::system::system_error& e) {
                lo::l(lo::ERROR) << "socket_service::run: " << e.what();
                m_sockets.clear();
            } catch (...) {
                lo::l(lo::FATAL) << "socket_service::run";
                throw 0;
            }
        }
    }

    void stop()
    {
        m_service.reset();
        m_service.stop();
    }

private:
    inline void set_handlers(std::shared_ptr<SOCKET_PROCESSOR> socket_ptr)
    {
        socket_ptr->set_accept_handler(on_accept);
        socket_ptr->set_connect_handler(on_connect);
        socket_ptr->set_close_handler(on_close);
        socket_ptr->set_receive_handler(on_receive);
        socket_ptr->set_send_handler(on_send);
        socket_ptr->set_wait_handler(on_wait);
    }

    inline void add_socket_into_list(std::shared_ptr<SOCKET_PROCESSOR> socket_ptr)
    {
        set_handlers(socket_ptr);

        while (check_number(socket_ptr->get_number())) socket_ptr->counter_increment();
        // секция для лока
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_sockets.insert(std::make_pair(socket_ptr->get_number(), socket_ptr));
        }
    }

    void accept_handler(int socket_id, const boost::system::error_code &err)
    {
        if (err) {
            close_handler(socket_id);            
            return;
        } else {
            std::shared_ptr<SOCKET_PROCESSOR> socket_ptr;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto it = m_sockets.find(socket_id);
                if (it != m_sockets.end()) socket_ptr = it->second;
            }
            if (socket_ptr) socket_ptr->accept_handler();
            accept();
        }
    }

    void connect_handler(int socket_id, const boost::system::error_code &err)
    {
        if (err) {
            close_handler(socket_id);            
            return;
        } else {
            std::shared_ptr<SOCKET_PROCESSOR> socket_ptr;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto it = m_sockets.find(socket_id);
                if (it != m_sockets.end()) socket_ptr = it->second;
            }
            if (socket_ptr) socket_ptr->connect_handler();
        }
    }

    void close_handler(int socket_id)
    {
        std::shared_ptr<SOCKET_PROCESSOR> socket_ptr;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_sockets.find(socket_id);
            if (it == m_sockets.end()) return; 
            socket_ptr = it->second;
            m_sockets.erase(it);
        }
        if (socket_ptr) socket_ptr->close_handler();
    }

    void wait_handler(int socket_id, const boost::system::error_code &err)
    {
        std::shared_ptr<SOCKET_PROCESSOR> socket;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_sockets.find(socket_id);
            if (it != m_sockets.end()) return; 
            socket = it->second;
        }
        socket->wait_handler(err);
    }

    bool check_number(int socket_id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sockets.find(socket_id);
        return it != m_sockets.end();
    }

public:
    size_t size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_sockets.size();
    }

    void send(int socket_id, const std::vector<unsigned char>& data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sockets.find(socket_id);
        if (it != m_sockets.end()) it->second->send(data);
    }

    void receive(int socket_id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_sockets.find(socket_id);
        if (it != m_sockets.end()) it->second->receive();
    }

    void set_accept_handler(std::function<void(int)> fn)
    {
        on_accept = fn;
    }

    void set_connect_handler(std::function<void(std::string, int)> fn)
    {
        on_connect = fn;
    }

    void set_close_handler(std::function<void(int)> fn)
    {
        on_close = fn;
    }

    void set_receive_handler(std::function<void(int, const std::vector<unsigned char>&)> fn)
    {
        on_receive = fn;
    }

    void set_send_handler(std::function<void(int)> fn)
    {
        on_send = fn;
    }

    void set_wait_handler(std::function<void(int)> fn)
    {
        on_wait = fn;
    }
};

} // namespace net

#endif
