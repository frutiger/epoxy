// crude_valueutil.cpp
#include <crude_valueutil.h>

#include <crude_context.h>
#include <crude_runtime.h>

#include <v8.h>

#include <iostream>

namespace crude {

namespace {

static std::ostream& indent(std::ostream& stream, unsigned int indent)
{
    while (indent > 0) {
        stream << ' ';
        --indent;
    }
    return stream;
}

}

std::ostream& ValueUtil::print(std::ostream&                  stream,
                               v8::Isolate                   *isolate,
                               const v8::Local<v8::Context>&  context,
                               const v8::Local<v8::Value>&    value,
                               unsigned int                   indentation)
{
    if (value->IsUndefined()) {
        return stream << "undefined";
    }

    if (value->IsNull()) {
        return stream << "null";
    }

    if (value->IsInt32()) {
        return stream << value->ToInt32(context).ToLocalChecked()->Value();
    }

    if (value->IsNumber()) {
        return stream << value->ToNumber(context).ToLocalChecked()->Value();
    }

    if (value->IsObject()) {
        auto object = value->ToObject(context).ToLocalChecked();

        if (object->IsCallable()) {
            return stream << "[Function]";
        }

        if (object->IsArray()) {
            stream << "[\n";

            auto *array  = v8::Array::Cast(*object);
            auto  length = array->Length();

            for (unsigned int i = 0; i < length; ++i) {
                auto element = array->Get(context, i).ToLocalChecked();
                indent(stream, indentation + 2);
                print(stream, isolate, context, element, indentation + 2);
                if (i < length - 1) {
                    stream << ',';
                }
                stream << '\n';
            }

            return indent(stream, indentation) << ']';
        }

        auto maybeProperties = object->GetPropertyNames(context);
        if (maybeProperties.IsEmpty()) {
            return stream;
        }

        stream << "{\n";

        auto properties    = maybeProperties.ToLocalChecked();
        auto numProperties = properties->Length();
        for (unsigned int i = 0; i < numProperties; ++i) {
            auto propertyValue  = properties->Get(context, i).ToLocalChecked();
            auto propertyString = propertyValue->ToString(context)
                                  .ToLocalChecked();
            auto propertySize   = propertyString->Length();

            std::string property;
            property.resize(propertySize);
            propertyString->WriteUtf8(isolate, property.data());
            indent(stream, indentation + 2) << property << ": ";

            auto element = object->Get(context,
                                       propertyString).ToLocalChecked();
            print(stream, isolate, context, element, indentation + 2);

            if (i < numProperties - 1) {
                stream << ',';
            }
            stream << '\n';
        }

        return indent(stream, indentation) << '}';
    }

    // TBD: other types
    std::abort();
}

std::ostream& ValueUtil::print(std::ostream&  stream,
                               const Runtime& runtime,
                               const Context& context,
                               const Value&   value,
                               unsigned int   indentation)
{
    auto *isolate = runtime.isolate();
    v8::HandleScope handles(isolate);
    return print(stream,
                 isolate,
                 context.local(),
                 value.Get(isolate),
                 indentation);
}

}

