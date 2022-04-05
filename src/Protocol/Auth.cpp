#include "Auth.hpp"
#include "Misc/Utils.hpp"

#include <Misc/Debug.hpp>

#include <openssl/rsa.h>
#include <openssl/hmac.h>

#define RCA_KEY_BITS                        1024

#define PUBKEY_PACKET_LOGIN_SIZE            48
#define RSA_1024_F4_PUBKEY_SIZE             140

#define CRYPTED_KEY_PACKET_HEADER_SIZE      12
#define RSA_CRYPTED_KEY_LENGTH              128
#define DECRYPTED_KEY_LENGTH                32

#define HMAC_MD5_LENGTH                     16

#define RSA_KEY_PACKET_TYPE                 0x00000063
#define RSA_SALT_KEY_PACKET_TYPE            0x01000063
#define BASE64_KEY_PACKET_TYPE              0x00000064

static const uint8_t g_loginCmdHeader[] = { 0x5a, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x01, 0x00, 0x00,
                                            0x05, 0x01, 0x3d, 0x4b,
                                            0x00, 0x00, 0x01, 0x01,
                                            0xf6, 0x01, 0xa8, 0xc0,
                                            0x82, 0x64, 0x49, 0x01,
                                            0xee, 0xe8, 0x6f, 0x00
                                          };

// для первого пакета:
// потом 48 байт логина
// потом 140 байт pubkey asn1

// для второго пакета:
// hmac md5(login, decrypted_key)
// 16 нулей
// hmac md5(password, decrypted_key)

AuthProtocol::AuthProtocol(const std::string& login, const std::string& pass)
    : m_login(login),
      m_pass(pass),
      m_bn(nullptr),
      m_rsa(nullptr),
      m_pubKey(RSA_1024_F4_PUBKEY_SIZE) {
    debug_print_this("");
}

AuthProtocol::~AuthProtocol() {
    if (m_rsa) {
        RSA_free(m_rsa);
        m_rsa = nullptr;
    }

    if (m_bn) {
        BN_free(m_bn);
        m_bn = nullptr;
    }

    debug_print_this("");
}

TAwaitVoid AuthProtocol::start(ITransport* t) {
    co_await BaseProtocol::start(t);
    co_await sendFirstPacket();
    keyPacketHandler(co_await readCmd());
    co_await sendSecondPacket();
    resultPacketHandler(co_await readCmd());
    co_return;
}

const AuthResult& AuthProtocol::result() const {
    return m_result;
}

bool AuthProtocol::genRSAKeys() {
    m_bn = BN_new();
    m_rsa = RSA_new();

    BN_set_word(m_bn, RSA_F4);

    if (!RSA_generate_key_ex(m_rsa, RCA_KEY_BITS, m_bn, nullptr))
        return false;

    uint8_t* pubP = m_pubKey.data();

    i2d_RSAPublicKey(m_rsa, &pubP);

    const size_t pubSize = pubP - m_pubKey.data();

    if (pubSize != RSA_1024_F4_PUBKEY_SIZE)
        return false;

    m_pubKey.resize(pubSize);
    return true;
}

TAwaitVoid AuthProtocol::sendFirstPacket() {
    if (!genRSAKeys())
        throw std::runtime_error("genRSAKeys failed");

    TData d;
    genFirstPacket(d);
    co_await sendCmd(d);
    co_return;
}

void AuthProtocol::genFirstPacket(TData& d) {
    d.insert(d.end(), g_loginCmdHeader, g_loginCmdHeader + sizeof(g_loginCmdHeader));

    {
        TData login(m_login.begin(), m_login.end());

        if (login.size() < PUBKEY_PACKET_LOGIN_SIZE)
            login.resize(PUBKEY_PACKET_LOGIN_SIZE, 0);

        d.insert(d.end(), login.begin(), login.end());
    }

    d.insert(d.end(), m_pubKey.begin(), m_pubKey.end());
}

std::string AuthProtocol::hmac(const std::string& data) {
    unsigned char* hmac = HMAC(EVP_md5(), m_receivedKey.data(), m_receivedKey.size(),
                               (unsigned char*) data.data(), data.length(),
                               NULL, NULL);

    if (hmac == nullptr)
        return "";

    return std::string(hmac, hmac + HMAC_MD5_LENGTH);
}

