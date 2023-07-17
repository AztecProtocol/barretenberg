// #pragma once
//
// #include "msgpack_apply.hpp"
//
//// Make sure this is available to serialize namespace
// namespace serialize {
//     /**
//      * @brief Automatically derived serialize::read for any object that defines .msgpack() (implicitly defined by
//      MSGPACK_FIELDS).
//      * @param it The iterator to read from.
//      * @param func The function to call with each field as an argument.
//      */
//     template <msgpack_concepts::HasMsgPack T> void read(uint8_t const*& it, T& obj)
//     {
//         msgpack::msgpack_apply(obj, [&](auto&... obj_fields) {
//             // apply 'read' to each object field
//             (read(it, obj_fields),...);
//         });
//     };
//     /**
//      * @brief Automatically derived serialize::write for any object that defines .msgpack() (implicitly defined by
//      MSGPACK_FIELDS).
//      * @param buf The buffer to write to.
//      * @param func The function to call with each field as an argument.
//      */
//     template <msgpack_concepts::HasMsgPack T> void write(std::vector<uint8_t>& buf, T& obj)
//     {
//         msgpack::msgpack_apply(obj, [&](auto&... obj_fields) {
//             // apply 'write' to each object field
//             (write(buf, obj_fields),...);
//         });
//     };
// } // namespace serialize
//
////namespace std {
////    // Hacky forwarding of our read and write methods above so that std helper functions can see them.
////    template <msgpack_concepts::HasMsgPack T> void read(uint8_t const*& it, T& obj)
////    {
////        serialize::read(it, obj);
////    }
////    template <msgpack_concepts::HasMsgPack T> void write(std::vector<uint8_t>& buf, T& obj)
////    {
////        serialize::write(buf, obj);
////    }
////}