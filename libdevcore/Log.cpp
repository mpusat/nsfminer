
#include "Log.h"

#include <map>
#include <thread>

using namespace std;
using namespace dev;

// Logging
unsigned g_logOptions = 0;
bool g_logNoColor = false;
bool g_logSyslog = false;
bool g_logStdout = false;

bool LogChannel::name()
{
    return false;
}
bool WarnChannel::name()
{
    return true;
}
bool NoteChannel::name()
{
    return false;
}

LogOutputStreamBase::LogOutputStreamBase(bool error)
{
    static locale logLocl = locale("");
    m_sstr.imbue(logLocl);
    if (g_logSyslog)
        m_sstr << left << setw(8) << getThreadName() << " " EthReset;
    else
    {
        time_t rawTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        char buf[24];
        if (strftime(buf, 24, "%X", localtime(&rawTime)) == 0)
            buf[0] = '\0';  // empty if case strftime fails
        m_sstr << (error ? EthRed : EthCoalBold) << buf << " " EthWhite << left << setw(5)
               << getThreadName() << " " EthReset;
        }
}

/// Associate a name with each thread for nice logging.
struct ThreadLocalLogName
{
    ThreadLocalLogName(char const* _name) { name = _name; }
    thread_local static char const* name;
};

thread_local char const* ThreadLocalLogName::name;

ThreadLocalLogName g_logThreadName("main");

string dev::getThreadName()
{
#if defined(__linux__)
    char buffer[128];
    pthread_getname_np(pthread_self(), buffer, 127);
    buffer[127] = 0;
    return buffer;
#else
    return ThreadLocalLogName::name ? ThreadLocalLogName::name : "<unknown>";
#endif
}

void dev::setThreadName(char const* _n)
{
#if defined(__linux__)
    pthread_setname_np(pthread_self(), _n);
#else
    ThreadLocalLogName::name = _n;
#endif
}

void dev::simpleDebugOut(string const& _s)
{
    try
    {
        ostream& os = g_logStdout ? cout : clog;
        if (!g_logNoColor)
        {
            os << _s + '\n';
            os.flush();
            return;
        }
        bool skip = false;
        stringstream ss;
        for (auto it : _s)
        {
            if (!skip && it == '\x1b')
                skip = true;
            else if (skip && it == 'm')
                skip = false;
            else if (!skip)
                ss << it;
        }
        ss << '\n';
        os << ss.str();
        os.flush();
    }
    catch (...)
    {
        return;
    }
}
