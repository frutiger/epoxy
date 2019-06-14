// crude_runtime.h
#pragma once

#include <v8.h>

#include <functional>
#include <string>
#include <vector>

namespace crude {

using Value  = v8::Global<v8::Value>;
using Values = std::vector<Value>;
using Script = v8::Global<v8::UnboundScript>;

class Context;

class Runtime
{
    std::unique_ptr<v8::Platform>  d_platform;
    v8::Isolate                   *d_isolate_p;

  public:
    using Signature = std::function<Value (const Runtime& runtime,
                                           const Object&  receiver,
                                           const Value&   target,
                                           const Values&  arguments)>;

    static void callback(const v8::FunctionCallbackInfo<v8::Value>& info);

    Runtime();
    ~Runtime();

    Context createContext();
    int compile(Script                  *result,
                const Context&           context,
                const std::string_view&  name,
                const std::string_view&  text);
    int evaluate(Value *result, const Context& context, const Script& script);

    int host(Object           *result,
             const Context&    context,
             const Signature&  function);

    v8::Isolate *isolate() const;
};

}

