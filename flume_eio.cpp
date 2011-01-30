/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
* LICENSE file.
*/

#include <v8.h>
#include <node.h>

#include <unistd.h>

using namespace node;
using namespace v8;

#define REQ_FUN_ARG(I, VAR) \
  if (args.Length() <= (I) || !args[I]->IsFunction()) \
    return ThrowException(Exception::TypeError( \
        String::New("Argument " #I " must be a function"))); \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

class FlumeLogEio: ObjectWrap
{
private:
  int m_count;
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
    m_count(0)
  {
  }

  ~FlumeLogEio()
  {
  }

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    FlumeLogEio* fl = new FlumeLogEio();
    fl->Wrap(args.This());
    return args.This();
  }

  struct flume_baton_t {
    FlumeLogEio *fl;
    Local<String> message;
    Persistent<Function> cb;
  };

  static Handle<Value> Log(const Arguments& args)
  {
    HandleScope scope;

    REQ_FUN_ARG(0, cb);

    FlumeLogEio* fl = ObjectWrap::Unwrap<FlumeLogEio>(args.This());

    flume_baton_t *baton = new flume_baton_t();
    baton->fl = fl;
    baton->message = "XXX Need to make this be the function argument";
    baton->cb = Persistent<Function>::New(cb);

    fl->Ref();

    eio_custom(EIO_Log, EIO_PRI_DEFAULT, EIO_AfterLog, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }


  static int EIO_Log(eio_req *req)
  {
    flume_baton_t *baton = static_cast<flume_baton_t *>(req->data);

    // XXX Write a lock/mutex here
    logToFlume(baton->message);

    return 0;
  }

  static int EIO_AfterLog(eio_req *req)
  {
    HandleScope scope;
    flume_baton_t *baton = static_cast<flume_baton_t *>(req->data);
    ev_unref(EV_DEFAULT_UC);
    baton->fl->Unref();

    Local<Value> argv[1];

    argv[0] = String::New("XXX Need to make this be the function argument");

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
