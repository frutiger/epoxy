// crude_wrapper.h
#pragma once

namespace v8 { template <class> class Global; }
namespace v8 { class Object;                  }

namespace crude {

using Object = v8::Global<v8::Object>;

class Wrapper {
  public:
    virtual ~Wrapper();

    virtual void hosted(const Object& object);
};

}

