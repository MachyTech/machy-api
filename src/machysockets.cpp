#include "machysockets.h"

namespace machysockets_aSync
{
    void aSyncTCPClient::send_data(const std::string& raw_ip_address, 
        unsigned short port_num, Callback callback,
        unsigned int request_id, const std::string& trajectory_data)
    {
        std::string request =  trajectory_data;
        // initialize a new session using defined port and ip
        std::shared_ptr<aSync_Session> session = 
        std::shared_ptr<aSync_Session>(new aSync_Session(m_ios,
            raw_ip_address, port_num, request, request_id, callback));
        session->m_sock.open(session->m_ep.protocol());

        // thread safe (m_active_sessions is a memory pool with all active sessions)
        std::unique_lock<std::mutex> lock(m_active_sessions_guard);
        m_active_sessions[request_id] = session;
        lock.unlock();
        session->m_sock.async_connect(session->m_ep,
            [this, session](const system::error_code& ec)
            {
                if (ec != 0) {
                    session->m_ec = ec;
                    onRequestComplete(session);
                    return;
                }
                // check that the request is not yet canceled
                std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);
                if(session->m_was_cancelled){
                    onRequestComplete(session);
                    return;
                }
                // write when connect was successfull
                asio::async_write(session->m_sock, asio::buffer(session->m_request),
                [this, session](const boost::system::error_code& ec,
                                    std::size_t bytes_transferred)
                    {
                        // same checks
                        if(ec != 0) {
                            session->m_ec = ec;
                            onRequestComplete(session);
                            return;
                        }
                        std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);
                        if(session->m_was_cancelled) {
                            onRequestComplete(session);
                            return;
                        }
                        // read the response when write was succesfull
                        asio::async_read_until(session->m_sock, session->m_response_buf,'\n',
                        [this, session](const boost::system::error_code& ec,
                            std::size_t bytes_transferred)
                            {
                                if (ec != 0) {
                                    session->m_ec = ec;
                                } else {
                                    std::istream strm(&session->m_response_buf);
                                    std::getline(strm, session->m_response);
                                }

                                onRequestComplete(session);
                            });//async_read_until
                    });//async_write
            });//async_connect
    }
    void aSyncTCPClient::emulateLongComputationOp(unsigned int duration_sec, 
        const std::string& raw_ip_address,
        unsigned short port_num,
        Callback callback,
        unsigned int request_id)
    {
        std::string request = "EMULATE_LONG_CALC_OP " + std::to_string(duration_sec) + "\n";
        std::shared_ptr<aSync_Session> session =
        std::shared_ptr<aSync_Session>(new aSync_Session(m_ios, 
            raw_ip_address, port_num, request, request_id, callback));
        session->m_sock.open(session->m_ep.protocol());

        std::unique_lock<std::mutex> lock(m_active_sessions_guard);
        m_active_sessions[request_id] = session;
        lock.unlock();
        session->m_sock.async_connect(session->m_ep,
            [this, session](const system::error_code& ec)
            {
                if (ec != 0) {
                    session->m_ec = ec;
                    onRequestComplete(session);
                    return;
                }
                // has the request not been canceled yet
                std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);
                if(session->m_was_cancelled){
                    onRequestComplete(session);
                    return;
                }
                asio::async_write(session->m_sock, asio::buffer(session->m_request),
                [this, session](const boost::system::error_code& ec,
                                    std::size_t bytes_transferred)
                    {
                        if(ec != 0) {
                            session->m_ec = ec;
                            onRequestComplete(session);
                            return;
                        }
                        std::unique_lock<std::mutex>cancel_lock(session->m_cancel_guard);
                        if (session->m_was_cancelled) {
                            onRequestComplete(session);
                            return;
                        }
                        asio::async_read_until(session->m_sock, session->m_response_buf,'\n',
                        [this, session](const boost::system::error_code& ec,
                            std::size_t bytes_transferred)
                            {
                                if (ec != 0) {
                                    session->m_ec = ec;
                                } else {
                                    std::istream strm(&session->m_response_buf);
                                    std::getline(strm, session->m_response);
                                }

                                onRequestComplete(session);
                            });//async_read_until
                    });// async_write
            });// async_connect
    }

    void aSyncTCPClient::cancelRequest(unsigned int request_id) 
    {
        std::unique_lock<std::mutex> lock(m_active_sessions_guard);
        auto it = m_active_sessions.find(request_id);
        if (it != m_active_sessions.end()) {
            std::unique_lock<std::mutex>cancel_lock(it->second->m_cancel_guard);
            it->second->m_was_cancelled = true;
            it->second->m_sock.cancel();
        }
    }

    void aSyncTCPClient::onRequestComplete(std::shared_ptr<aSync_Session> session)
    {
            boost::system::error_code ignored_ec;
            session->m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, ignored_ec);
            std::unique_lock<std::mutex> lock(m_active_sessions_guard);
            auto it = m_active_sessions.find(session->m_id);
            if (it != m_active_sessions.end())
                m_active_sessions.erase(it);
            lock.unlock();
            boost::system::error_code ec;
            if (session->m_ec == 0 && session->m_was_cancelled)
                ec = asio::error::operation_aborted;
            else
                ec = session->m_ec;
            session->m_callback(session->m_id, session->m_response, ec);
    }
}