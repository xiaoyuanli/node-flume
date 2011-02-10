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

#define HOST_NAME_MAX_SIZE 255

#define REQ_FUN_ARG(I, VAR)                                             \
	if (args.Length() <= (I) || !args[I]->IsFunction())                   \
	return ThrowException(Exception::TypeError(                         \
	String::New("Argument " #I " must be a function")));  \
	Local<Function> VAR = Local<Function>::Cast(args[I]);


class FlumeLogEio: ObjectWrap
{
private:
  Local<Value> flume_host;
  Local<Value> flume_port;
public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target)
  {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    s_ct->SetClassName(String::NewSymbol("FlumeLogEio"));

    NODE_SET_PROTOTYPE_METHOD(s_ct, "log", Log);

    target->Set(String::NewSymbol("FlumeLogEio"),
                s_ct->GetFunction());
  }

  FlumeLogEio() :
    flume_host(String::New("localhost")),
    flume_port(String::New("35853"))
  {
  }

  ~FlumeLogEio()
  {
  }

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;

    if (args.IsConstructCall()) {
      FlumeLogEio* fl = new FlumeLogEio();
      fl->Wrap(args.This());

      return args.This();
    }
	else {
	  return scope.Close(ThrowException(Exception::Error(String::New("Must use new operator"))));
	}

  }

  struct flume_baton_t {
    FlumeLogEio *fl;
    Local<Value> message;
    Persistent<Function> cb;
  };

  static Handle<Value> Log(const Arguments& args)
  {
    HandleScope scope;


    FlumeLogEio* fl = ObjectWrap::Unwrap<FlumeLogEio>(args.This());

    //char* message = args[0]->IsUndefined() ? (char*)"" : *String::Utf8Value(args[1]);

    // TODO Ensure function argument (arg[0]) is a string
    // TODO Since tags are optional, tag_key/tag_val should be used if exist

	REQ_FUN_ARG(0, cb);

    flume_baton_t *baton = new flume_baton_t();
    baton->fl = fl;
    baton->cb = Persistent<Function>::New(cb);

    fl->Ref();

    // TODO Move all the v8 string conversions here

    eio_custom(EIO_Log, EIO_PRI_DEFAULT, EIO_AfterLog, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }


  static int EIO_Log(eio_req *req)
  {
    flume_baton_t *baton = static_cast<flume_baton_t *>(req->data);

    // XXX Write a lock/mutex here
    std::string hostString(*v8::String::Utf8Value(baton->fl->flume_host));
    boost::shared_ptr<TSocket> socket(new TSocket(hostString, baton->fl->flume_port->Uint32Value()));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    v8::String::Utf8Value hostnameString(baton->fl->flume_host);
    if (0 != gethostname(*hostnameString, HOST_NAME_MAX_SIZE)) {
      //return ThrowException(Exception::TypeError(String::New("Invalid hostname")));
      return -1;
    }

    /* TODO Only use this if tags are specified
    map<string,string> tag;
    tag[tag_key] = tag_value;
    */

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
            //return ThrowException(Exception::TypeError(String::New("Cannot read process cputime clock, quitting")));
            return -1;
        }

        struct timespec t_stamp;
        if(0 == clock_gettime(CLOCK_REALTIME, &t_stamp)) {
            event.timestamp = (int64_t)t_stamp.tv_sec * 1000; // timestamp is needed in milliseconds
        } else {
            //return ThrowException(Exception::TypeError(String::New("Cannot read system clock")));
            return -1;
        }
#endif

        event.host = std::string(*hostnameString);

        std::string message(*v8::String::Utf8Value(baton->message));
        event.body = message;

        // if tags exist {
        // event.fields = tag
        // }
        client.append(event);

        transport->close();
    } catch ( ... ) {
      //ThrowException(Exception::TypeError(String::New("Flume issue")));
      return -1;
    }

    return 0;
  }

  static int EIO_AfterLog(eio_req *req)
  {
    HandleScope scope;
    flume_baton_t *baton = static_cast<flume_baton_t *>(req->data);
    ev_unref(EV_DEFAULT_UC);
    baton->fl->Unref();

    Local<Value> argv[1];

    TryCatch try_catch;

    baton->cb->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    baton->cb.Dispose();

    delete baton;
    return 0;
  }

};

Persistent<FunctionTemplate> FlumeLogEio::s_ct;

extern "C" {
  static void init (Handle<Object> target)
  {
    FlumeLogEio::Init(target);
  }

  NODE_MODULE(flume_eio, init);
}
