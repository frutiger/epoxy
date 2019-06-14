// crude_context.cpp
#include <crude_context.h>

namespace crude {

Context::Context(v8::Isolate *isolate, const v8::Local<v8::Context>& context)
: d_isolate_p(isolate)
, d_context(isolate, context)
{
}

int Context::set(const std::string_view& property, const Value& value)
{
    v8::HandleScope    handles(d_isolate_p);
    v8::Context::Scope scope(d_context.Get(d_isolate_p));

    auto name = v8::String::NewFromUtf8(d_isolate_p,
                                        property.data(),
                                        v8::NewStringType::kNormal,
                                        property.size());
    if (name.IsEmpty()) {
        return -1;
    }

    auto context = d_context.Get(d_isolate_p);
    context->Global()->Set(context,
                           name.ToLocalChecked(),
                           value.Get(d_isolate_p)).Check();
    return 0;
}

int Context::set(const std::string_view& property, const Object& object)
{
    v8::HandleScope    handles(d_isolate_p);
    v8::Context::Scope scope(d_context.Get(d_isolate_p));

    return set(property, Value(d_isolate_p, object.Get(d_isolate_p)));
}

v8::Local<v8::Context> Context::local() const
{
    return d_context.Get(d_isolate_p);
}

}

