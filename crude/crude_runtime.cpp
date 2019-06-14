// crude_runtime.cpp
#include <crude_runtime.h>

#include <crude_context.h>

#include <libplatform/libplatform.h>
#include <v8.h>

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

    if (!info.Data()->IsExternal()) {
        std::abort();
    }
    auto  dataExternal = v8::External::Cast(*info.Data());
    auto *data         = static_cast<FunctionData *>(dataExternal->Value());

    auto *runtime = data->d_runtime_p;
    if (runtime->isolate() != isolate) {
        std::abort();
    }

    Values arguments;
    for (int i = 0; i < info.Length(); ++i) {
        arguments.push_back(Value(isolate, info[i]));
    }

    Object receiver(isolate, info.This());
    Value  target(isolate, info.NewTarget());

    info.GetReturnValue().Set(data->d_function(*runtime,
                                               receiver,
                                               target,
                                               arguments));
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
}

Runtime::~Runtime()
{
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

int Runtime::compile(Script                  *result,
                     const Context&           context,
                     const std::string_view&  name,
                     const std::string_view&  text)
{
    v8::HandleScope handles(d_isolate_p);

    auto nameString = v8::String::NewFromUtf8(d_isolate_p,
                                              name.begin(),
                                              v8::NewStringType::kNormal,
                                              name.length());
    if (nameString.IsEmpty()) {
        return -1;
    }

    v8::ScriptOrigin origin (nameString.ToLocalChecked());

    auto textString = v8::String::NewFromUtf8(d_isolate_p,
                                              text.begin(),
                                              v8::NewStringType::kNormal,
                                              text.length());
    if (textString.IsEmpty()) {
        return -1;
    }


    auto source = v8::ScriptCompiler::Source(textString.ToLocalChecked(),
                                             origin);

    context.local()->Enter();
    auto maybeScript = v8::ScriptCompiler::CompileUnboundScript(d_isolate_p,
                                                                &source);
    context.local()->Exit();

    if (maybeScript.IsEmpty()) {
        return -1;
    }
    *result = Script(d_isolate_p, maybeScript.ToLocalChecked());
    return 0;
}

int Runtime::evaluate(Value          *result,
                      const Context&  context,
                      const Script&   script)
{
    v8::HandleScope handles(d_isolate_p);
    context.local()->Enter();
    auto local = script.Get(d_isolate_p)->BindToCurrentContext();
    auto maybeResult = local->Run(context.local());
    context.local()->Exit();

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

v8::Isolate *Runtime::isolate() const
{
    return d_isolate_p;
}

}

