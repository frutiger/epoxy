// crude-example.m.cpp

#include <crude_context.h>
#include <crude_convert.h>
#include <crude_runtime.h>
#include <crude_valueutil.h>
#include <crude_wrapper.h>
#include <epoxy_adapter.h>

#include <cmath>
#include <iostream>
#include <sstream>

static void printF(std::string message)
{
    std::cout << message;
}

static int addF(int a, int b)
{
    return a + b;
}

static double squareRootF(epoxy::Error& errorStream, double x)
{
    if (x < 0) {
        errorStream << "squareRoot called with negative number";
        return 0.0;
    }
    return std::sqrt(x);
}

class Counter : public crude::Wrapper {
    int d_value = 0;

  public:
    void increment(int x)
    {
        d_value += x;
    }

    int get()
    {
        return d_value;
    }
};

int main()
{
    crude::Runtime runtime;

    auto context = runtime.createContext();

    using Adapter = epoxy::Adapter<crude::Runtime::Signature, crude::Convert>;

    crude::Object print;
    runtime.host(&print, context, Adapter::adapt(&printF));
    context.set("print", print);

    crude::Object add;
    runtime.host(&add, context, Adapter::adapt(&addF));
    context.set("add", add);

    crude::Object squareRoot;
    runtime.host(&squareRoot, context, Adapter::adapt(&squareRootF));
    context.set("squareRoot", squareRoot);

    crude::Object Counter_increment;
    runtime.host(&Counter_increment,
                 context,
                 Adapter::adapt(&Counter::increment));
    context.set("Counter_increment", Counter_increment);

    crude::Object Counter_get;
    runtime.host(&Counter_get, context, Adapter::adapt(&Counter::get));
    context.set("Counter_get", Counter_get);

    auto counter = runtime.wrap(context, std::make_unique<Counter>());
    context.set("counter", counter);

    crude::Script script;
    if (runtime.compile(std::cerr,
                        &script,
                        context,
                        "<example>",
                        "try {\n"
                        "    print(squareRoot(-4) + '\\n');\n"
                        "}\n"
                        "catch (e) {\n"
                        "    print(e.stack + '\\n');\n"
                        "}\n"
                        "Counter_increment.call(counter, add(squareRoot(4), 3));\n"
                        "Counter_increment.call(counter, 20);\n"
                        "Counter_get.call(counter);")) {
        return 1;
    }

    crude::Value result;
    if (runtime.evaluate(std::cerr, &result, context, script)) {
        return 1;
    }

    std::ostringstream output;
    crude::ValueUtil::print(output, runtime, context, result) << '\n';

    assert(output.str() == "25\n");

    return 0;
}

