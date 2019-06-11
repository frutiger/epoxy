// crude_runtime.cpp
#include <crude_runtime.h>

#include <libplatform/libplatform.h>
#include <v8.h>

namespace crude {

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

    context.Get(d_isolate_p)->Enter();
    auto maybeScript = v8::ScriptCompiler::CompileUnboundScript(d_isolate_p,
                                                                &source);
    context.Get(d_isolate_p)->Exit();

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
    context.Get(d_isolate_p)->Enter();
    auto local = script.Get(d_isolate_p)->BindToCurrentContext();
    auto maybeResult = local->Run(context.Get(d_isolate_p));
    context.Get(d_isolate_p)->Exit();

    if (maybeResult.IsEmpty()) {
        return -1;
    }
    *result = Value(d_isolate_p, maybeResult.ToLocalChecked());
    return 0;
}

v8::Isolate *Runtime::isolate() const
{
    return d_isolate_p;
}

}

