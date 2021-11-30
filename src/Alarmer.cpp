#include "Alarmer.hpp"

#include "Protocol/Auth.hpp"
#include "Protocol/Alarm.hpp"
#include "Connection.hpp"
#include "Debug.hpp"

#define RETRY_SEC   5

Alarmer::Alarmer(const AlarmerArgs &a)
    : m_args(a) {
}

void Alarmer::runClient(BaseProtocol* p) {
    try {
        boost::asio::io_context io;
        boost::shared_ptr<Connection> client(new Connection(io));
        client->start(p, m_args.ip, m_args.port);
        io.run();
    } catch (std::exception& e) {
        debug_print("runClient exception: %s\n", e.what());
    }
}

bool Alarmer::authorize(AuthResult& auth) {
    bool authorized = false;

    AuthProtocol* p = new AuthProtocol(m_args.login, m_args.pass);
    p->setResultCallback([&auth, &authorized](const AuthResult& r) {
        auth = r;
        authorized = true;
    });

    runClient(p);
    return authorized;
}

void Alarmer::runAlarm(const AuthResult& auth) {
    AlarmProtocol *p = new AlarmProtocol(auth);
    p->setMotionCallback(m_motionCallback);
    runClient(p);
}

void Alarmer::run(const TMotionCallback& c) {
    m_motionCallback = c;
    AuthResult auth;

    while(1) {
        if (authorize(auth)) {
            runAlarm(auth);
        }

        sleep(RETRY_SEC);
        debug_print("After sleep\n");
    }
}
