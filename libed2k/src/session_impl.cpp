
#include "libtorrent/peer_connection.hpp"

#include "session_impl.hpp"

using namespace libed2k;
using namespace libed2k::aux;

session_impl::session_impl(int lst_port, const char* listen_interface,
                           const fingerprint& id, const std::string& logpath):
    m_ipv4_peer_pool(500),
    m_send_buffers(send_buffer_size),
    m_files(40),
    m_io_service(),
	m_host_resolver(m_io_service),
    m_alerts(m_io_service),
    m_disk_thread(m_io_service, boost::bind(&session_impl::on_disk_queue, this),
                  m_files),
    m_half_open(m_io_service),
    m_download_rate(libtorrent::peer_connection::download_channel),
    m_upload_rate(libtorrent::peer_connection::upload_channel),
    m_server_manager(*this),
    m_abort(false),
    m_paused(false),
    m_max_uploads(8),
    m_allowed_upload_slots(8),
    m_max_connections(200),
    m_logpath(logpath)
{
    libtorrent::error_code ec;
    m_listen_interface = tcp::endpoint(
        libtorrent::address::from_string(listen_interface, ec), lst_port);
    TORRENT_ASSERT(!ec);

#ifdef WIN32
    // windows XP has a limit on the number of
    // simultaneous half-open TCP connections
    DWORD windows_version = ::GetVersion();
    if ((windows_version & 0xff) >= 6)
    {
        // on vista the limit is 5 (in home edition)
        m_half_open.limit(4);
    }
    else
    {
        // on XP SP2 it's 10
        m_half_open.limit(8);
    }
#endif

    m_bandwidth_channel[libtorrent::peer_connection::download_channel] =
        &m_download_channel;
    m_bandwidth_channel[libtorrent::peer_connection::upload_channel] =
        &m_upload_channel;

    m_logger = create_log("main_session", listen_port(), false);
    (*m_logger) << libtorrent::time_now_string() << "\n";

    m_thread.reset(new boost::thread(boost::ref(*this)));
}

void session_impl::operator()()
{
}

unsigned short session_impl::listen_port() const
{
}

void session_impl::on_disk_queue()
{
}

boost::shared_ptr<logger> session_impl::create_log(
    std::string const& name, int instance, bool append)
{
    return boost::shared_ptr<logger>(
        new logger(m_logpath, name + ".log", instance, append));
}