bool AuthProtocol::genSecondPacket(TData& d) {
    d.insert(d.end(), g_loginCmdHeader, g_loginCmdHeader + sizeof(g_loginCmdHeader));

    const std::string loginHmac = hmac(m_login);
    const std::string secondHmac = hmac(m_salt.empty() ? m_pass : Utils::sha256(m_login + m_salt + m_pass));

    if (loginHmac.empty() || secondHmac.empty())
        return false;

    d.insert(d.end(), loginHmac.begin(), loginHmac.end());
    d.resize(d.size() + HMAC_MD5_LENGTH, 0);
    d.insert(d.end(), secondHmac.begin(), secondHmac.end());
    return true;
}

TAwaitVoid AuthProtocol::sendSecondPacket() {
    debug_print_this(fmt("decrypted key: %1%") % m_receivedKey.c_str());
    TData d;

    if (!genSecondPacket(d))
        throw std::runtime_error("genSecondPacket failed");

    co_await sendCmd(d);
    co_return;
}

bool AuthProtocol::decryptKeyRSA(const TCmdDesc& cmd) {
    if (cmd.data.size() < (CRYPTED_KEY_PACKET_HEADER_SIZE + RSA_CRYPTED_KEY_LENGTH))
        return false;

    const uint8_t *crypted_data = cmd.data.data() + CRYPTED_KEY_PACKET_HEADER_SIZE;
    TData crypted(crypted_data, crypted_data + RSA_CRYPTED_KEY_LENGTH);
    TData result(crypted.size());
    const int len = RSA_private_decrypt(crypted.size(), crypted.data(), result.data(), m_rsa, RSA_PKCS1_PADDING);

    if (len != DECRYPTED_KEY_LENGTH)
        return false;

    result.resize(len);
    m_receivedKey = std::string(result.begin(), result.end());

    const size_t salt_length = cmd.data.size() - CRYPTED_KEY_PACKET_HEADER_SIZE - RSA_CRYPTED_KEY_LENGTH;

    if (salt_length > 0) {
        const uint8_t *salt_begin = cmd.data.data() + CRYPTED_KEY_PACKET_HEADER_SIZE + RSA_CRYPTED_KEY_LENGTH;
        m_salt = std::string(salt_begin, salt_begin + salt_length);
    }

    return true;
}

bool AuthProtocol::decryptKeyBase64(const TCmdDesc& cmd) {
    size_t crypted_data_length = cmd.data.size() - CRYPTED_KEY_PACKET_HEADER_SIZE;

    if (crypted_data_length < 1)
        return false;

    const uint8_t *begin = cmd.data.data() + CRYPTED_KEY_PACKET_HEADER_SIZE;
    const uint8_t *end = begin + crypted_data_length;
    const uint8_t* zero_pos = std::find(begin, end, 0);

    if (zero_pos == end)
        return false;

    crypted_data_length = zero_pos - begin;

    if (crypted_data_length < 1)
        return false;

    const std::string base64(begin, begin + crypted_data_length);
    const std::string result = Utils::base64Decode(base64);

    if (result.length() != DECRYPTED_KEY_LENGTH)
        return false;

    m_receivedKey = result;
    return true;
}

//  TODO 47

void AuthProtocol::keyPacketHandler(const TCmdDesc& cmd) {
    if (cmd.data.size() < CRYPTED_KEY_PACKET_HEADER_SIZE)
        throw std::runtime_error("invalid key packet header size");

    const uint32_t packetType = readU32(cmd, 4);
    bool keyDecrypted = false;

    switch(packetType) {
    case RSA_KEY_PACKET_TYPE:
    case RSA_SALT_KEY_PACKET_TYPE:
        keyDecrypted = decryptKeyRSA(cmd);
        break;
    case BASE64_KEY_PACKET_TYPE:
        keyDecrypted = decryptKeyBase64(cmd);
        break;
    default: {
        throw std::runtime_error("Unimplemented key packet");
    }
    }

    if (!keyDecrypted)
        throw std::runtime_error("Key not decrypted");
}

void AuthProtocol::resultPacketHandler(const TCmdDesc& cmd) {
    if (cmd.data.size() < 16)
        throw std::runtime_error("Result packet invalid data length");

    m_result.key = m_receivedKey;
    m_result.firstU32 = cmd.firstU32;
    m_result.secondU32 = readU32(cmd, 12);
}
