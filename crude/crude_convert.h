// crude_convert.h
#pragma once

#include <crude_exchanger.h>
#include <crude_wrapper.h>

#include <string>
#include <type_traits>

namespace v8 { template <class> class Global; }
namespace v8 { class Object;                  }
namespace v8 { class Value;                   }

namespace crude {

using Object = v8::Global<v8::Object>;
using Value  = v8::Global<v8::Value>;

class Context;

struct Convert
{
    template <class T>
    static std::enable_if_t<std::is_convertible_v<T *, Wrapper *>, int>
    to(T                **destination,
       const Exchanger&   exchanger,
       const Context&     context,
       const Object&      source);

    static int to(int               *destination,
                  const Exchanger&   exchanger,
                  const Context&     context,
                  const Value&       source);
    static int to(std::string       *destination,
                  const Exchanger&   exchanger,
                  const Context&     context,
                  const Value&       source);

    static int from(Value                   *destination,
                    const Exchanger&         exchanger,
                    const Context&           context,
                    int                      source);
    static int from(Value                   *destination,
                    const Exchanger&         exchanger,
                    const Context&           context,
                    const std::string_view&  source);
};

template <class T>
std::enable_if_t<std::is_convertible_v<T *, Wrapper *>, int>
Convert::to(T                **destination,
            const Exchanger&   exchanger,
            const Context&,
            const Object&      source)
{
    *destination = dynamic_cast<T *>(exchanger.unwrap(source));
    if (!*destination) {
        return -1;
    }
    return 0;
}

}

