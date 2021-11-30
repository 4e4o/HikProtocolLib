#ifndef ALARMER_H
#define ALARMER_H

#include "Protocol/Alarm.hpp"
#include "AlarmerArgs.hpp"

class Alarmer {
public:
    using TMotionCallback = AlarmProtocol::TMotionCallback;

    Alarmer(const AlarmerArgs&);

    void run(const TMotionCallback&);

private:
    void runClient(BaseProtocol* p);
    bool authorize(AuthResult& auth);
    void runAlarm(const AuthResult& auth);

    AlarmerArgs m_args;
    TMotionCallback m_motionCallback;
};

#endif /* ALARMER_H */
