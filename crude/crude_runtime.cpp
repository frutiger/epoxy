// crude_runtime.cpp
#include <crude_runtime.h>

#include <crude_assert.h>
#include <crude_convert.h>
#include <crude_context.h>
#include <crude_wrapper.h>

#include <libplatform/libplatform.h>
#include <v8.h>

#include <sstream>

namespace crude {

namespace {

struct FunctionData {
    const Runtime      *d_runtime_p;
    Runtime::Signature  d_function;
};

}

void Runtime::callback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    auto *isolate = info.GetIsolate();
    v8::HandleScope handles(isolate);

    CRUDE_ASSERT(info.Data()->IsExternal());
    auto  dataExternal = v8::External::Cast(*info.Data());
    auto *data         = static_cast<FunctionData *>(dataExternal->Value());

    auto *runtime = data->d_runtime_p;
    CRUDE_ASSERT(runtime->isolate() == isolate);

    auto context = isolate->GetCurrentContext();
    Values arguments;
    for (int i = 0; i < info.Length(); ++i) {
        arguments.push_back(Value(isolate, info[i]));
    }

    std::ostringstream errorStream;
    crude::Value       result;
    if (data->d_function(errorStream,
                         &result,
                         *runtime,
                         Context(isolate, context),
                         Object(isolate, info.This()),     // receiver
                         Value(isolate, info.NewTarget()), // target
                         arguments)) {
        Value error;
        Convert::from(errorStream,
                      &error,
                      *runtime,
                      Context(isolate, context),
                      errorStream.str());
        v8::Isolate::Scope scope(isolate);
        auto string = error.Get(isolate)->ToString(context).ToLocalChecked();
        isolate->ThrowException(v8::Exception::Error(string));
    }
    info.GetReturnValue().Set(result);
}

Runtime::Runtime()
{
    d_platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(d_platform.get());

    v8::V8::Initialize();
    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator =
                             v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    d_isolate_p = v8::Isolate::New(createParams);

    v8::HandleScope handles(d_isolate_p);
    d_wrapKey = v8::Global<v8::Symbol>(d_isolate_p,
                                       v8::Symbol::New(d_isolate_p));
}

Runtime::~Runtime()
{
    d_wrapKey.Reset();
    d_isolate_p->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
}

Context Runtime::createContext()
{
    v8::HandleScope handles(d_isolate_p);
    auto context = v8::Context::New(d_isolate_p);
    return Context(d_isolate_p, context);
}

int Runtime::compile(std::ostream&            errorStream,
                     Script                  *result,
                     const Context&           context,
                     const std::string_view&  name,
                     const std::string_view&  text)
{
    v8::HandleScope handles(d_isolate_p);

    Value nameValue;
    if (Convert::from(errorStream, &nameValue, *this, context, name)) {
        return -1;
    }

    Value textValue;
    if (Convert::from(errorStream, &textValue, *this, context, text)) {
        return -1;
    }

    auto textString = textValue.Get(d_isolate_p)
                               ->ToString(context.local())
                               .ToLocalChecked();

    v8::ScriptOrigin origin(nameValue.Get(d_isolate_p));
    auto source = v8::ScriptCompiler::Source(textString, origin);

    v8::MaybeLocal<v8::UnboundScript> maybeScript;
    {
        v8::Context::Scope scope(context.local());
        v8::TryCatch       handler(d_isolate_p);
        maybeScript = v8::ScriptCompiler::CompileUnboundScript(d_isolate_p,
                                                               &source);
        if (handler.HasCaught()) {
            Value message(d_isolate_p, handler.Message()->Get());
            std::string error;
            CRUDE_ASSERT(0 == Convert::to(errorStream,
                                          &error,
                                          *this,
                                          context,
                                          message));
            errorStream << error << '\n';
            return -1;
        }
    }

    if (maybeScript.IsEmpty()) {
        return -1;
    }
    *result = Script(d_isolate_p, maybeScript.ToLocalChecked());
    return 0;
}

int Runtime::evaluate(std::ostream&   errorStream,
                      Value          *result,
                      const Context&  context,
                      const Script&   script)
{
    v8::HandleScope handles(d_isolate_p);

    v8::MaybeLocal<v8::Value> maybeResult;
    {
        v8::Context::Scope scope(context.local());
        auto local = script.Get(d_isolate_p)->BindToCurrentContext();

        v8::TryCatch handler(d_isolate_p);
        maybeResult = local->Run(context.local());
        if (handler.HasCaught()) {
            Value message(d_isolate_p, handler.Message()->Get());
            std::string error;
            CRUDE_ASSERT(0 == Convert::to(errorStream,
                                          &error,
                                          *this,
                                          context,
                                          message));
            errorStream << error << '\n';
            return -1;
        }
    }

    if (maybeResult.IsEmpty()) {
        return -1;
    }
    *result = Value(d_isolate_p, maybeResult.ToLocalChecked());
    return 0;
}

int Runtime::host(Object           *result,
                  const Context&    context,
                  const Signature&  function)
{
    v8::HandleScope handles(d_isolate_p);

    auto *functionData = new FunctionData { this, function };
    auto data = v8::External::New(d_isolate_p, functionData);
    auto v8Function = v8::Function::New(context.local(),
                                        &Runtime::callback,
                                        data);
    if (v8Function.IsEmpty()) {
        return -1;
    }

    *result = Object(d_isolate_p, v8Function.ToLocalChecked());
    result->SetWeak(functionData,
                    [] (auto info) {
                        delete info.GetParameter();
                    },
                    v8::WeakCallbackType::kParameter);
    return 0;
}

Object Runtime::wrap(const crude::Context&      context,
                     std::unique_ptr<Wrapper>&& wrapper) const
{
    v8::HandleScope handles(d_isolate_p);

    Object result;
    {
        v8::Context::Scope scope(context.local());
        result = Object(d_isolate_p, v8::Object::New(d_isolate_p));
    }
    wrapper->hosted(result);

    auto *rawWrapper = wrapper.release();
    result.SetWeak(rawWrapper,
                   [] (auto info) {
                       delete info.GetParameter();
                   },
                   v8::WeakCallbackType::kParameter);

    {
        v8::Context::Scope scope(context.local());
        auto object = result.Get(d_isolate_p);
        object->Set(object->CreationContext(),
                    d_wrapKey.Get(d_isolate_p),
                    v8::External::New(d_isolate_p, rawWrapper)).Check();
    }
    return result;
}

Wrapper *Runtime::unwrap(const Object& object) const
{
    auto localObject = object.Get(d_isolate_p);
    auto key         = d_wrapKey.Get(d_isolate_p);

    auto isWrapped = localObject->Has(localObject->CreationContext(), key)
                     .FromMaybe(false);

    if (!isWrapped) {
        return nullptr;
    }

    auto wrapper = localObject->Get(localObject->CreationContext(), key)
                                .ToLocalChecked();
    CRUDE_ASSERT(wrapper->IsExternal());
    auto external = v8::External::Cast(*wrapper);
    return static_cast<Wrapper *>(external->Value());
}

v8::Isolate *Runtime::isolate() const
{
    return d_isolate_p;
}

}

