// crude_runtime.h
#pragma once

#include <v8.h>

#include <functional>
#include <string>
#include <vector>

namespace crude {

using Value  = v8::Global<v8::Value>;
using Script = v8::Global<v8::UnboundScript>;

class Context;

class Runtime
{
    std::unique_ptr<v8::Platform>  d_platform;
    v8::Isolate                   *d_isolate_p;

  public:
    Runtime();
    ~Runtime();

    Context createContext();
    int compile(Script                  *result,
                const Context&           context,
                const std::string_view&  name,
                const std::string_view&  text);
    int evaluate(Value *result, const Context& context, const Script& script);

    v8::Isolate *isolate() const;
};

}

