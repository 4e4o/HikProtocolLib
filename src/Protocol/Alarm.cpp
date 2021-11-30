#include "Alarm.hpp"
#include "Utils.hpp"
#include "Debug.hpp"

#include <arpa/inet.h>

#include <string>
#include <string.h>

#include <openssl/aes.h>

#define PING_CMD_ID                 0x2
#define ALARM_CMD_ID                0x68

#define MOTION_DETECTION_TYPE       0x3

#define MOTION_DATA_OFFSET          72
#define MOTION_DATA_SIZE            (MOTION_MAX_CHANNUM_V30 / 8)

#define AES_BLOCK_SIZE              16

#define MAGIC_CONST_1               2105
#define MAGIC_CONST_2               0x00111020

static constexpr uint8_t g_alarmHeader[] = { 0x63, 0x00, 0x00, 0x01,
                                             0xAA, 0xAA, 0xAA, 0xAA,     // checksum
                                             0x00, 0x11, 0x10, 0x20,
                                             0xf6, 0x01, 0xa8, 0xc0,
                                             0xAA, 0xAA, 0xAA, 0xAA,     // unknown id
                                             0x82, 0x64, 0x49, 0x01,
                                             0xee, 0xe8, 0x00, 0x00
                                           };

static constexpr uint8_t g_alarmEncBody[] = { 0x00, 0x00, 0x00, 0x14,
                                              0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x8c, 0x00,
                                              0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00
                                            };

static constexpr uint8_t g_checksumKey[] = { 0x82, 0x64, 0x49, 0x01,
                                             0xee, 0xe8, 0x00, 0x00,
                                             0xf6, 0x01, 0xa8, 0xc0
                                           };

AlarmProtocol::AlarmProtocol(const AuthResult& a)
    : m_aesKey(nullptr),
      m_auth(a) {
    debug_print("AlarmProtocol::AlarmProtocol %p\n", this);
}

AlarmProtocol::~AlarmProtocol() {
    if (m_aesKey) {
        delete m_aesKey;
        m_aesKey = nullptr;
    }

    debug_print("AlarmProtocol::~AlarmProtocol %p\n", this);
}

bool AlarmProtocol::start(ITransport* t) {
    return BaseProtocol::start(t) && initAes128() && sendAlarmSubscribe();
}

void AlarmProtocol::setMotionCallback(const TMotionCallback& c) {
    m_motionCallback = c;
}

bool AlarmProtocol::initAes128() {
    const std::string key = m_auth.key.substr(0, AES_BLOCK_SIZE);
    m_aesKey = new AES_KEY();
    AES_set_encrypt_key((const unsigned char *) key.data(), AES_BLOCK_SIZE * 8, m_aesKey);
    debug_print("AlarmProtocol::initAes128 %p, key: %s\n", this, key.c_str());
    return true;
}

bool AlarmProtocol::aes128Enc(const TData& in, TData& d, int rounds) {
    const size_t inSize = Utils::roundUp(in.size(), AES_BLOCK_SIZE);
    TData input(in.begin(), in.end());

    if (inSize > in.size())
        input.resize(inSize, 0);

    d.resize(input.size());
    m_aesKey->rounds = rounds;

    for (size_t i = 0 ; i < (input.size() / AES_BLOCK_SIZE) ; i++)
        AES_encrypt(input.data() + i * AES_BLOCK_SIZE,
                    d.data() + i * AES_BLOCK_SIZE, m_aesKey);

    return true;
}

uint32_t AlarmProtocol::encChecksum() {
    // TODO по пакету определил эту константу, надо бы в коде найти откуда она
    const uint32_t unknown_base = m_auth.secondU32 - MAGIC_CONST_1;
    uint32_t sum = m_auth.firstU32 + 2 * MAGIC_CONST_2;

    debug_print("AlarmProtocol::encChecksum %p, unknown_base = %u\n", this, unknown_base);

    for (int i = 0, shift = 0; i < 6 ; i++, shift += 5) {
        sum += g_checksumKey[i] & (m_auth.secondU32 >> shift);
    }

    TData encodedSum;

    {
        TData in((char*)&sum, ((char*)&sum) + sizeof(sum));

        if (!aes128Enc(in, encodedSum, 4))
            return false;
    }

    uint8_t sum2[4];
    memset(sum2, 0, sizeof(sum2));

    for (int i = 0 ; i < 4 ; i++) {
        for (int k = 0, j = i << 2; k < 4 ; k++, j++)
            sum2[k] ^= encodedSum[j];
    }

    const uint32_t encp = *((uint32_t*) sum2);
    return unknown_base + encp;
}

bool AlarmProtocol::sendAlarmSubscribe() {
    TData packet(g_alarmHeader, g_alarmHeader + sizeof(g_alarmHeader));

    *((uint32_t*) (packet.data() + 4)) = htonl(encChecksum());
    *((uint32_t*) (packet.data() + 16)) = htonl(m_auth.secondU32);

    TData d;

    if (!aes128Enc({g_alarmEncBody, g_alarmEncBody + sizeof(g_alarmEncBody)}, d))
        return false;

    packet.insert(packet.end(), d.begin(), d.end());
    sendCmd(packet, [this]() { readCmd(); });
    return true;
}

bool AlarmProtocol::pingHandler(const TCmdDesc&) {
    // debug_print("AlarmProtocol::pingHandler %p, %i\n", this, cmd.data.size());
    return true;
}

bool AlarmProtocol::alarmHandler(const TCmdDesc& cmd) {
    const uint32_t alarmType = readU32(cmd, 4);

    if (alarmType != MOTION_DETECTION_TYPE)
        return false;

    if (cmd.data.size() < (MOTION_DATA_OFFSET + MOTION_DATA_SIZE))
        return false;

    size_t mdSize = 0;

    for (int i = 0 ; i < MOTION_DATA_SIZE ; i++) {
        const uint8_t data = cmd.data.data()[MOTION_DATA_OFFSET + i];

        if (data == 0)
            continue;

        for (int j = 0 ; j < 8 ; j++) {
            if (data & (1 << j)) {
                const int channel = i * 8 + j;
                m_motion[mdSize++] = channel;
            }
        }
    }

    if ((mdSize > 0) && m_motionCallback) {
        m_motionCallback(m_motion, mdSize);
    }

    return true;
}

bool AlarmProtocol::onCmd(const TCmdDesc& cmd) {
    // debug_print("AlarmProtocol::onCmd %p, %i\n", this, cmd.data.size());
    bool handlerResult = true;

    switch(cmd.firstU32) {
    case PING_CMD_ID: {
        handlerResult = pingHandler(cmd);
        break;
    }
    case ALARM_CMD_ID: {
        handlerResult = alarmHandler(cmd);
        break;
    }
    }

    if (!handlerResult)
        return false;

    readCmd();
    return true;
}
