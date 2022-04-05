#ifndef ALARM_PROTOCOL_HPP
#define ALARM_PROTOCOL_HPP

#include "Base.hpp"
#include "AuthResult.hpp"
#include "AlarmConstants.hpp"

typedef struct aes_key_st AES_KEY;

class AlarmProtocol : public BaseProtocol {
public:
    typedef uint8_t TMotion[AlarmConstants::MOTION_MAX_CHANNUM_V30];
    typedef std::function<void(const TMotion&, const size_t&)> TMotionCallback;

    AlarmProtocol(const AuthResult&);
    ~AlarmProtocol();

    void setMotionCallback(const TMotionCallback&);

private:
    TAwaitVoid start(ITransport*) override final;
    void pingHandler(const TCmdDesc&);
    void alarmHandler(const TCmdDesc&);
    void initAes128();
    TAwaitVoid sendAlarmSubscribe();
    uint32_t encChecksum();
    bool aes128Enc(const TData&, TData& d, int rounds = 10);
    void cmdHandler(const TCmdDesc& cmd);

    AES_KEY* m_aesKey;
    const AuthResult m_auth;
    TMotion m_motion;
    TMotionCallback m_motionCallback;
};

#endif /* ALARM_PROTOCOL_HPP */
