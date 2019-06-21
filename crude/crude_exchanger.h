// crude_exchanger.h
#pragma once

namespace v8 { template <class> class Global; }
namespace v8 { class Isolate;                 }
namespace v8 { class Object;                  }

namespace crude {

using Object = v8::Global<v8::Object>;

class Wrapper;

class Exchanger {
  public:
    virtual ~Exchanger();

    virtual v8::Isolate *isolate() const = 0;
    virtual Wrapper *unwrap(const Object& object) const = 0;
};

}

