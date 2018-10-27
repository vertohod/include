#ifndef SOCKET_H
#define SOCKET_H

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>

#include <functional>
#include <stdexcept>
#include <vector>
#include <string>
#include <memory>
#include <atomic>
#include <map>

namespace net
{

class close_handler_exception : public std::exception
{
    int m_socket_number;

public:
    explicit close_handler_exception(int socket_number);
    const char* what() const noexcept;
    int socket_number() const noexcept;
};

typedef boost::asio::ip::tcp::socket    boost_socket;
typedef boost::asio::deadline_timer     boost_deadline;
typedef boost::asio::ip::tcp::endpoint  boost_endpoint;

class socket
{
private:
    static std::atomic<int>         m_socket_counter;

protected:
    static const int                EXTENSION_BUFFER;

private:
    const std::string               m_address;
    const int                       m_socket_number;
    boost_socket                    m_socket;
    int                             m_status;
    std::atomic<size_t>             m_data_size;
    std::vector<unsigned char>      m_data;

    std::shared_ptr<boost_deadline> m_deadline_ptr;
    std::shared_ptr<boost_endpoint> m_endpoint_ptr;

private:
    std::function<void(int)> on_accept;
    std::function<void(std::string, int)> on_connect;
    std::function<void(int)> on_close;
    std::function<void(int)> on_send;
    std::function<void(int)> on_wait;
    std::function<void(int, const std::vector<unsigned char>&)> on_receive;

public:
    enum STATUS : int {
        ST_EMPTY = 0,
        ST_RECEIVE,
        ST_SEND,
        ST_CLOSE,
        ST_RECEIVE_RECEIVE,
        ST_RECEIVE_SEND,
        ST_SEND_SEND,
        ST_SEND_RECEIVE,
        ST_RECEIVE_CLOSE,
        ST_SEND_CLOSE
    };

    socket(boost::asio::io_service& io_service, const std::string& address);
    virtual ~socket();

    virtual void accept_handler();
    virtual void connect_handler();
    virtual void close_handler();
    virtual void send_handler(const boost::system::error_code &err);
    virtual void wait_handler(const boost::system::error_code &err);
    virtual void receive_handler(const boost::system::error_code &err, size_t bytes);

    virtual void set_accept_handler(std::function<void(int)> fn);
    virtual void set_connect_handler(std::function<void(std::string, int)> fn);
    virtual void set_close_handler(std::function<void(int)> fn);
    virtual void set_send_handler(std::function<void(int)> fn);
    virtual void set_wait_handler(std::function<void(int)> fn);
    virtual void set_receive_handler(std::function<void(int, const std::vector<unsigned char>&)> fn);

    boost_socket& sock();
    boost_deadline& deadline(boost::asio::io_service& io_service);
    boost_deadline& deadline();
    boost_endpoint& endpoint(const std::string& ip, uint16_t port);
    boost_endpoint& endpoint();

    void send(const std::vector<unsigned char>& data);
    void receive();
    size_t get_data_size();
    std::vector<unsigned char>& get_data();
    void clear_buffer();
    void close();

    void counter_increment();
    int get_status();
    int get_number();
    void set_status(int status);

private:
    void _receive_handler(const boost::system::error_code &err, size_t bytes);
};

}

#endif
