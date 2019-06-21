// crude_convert.cpp
#include <crude_convert.h>

#include <crude_context.h>

namespace crude {

int Convert::to(int              *destination,
                const Exchanger&  exchanger,
                const Context&    context,
                const Value&      source)
{
    auto *isolate = exchanger.isolate();
    v8::HandleScope handles(isolate);

    v8::MaybeLocal<v8::Integer> maybeNumber;
    {
        v8::Context::Scope scope(context.local());
        maybeNumber = source.Get(isolate)->ToInteger(context.local());
    }
    if (maybeNumber.IsEmpty()) {
        return -1;
    }

    *destination = maybeNumber.ToLocalChecked()->Value();
    return 0;
}

int Convert::to(std::string      *destination,
                const Exchanger&  exchanger,
                const Context&    context,
                const Value&      source)
{
    auto *isolate = exchanger.isolate();
    v8::HandleScope handles(isolate);

    v8::MaybeLocal<v8::String> maybeString;
    {
        v8::Context::Scope scope(context.local());
        maybeString = source.Get(isolate)->ToString(context.local());
    }
    if (maybeString.IsEmpty()) {
        return -1;
    }

    auto string = maybeString.ToLocalChecked();
    destination->resize(string->Utf8Length(isolate));
    string->WriteUtf8(isolate, destination->data());
    return 0;
}

int Convert::from(Value            *destination,
                  const Exchanger&  exchanger,
                  const Context&    context,
                  int               source)
{
    auto *isolate = exchanger.isolate();
    v8::HandleScope handles(isolate);

    {
        v8::Context::Scope scope(context.local());
        *destination = Value(isolate, v8::Int32::New(isolate, source));
    }
    return 0;
}

int Convert::from(Value                   *destination,
                  const Exchanger&         exchanger,
                  const Context&           context,
                  const std::string_view&  source)
{
    auto *isolate = exchanger.isolate();
    v8::HandleScope handles(isolate);

    {
        v8::Context::Scope scope(context.local());
        auto maybeString = v8::String::NewFromUtf8(isolate,
                                                   source.begin(),
                                                   v8::NewStringType::kNormal,
                                                   source.length());
        if (maybeString.IsEmpty()) {
            return -1;
        }

        *destination = Value(isolate, maybeString.ToLocalChecked());
    }
    return 0;
}

}

