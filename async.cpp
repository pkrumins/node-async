#include <cstdlib>
#include <node.h>
#include <node_buffer.h>

using namespace v8;
using namespace node;

class Async : public ObjectWrap {
    static Persistent<FunctionTemplate> constructor_template;
    int x, y;

public:
    Async(int xx, int yy) : x(xx), y(yy) {}

    static void Initialize(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);
        constructor_template = Persistent<FunctionTemplate>::New(t);
        constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
        constructor_template->SetClassName(String::NewSymbol("Async"));

        NODE_SET_PROTOTYPE_METHOD(constructor_template, "moo", Moo);
        target->Set(String::NewSymbol("Async"), constructor_template->GetFunction());
    }

    static Handle<Value> New(const Arguments &args) {
        HandleScope scope;

        Async *async = new Async(args[0]->Int32Value(), args[1]->Int32Value());
        async->Wrap(args.This());
        return args.This();
    }

    struct moo_request {
        Persistent<Function> cb;
        Async *async;
        int x, y;
    };

    static int EIO_Moo(eio_req *req) {
        //
        // req->data pointer gets messed up!!!
        //
        printf("%x %x\n", req, req->data);
        moo_request *moo_req = (moo_request *)req->data;
        moo_req->x = 11;
        moo_req->y = 4;
        return 0;
    }

    static int EIO_MooAfter(eio_req *req) {
        HandleScope scope;

        ev_unref(EV_DEFAULT_UC);
        moo_request *moo_req = (moo_request *)req->data;

        Local<Value> argv[1];
        argv[0] = Integer::New(moo_req->x * moo_req->y);

        TryCatch try_catch;

        moo_req->cb->Call(Context::GetCurrent()->Global(), 1, argv);

        if (try_catch.HasCaught())
            FatalException(try_catch);

        moo_req->cb.Dispose();

        moo_req->async->Unref();
        free(moo_req);

        return 0;
    }

    static Handle<Value> Moo(const Arguments &args) {
        HandleScope scope;

        Local<Function> cb = Local<Function>::Cast(args[0]);
        Async *async = ObjectWrap::Unwrap<Async>(args.This());

        moo_request *moo_req = (moo_request *)malloc(sizeof(moo_request));
        moo_req->cb = Persistent<Function>::New(cb);
        moo_req->async = async;

        eio_custom(EIO_Moo, EIO_PRI_DEFAULT, EIO_MooAfter, moo_req);

        ev_ref(EV_DEFAULT_UC);
        async->Ref();

        return Undefined();
    }
};

Persistent<FunctionTemplate> Async::constructor_template;

extern "C" void
init(Handle<Object> target)
{
    HandleScope scope;

    Async::Initialize(target);
}

