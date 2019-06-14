// crude_runtime.h
#pragma once

#include <v8.h>

#include <functional>
#include <string>
#include <vector>

namespace crude {

using Object = v8::Global<v8::Object>;
using Value  = v8::Global<v8::Value>;
using Values = std::vector<Value>;
using Script = v8::Global<v8::UnboundScript>;

class Context;
class Wrapper;

class Runtime
{
    std::unique_ptr<v8::Platform>  d_platform;
    v8::Isolate                   *d_isolate_p;
    v8::Global<v8::Symbol>         d_wrapKey;

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
    Object wrap(const Context& context, std::unique_ptr<Wrapper>&& wrapper);
    Wrapper *unwrap(const Object& object) const;

    v8::Isolate *isolate() const;
};

}

