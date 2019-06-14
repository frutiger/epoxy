// crude-example.m.cpp

#include <crude_context.h>
#include <crude_runtime.h>
#include <crude_valueutil.h>
#include <crude_wrapper.h>

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

class Counter : public crude::Wrapper {
    int d_value = 0;

  public:
    static crude::Value increment(const crude::Runtime& runtime,
                                  const crude::Object&  receiver,
                                  const crude::Value&,
                                  const crude::Values&  arguments)
    {
        auto *isolate = runtime.isolate();
        v8::HandleScope handles(isolate);

        auto context = isolate->GetCurrentContext();

        auto x = arguments[0].Get(isolate)->ToNumber(context).ToLocalChecked();

        auto *self = static_cast<Counter *>(runtime.unwrap(receiver));
        self->d_value += x->Value();

        return crude::Value(isolate, v8::Undefined(isolate));
    }

    static crude::Value get(const crude::Runtime& runtime,
                            const crude::Object&  receiver,
                            const crude::Value&,
                            const crude::Values&)
    {
        auto *isolate = runtime.isolate();
        v8::HandleScope handles(isolate);

        auto *self = static_cast<Counter *>(runtime.unwrap(receiver));
        return crude::Value(isolate, v8::Number::New(isolate, self->d_value));
    }

    void hosted(const crude::Object&) override
    {
    }
};

int main()
{
    crude::Runtime runtime;

    auto context = runtime.createContext();

    crude::Object add;
    runtime.host(&add, context, &addF);
    context.set("add", add);

    crude::Object Counter_increment;
    runtime.host(&Counter_increment, context, &Counter::increment);
    context.set("Counter_increment", Counter_increment);

    crude::Object Counter_get;
    runtime.host(&Counter_get, context, &Counter::get);
    context.set("Counter_get", Counter_get);

    auto counter = runtime.wrap(context, std::make_unique<Counter>());
    context.set("counter", counter);

    crude::Script script;
    if (runtime.compile(&script,
                        context,
                        "<example>",
                        "Counter_increment.call(counter, add(2, 3)); \
                         Counter_increment.call(counter, 20); \
                         Counter_get.call(counter);")) {
        return 1;
    }

    crude::Value result;
    if (runtime.evaluate(&result, context, script)) {
        return 1;
    }

    std::ostringstream output;
    crude::ValueUtil::print(output, runtime, context, result) << '\n';

    assert(output.str() == "25\n");

    return 0;
}

