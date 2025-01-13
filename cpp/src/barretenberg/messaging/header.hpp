#pragma once
#include "barretenberg/serialize/msgpack.hpp"
#include <cstdint>
#include <cstring>

namespace bb::messaging {

enum SystemMsgTypes { TERMINATE = 0, PING = 1, PONG = 2 };

const uint32_t FIRST_APP_MSG_TYPE = 100;

// #pragma pack(push, 1)
struct MsgHeader {
    uint32_t messageId; // Unique Id for the message
    uint32_t requestId; // Id of the message this is responding too (may not be used)

    MSGPACK_FIELDS(messageId, requestId);

    MsgHeader() = default;

    MsgHeader(uint32_t reqId)
        : requestId(reqId)
    {}

    MsgHeader(uint32_t msgId, uint32_t reqId)
        : messageId(msgId)
        , requestId(reqId)
    {}
};

struct HeaderOnlyMessage {
    uint32_t msgType;
    MsgHeader header;

    HeaderOnlyMessage(uint32_t type, MsgHeader& hdr)
        : msgType(type)
        , header(hdr)
    {}

    HeaderOnlyMessage() = default;

    MSGPACK_FIELDS(msgType, header);
};

template <typename T>
struct TypedMessage {
    uint32_t msgType;  // The type of the message.
    MsgHeader header;  // The header of the message.
    T value;           // The actual message content.

    // Constructor to initialize the message with type, header, and value.
    TypedMessage(uint32_t type, const MsgHeader& hdr, const T& val)
        : msgType(type), header(hdr), value(val) {}

    // Default constructor for creating an empty message.
    TypedMessage() = default;

    // Serialization support using msgpack.
    MSGPACK_FIELDS(msgType, header, value);
};


// #pragma pack(pop)
} // namespace bb::messaging
