#include <functional>

#include "socket.h"
#include "log.h"

namespace net
{

close_handler_exception::close_handler_exception(int socket_number) : m_socket_number(socket_number) {}
const char* close_handler_exception::what() const noexcept
{
    return NULL;
}
int close_handler_exception::socket_number() const noexcept
{
    return m_socket_number;
}

std::atomic<int> socket::m_socket_counter(0);
const int socket::EXTENSION_BUFFER = 1024;

socket::socket(boost::asio::io_service& io_service, const std::string& address) :
    m_address(address),
    m_socket_number(++m_socket_counter),
    m_socket(io_service),
    m_status(ST_EMPTY),
    m_data_size(0)
{}

void socket::counter_increment()
{
    *const_cast<int*>(&m_socket_number) = ++m_socket_counter;
}

int socket::get_number()
{
    return m_socket_number;
}

int socket::get_status()
{
    return m_status;
}

void socket::set_status(int status)
{
    m_status = status;
}

void socket::_receive_handler(const boost::system::error_code &err, size_t bytes)
{
    lo::l(lo::DEBUG) << "socket::_receive_handler, bytes: " << bytes;

    m_data_size += bytes;

    if (err == boost::asio::error::eof ||
        err == boost::asio::error::connection_reset) {
        throw close_handler_exception(m_socket_number);
    } else receive_handler(err, bytes);
}

boost_socket& socket::sock()
{
    return m_socket;
}

boost_deadline& socket::deadline(boost::asio::io_service& io_service)
{
    if (!m_deadline_ptr) m_deadline_ptr = std::make_shared<boost_deadline>(io_service);
    return *m_deadline_ptr;
}

boost_deadline& socket::deadline()
{
    return *m_deadline_ptr;
}

boost_endpoint& socket::endpoint(const std::string& ip, uint16_t port)
{
    if (!m_endpoint_ptr) m_endpoint_ptr = std::make_shared<boost_endpoint>(boost::asio::ip::address_v4::from_string(ip), port);
    return *m_endpoint_ptr;
}

boost_endpoint& socket::endpoint()
{
    return *m_endpoint_ptr;
}

void socket::send(const std::vector<unsigned char>& data)
{
    if (data.size() == 0) return;
    m_socket.async_send(boost::asio::buffer(data.data(), data.size()), std::bind(&socket::send_handler, this, std::placeholders::_1));
}

void socket::receive()
{
    if (m_data.size() == 0) m_data.resize(EXTENSION_BUFFER);
    else if (m_data.size() - m_data_size == 0) m_data.resize(m_data.size() * 1.5);

    size_t length = m_data.size() - m_data_size;

    m_socket.async_receive(boost::asio::buffer(&m_data.at(m_data_size), length), std::bind(&socket::_receive_handler, this, std::placeholders::_1, std::placeholders::_2));
}
    
size_t socket::get_data_size()
{
    return m_data_size;
}

void socket::clear_buffer()
{
    m_data_size = 0;
    m_data.clear();
}

void socket::close()
{
    throw close_handler_exception(m_socket_number);
}

socket::~socket()
{
    /* TODO вызывает исключение. необходимо разобраться
    try {
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    } catch (const boost::system::system_error& e) {
        lo::l(lo::ERROR) << e.what();
    }
    */

    try {
        m_socket.close();
    } catch (const boost::system::system_error& e) {
        lo::l(lo::ERROR) << e.what();
    }
}

// ********* HANDLERS *******
void socket::accept_handler()
{
    if (on_accept) on_accept(m_socket_number);
}
void socket::connect_handler()
{
    if (on_connect) on_connect(m_address, m_socket_number);
}
void socket::receive_handler(const boost::system::error_code &err, size_t bytes)
{
    if (on_receive) {
        std::vector<unsigned char> temp;
        m_data.resize(m_data_size);
        temp.swap(m_data);
        m_data_size = 0;

        on_receive(m_socket_number, temp);
    }
}
void socket::send_handler(const boost::system::error_code &err)
{
    if (on_send) on_send(m_socket_number);
}
void socket::wait_handler(const boost::system::error_code &err)
{
    if (on_wait) on_wait(m_socket_number);
}
void socket::close_handler()
{
    if (on_close) on_close(m_socket_number);
}

// ************ SETS ************
void socket::set_accept_handler(std::function<void(int)> fn)
{
    on_accept = fn;
}
void socket::set_connect_handler(std::function<void(std::string, int)> fn)
{
    on_connect = fn;
}
void socket::set_close_handler(std::function<void(int)> fn)
{
    on_close = fn;
}
void socket::set_receive_handler(std::function<void(int, const std::vector<unsigned char>&)> fn)
{
    on_receive = fn;
}
void socket::set_send_handler(std::function<void(int)> fn)
{
    on_send = fn;
}
void socket::set_wait_handler(std::function<void(int)> fn)
{
    on_wait = fn;
}

} // namespace net
