/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 */
#ifndef flume_eio_TYPES_H
#define flume_eio_TYPES_H

#include <Thrift.h>
#include <TApplicationException.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>



namespace flume_eio {

struct Priority {
  enum type {
    FATAL = 0,
    ERROR = 1,
    WARN = 2,
    INFO = 3,
    DEBUG = 4,
    TRACE = 5
  };
};

extern const std::map<int, const char*> _Priority_VALUES_TO_NAMES;

struct EventStatus {
  enum type {
    ACK = 0,
    COMMITED = 1,
    ERR = 2
  };
};

extern const std::map<int, const char*> _EventStatus_VALUES_TO_NAMES;

typedef int64_t Timestamp;

typedef struct _ThriftFlumeEvent__isset {
  _ThriftFlumeEvent__isset() : timestamp(false), priority(false), body(false), nanos(false), host(false), fields(false) {}
  bool timestamp;
  bool priority;
  bool body;
  bool nanos;
  bool host;
  bool fields;
} _ThriftFlumeEvent__isset;

class ThriftFlumeEvent {
 public:

  static const char* ascii_fingerprint; // = "BC13FFB3246A0179557F7F60C181A557";
  static const uint8_t binary_fingerprint[16]; // = {0xBC,0x13,0xFF,0xB3,0x24,0x6A,0x01,0x79,0x55,0x7F,0x7F,0x60,0xC1,0x81,0xA5,0x57};

  ThriftFlumeEvent() : timestamp(0), body(""), nanos(0), host("") {
  }

  virtual ~ThriftFlumeEvent() throw() {}

  Timestamp timestamp;
  Priority::type priority;
  std::string body;
  int64_t nanos;
  std::string host;
  std::map<std::string, std::string>  fields;

  _ThriftFlumeEvent__isset __isset;

  bool operator == (const ThriftFlumeEvent & rhs) const
  {
    if (!(timestamp == rhs.timestamp))
      return false;
    if (!(priority == rhs.priority))
      return false;
    if (!(body == rhs.body))
      return false;
    if (!(nanos == rhs.nanos))
      return false;
    if (!(host == rhs.host))
      return false;
    if (!(fields == rhs.fields))
      return false;
    return true;
  }
  bool operator != (const ThriftFlumeEvent &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ThriftFlumeEvent & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _RawEvent__isset {
  _RawEvent__isset() : raw(false) {}
  bool raw;
} _RawEvent__isset;

class RawEvent {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  RawEvent() : raw("") {
  }

  virtual ~RawEvent() throw() {}

  std::string raw;

  _RawEvent__isset __isset;

  bool operator == (const RawEvent & rhs) const
  {
    if (!(raw == rhs.raw))
      return false;
    return true;
  }
  bool operator != (const RawEvent &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RawEvent & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

} // namespace

#endif
