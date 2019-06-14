// crude-example.m.cpp

#include <crude_context.h>
#include <crude_runtime.h>
#include <crude_valueutil.h>

#include <iostream>
#include <sstream>

int main()
{
    crude::Runtime runtime;

    auto context = runtime.createContext();
    crude::Script script;
    if (runtime.compile(&script,
                        context,
                        "<example>",
                        "5 + 5;")) {
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

