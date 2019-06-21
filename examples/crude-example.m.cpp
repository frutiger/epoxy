// crude-example.m.cpp

#include <crude_context.h>
#include <crude_convert.h>
#include <crude_runtime.h>
#include <crude_valueutil.h>
#include <crude_wrapper.h>

#include <iostream>
#include <sstream>

static int addF(crude::Value          *result,
                const crude::Runtime&  runtime,
                const crude::Context&  context,
                const crude::Object&,
                const crude::Value&,
                const crude::Values&   arguments)
{
    auto *isolate = runtime.isolate();
    v8::HandleScope handles(isolate);

    int a;
    if (crude::Convert::to(&a, runtime, context, arguments[0])) {
        return -1;
    }

    int b;
    if (crude::Convert::to(&b, runtime, context, arguments[1])) {
        return -1;
    }

    if (crude::Convert::from(result, runtime, context, a + b)) {
        return -1;
    }
    return 0;
}

class Counter : public crude::Wrapper {
    int d_value = 0;

  public:
    static int increment(crude::Value          *result,
                         const crude::Runtime&  runtime,
                         const crude::Context&  context,
                         const crude::Object&   receiver,
                         const crude::Value&,
                         const crude::Values&   arguments)
    {
        auto *isolate = runtime.isolate();
        v8::HandleScope handles(isolate);

        int x;
        if (crude::Convert::to(&x, runtime, context, arguments[0])) {
            return -1;
        }

        Counter *self = nullptr;
        if (crude::Convert::to(&self, runtime, context, receiver)) {
            return -1;
        }

        self->d_value += x;

        *result = crude::Value(isolate, v8::Undefined(isolate));
        return 0;
    }

    static int get(crude::Value          *result,
                   const crude::Runtime&  runtime,
                   const crude::Context&  context,
                   const crude::Object&   receiver,
                   const crude::Value&,
                   const crude::Values&)
    {
        auto *isolate = runtime.isolate();
        v8::HandleScope handles(isolate);

        Counter *self = nullptr;
        if (crude::Convert::to(&self, runtime, context, receiver)) {
            return -1;
        }

        *result = crude::Value(isolate,
                               v8::Number::New(isolate, self->d_value));
        return 0;
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

