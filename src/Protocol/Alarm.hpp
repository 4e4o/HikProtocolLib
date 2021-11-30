#ifndef ALARM_PROTOCOL_HPP
#define ALARM_PROTOCOL_HPP

#include "Base.hpp"
#include "AuthResult.hpp"

typedef struct aes_key_st AES_KEY;

class AlarmProtocol : public BaseProtocol {
public:
    static constexpr int MOTION_MAX_CHANNUM_V30 = 64;

    typedef uint8_t TMotion[MOTION_MAX_CHANNUM_V30];
    typedef std::function<void(const TMotion&, const size_t&)> TMotionCallback;

    AlarmProtocol(const AuthResult&);
    ~AlarmProtocol();

    void setMotionCallback(const TMotionCallback&);

private:
    bool start(ITransport*) override final;
    bool onCmd(const TCmdDesc&) override final;
    bool pingHandler(const TCmdDesc&);
    bool alarmHandler(const TCmdDesc&);

    bool initAes128();
    bool sendAlarmSubscribe();
    uint32_t encChecksum();
    bool aes128Enc(const TData&, TData& d, int rounds = 10);

    AES_KEY* m_aesKey;
    const AuthResult m_auth;
    TMotion m_motion;
    TMotionCallback m_motionCallback;
};

#endif /* ALARM_PROTOCOL_HPP */
