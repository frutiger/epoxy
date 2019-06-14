// crude_context.h
#pragma once

#include <v8.h>

#include <string>

namespace crude {

using Object = v8::Global<v8::Object>;
using Value  = v8::Global<v8::Value>;

class Context
{
    v8::Isolate             *d_isolate_p;
    v8::Global<v8::Context>  d_context;

  public:
    Context(v8::Isolate *isolate, const v8::Local<v8::Context>& context);

    int set(const std::string_view& property, const Value&  value);
    int set(const std::string_view& property, const Object& value);

    v8::Local<v8::Context> local() const;
};

}

