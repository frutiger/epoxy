// crude-example.m.cpp

#include <crude_context.h>
#include <crude_runtime.h>
#include <crude_valueutil.h>

#include <iostream>
#include <sstream>

static crude::Value addF(const crude::Runtime& runtime,
                         const crude::Object&,
                         const crude::Value&,
                         const crude::Values&  arguments)
{
    auto *isolate = runtime.isolate();
    v8::HandleScope handles(isolate);

    auto context = isolate->GetCurrentContext();

    auto a = arguments[0].Get(isolate)->ToNumber(context).ToLocalChecked();
    auto b = arguments[1].Get(isolate)->ToNumber(context).ToLocalChecked();
    return crude::Value(isolate, v8::Number::New(isolate,
                                                 a->Value() + b->Value()));
}

int main()
{
    crude::Runtime runtime;

    auto context = runtime.createContext();

    crude::Object add;
    runtime.host(&add, context, &addF);
    context.set("add", add);

    crude::Script script;
    if (runtime.compile(&script,
                        context,
                        "<example>",
                        "add(5, 5);")) {
        return 1;
    }

    crude::Value result;
    if (runtime.evaluate(&result, context, script)) {
        return 1;
    }

    std::ostringstream output;
    crude::ValueUtil::print(output, runtime, context, result) << '\n';

    assert(output.str() == "10\n");

    return 0;
}

