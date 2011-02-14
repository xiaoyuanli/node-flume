#include <v8.h>
#include <node.h>

#include <unistd.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "ThriftFlumeEventServer.h"
#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <sys/param.h>
#include <string>
#include <map>


using namespace node;
using namespace v8;

using namespace std;
using namespace flume_eio;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

#define MAX_HOST_NAME_SIZE 255

// FlumeLog class prototype
class FlumeLog: ObjectWrap
{
public:
  FlumeLog(Local<Value> flume_host, Local<Value> flume_port) :
	 flume_host_(flume_host),
	 flume_port_(flume_port)
  {
  }
  Local<Value> flume_host_, flume_port_;
  Local<Value> _GetFlumeHost();
  Local<Value> _GetFlumePort();

  ~FlumeLog(){};
};

Local<Value> FlumeLog::_GetFlumeHost()
{
  return flume_host_;
}

Local<Value> FlumeLog::_GetFlumePort()
{
  return flume_port_;
}

Handle<Value> GetFlumeHost(Local<String> property, const AccessorInfo& info)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  Local<Value> value = static_cast<FlumeLog*>(ptr)->flume_host_;
  return value;
}

Handle<Value> GetFlumePort(Local<String> property, const AccessorInfo& info)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  Local<Value> value = static_cast<FlumeLog*>(ptr)->flume_port_;
  return value;
}

void SetFlumeHost(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<FlumeLog*>(ptr)->flume_host_ = value->ToString();
}

void SetFlumePort(Local<String> property, Local<Value> value, const AccessorInfo& info)
{
  Local<Object> self = info.Holder();
  Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
  void* ptr = wrap->Value();
  static_cast<FlumeLog*>(ptr)->flume_port_ = value->ToString();
}

// Destructor
Persistent<FunctionTemplate> NodeFlumeLogTemplate;
void NodeFlumeLogDispose(Persistent<Value> object, void* parameter) {
	((FlumeLog*)External::Unwrap(object->ToObject()->GetInternalField(0)))->FlumeLog::~FlumeLog();
}

// Called via Javascript
// var flume = require('flume');
// var f = new flume.FlumeLog('localhost',35853);
// f.Log("my log message");
Handle<Value> NodeFlumeLog(const Arguments &args) {
	HandleScope scope;
	if(args.IsConstructCall()) {
		Local<Value> host = args[0]->IsUndefined() ? String::New("localhost")->ToString() : args[0]->ToString();
		Local<Value> port = args[1]->IsUndefined() ? String::New("35853")->ToString() : args[1]->ToString();

		FlumeLog* fl = new FlumeLog(host,port);
		Handle<Object> m = args.This();
		m->SetInternalField(0,External::New(fl));
		Persistent<Object>::New(m).MakeWeak(NULL,NodeFlumeLogDispose);
		return scope.Close(m);
	}
	else {
		return scope.Close(ThrowException(Exception::Error(String::New("Must use new operator"))));
	}
}

Handle<Value> NodeFlumeLog_Log(const Arguments &args) {
	HandleScope scope;
	Local<Value> flume_host = String::New("localhost");
	Local<Value> flume_port = String::New("35854");
//XXX	Local<Value> flume_host = ((FlumeLog*)External::Unwrap(args.Holder()->GetInternalField(0)))->_GetFlumeHost();
//XXX	Local<Value> flume_port = ((FlumeLog*)External::Unwrap(args.Holder()->GetInternalField(0)))->_GetFlumePort();

    Local<Value> message = args[0]->IsUndefined() ? String::New("") : args[0]->ToString();

    /* TODO Only use this if tags are specified
     * Tags should be passed as a hash and then iterated over
     */
    std::string tag_key, tag_value;
    bool has_tags = false;
    typedef map<string,string> eventTags;
		eventTags tags;
    if (!args[1]->IsUndefined()) {
        Local<Object> tagsObj = args[1]->ToObject();
        Local<Array> tagNames = tagsObj->GetPropertyNames();
        uint32_t length = tagNames->Length();

        // Iterate over the array
        for (uint32_t i=0;i<length;i++) {
            Local<String> v8TagKey = tagNames->Get(i)->ToString();
            Local<String> v8TagValue = tagsObj->Get(v8TagKey)->ToString();

            std::string tagKey(*String::AsciiValue(v8TagKey));
            std::string tagValue(*String::AsciiValue(v8TagValue));
            tags[tagKey] = tagValue;
        }

        // Set has_tags to true to ensure tags are added to metadata
        has_tags = true;
    }

    // Convert the hostname to string and test its length
    std::string hostString(*v8::String::Utf8Value(flume_host));
    v8::String::Utf8Value hostnameString(flume_host);
    if (0 != gethostname(*hostnameString, MAX_HOST_NAME_SIZE)) {
        return ThrowException(Exception::TypeError(String::New("Invalid hostname")));
    }

    boost::shared_ptr<TSocket> socket(new TSocket(hostString, flume_port->Uint32Value()));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    // TODO Write a lock/mutex here
    try {
        flume_eio::ThriftFlumeEventServerClient client(protocol);
        transport->open();

        flume_eio::ThriftFlumeEvent event;
        // INFO is the default priority
        // TODO Make this configurable on message
        event.priority = flume_eio::Priority::INFO;

// This code doesn't compile on Macs, so ignore it
#ifndef __APPLE__
        struct timespec t_nanos;
        if (0 == clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_nanos)) {
            event.nanos = t_nanos.tv_sec * 1000000000LL + t_nanos.tv_nsec;
        } else {
            return ThrowException(Exception::TypeError(String::New("Cannot read process cputime clock, quitting")));
        }

        struct timespec t_stamp;
        if (0 == clock_gettime(CLOCK_REALTIME, &t_stamp)) {
            event.timestamp = (int64_t)t_stamp.tv_sec * 1000; // timestamp is needed in milliseconds
        } else {
            return ThrowException(Exception::TypeError(String::New("Cannot read system clock")));
        }
#endif

        // Set the hostname and the message
        event.host = std::string(*hostnameString);
        std::string flume_message(*v8::String::AsciiValue(message));
        event.body = flume_message;

        // We have tags, so let's add them to the meta data
        if (has_tags == true) { event.fields = tags; }

        // Send the exent down the pipe and close the transport
        client.append(event);
        transport->close();
    } catch ( ... ) {
        return ThrowException(Exception::TypeError(String::New("Flume issue")));
    }

	return scope.Close(Undefined());
}


extern "C" void
init (Handle<Object> target)
{
  HandleScope scope;

  NodeFlumeLogTemplate = Persistent<FunctionTemplate>::New(FunctionTemplate::New(NodeFlumeLog));
  Handle<ObjectTemplate> NodeFlumeLogInstanceTemplate = NodeFlumeLogTemplate->InstanceTemplate();
  NodeFlumeLogInstanceTemplate->SetInternalFieldCount(1);
  NodeFlumeLogInstanceTemplate->Set("log", FunctionTemplate::New(NodeFlumeLog_Log));
  NodeFlumeLogInstanceTemplate->SetAccessor(String::New("flume_host"), GetFlumeHost, SetFlumeHost);
  NodeFlumeLogInstanceTemplate->SetAccessor(String::New("flume_port"), GetFlumePort, SetFlumePort);
  target->Set(String::New("FlumeLog"), NodeFlumeLogTemplate->GetFunction());
}
