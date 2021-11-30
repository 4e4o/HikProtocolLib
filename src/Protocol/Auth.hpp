#ifndef AUTH_PROTOCOL_HPP
#define AUTH_PROTOCOL_HPP

#include "Base.hpp"
#include "AuthResult.hpp"

typedef struct bignum_st BIGNUM;
typedef struct rsa_st RSA;

class AuthProtocol : public BaseProtocol {
public:
    typedef std::function<void(const AuthResult&)> TResultCallback;

    AuthProtocol(const std::string& login, const std::string& pass);
    ~AuthProtocol();

    void setResultCallback(TResultCallback);

private:
    bool start(ITransport*) override final;
    bool onCmd(const TCmdDesc&) override final;

    bool genRSAKeys();
    bool sendFirstPacket();
    bool sendSecondPacket();
    void genFirstPacket(TData& d);
    bool genSecondPacket(TData& d);
    bool decryptKeyRSA(const TCmdDesc&);
    bool decryptKeyBase64(const TCmdDesc&);
    std::string hmac(const std::string&);
    bool keyPacketHandler(const TCmdDesc&);
    bool resultPacketHandler(const TCmdDesc&);

    enum class State {
        SEND_RSA_PUB_KEY,
        SEND_HMAC,
        LOGGED_IN
    };

    const std::string m_login;
    const std::string m_pass;
    BIGNUM *m_bn;
    RSA *m_rsa;
    TData m_pubKey;
    State m_state;
    std::string m_receivedKey;
    std::string m_salt;
    TResultCallback m_resultCallback;
};

#endif /* AUTH_PROTOCOL_HPP */
