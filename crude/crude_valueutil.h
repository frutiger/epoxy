// crude_valueutil.h
#pragma once

#include <iosfwd>

namespace v8 { template <class T> class Global; }
namespace v8 { template <class T> class Local;  }
namespace v8 { class Context;                   }
namespace v8 { class Isolate;                   }
namespace v8 { class Value;                     }

namespace crude {

using Value = v8::Global<v8::Value>;

class Context;
class Runtime;

struct ValueUtil {
  public:
    static std::ostream& print(std::ostream&                  stream,
                               v8::Isolate                   *isolate,
                               const v8::Local<v8::Context>&  context,
                               const v8::Local<v8::Value>&    value,
                               unsigned int                   indentation = 0);
    static std::ostream& print(std::ostream&  stream,
                               const Runtime& runtime,
                               const Context& context,
                               const Value&   value,
                               unsigned int   indentation = 0);
};

}

