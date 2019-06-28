// epoxy_adapter.h
#pragma once

#include <functional>
#include <sstream>
#include <tuple>

namespace epoxy {

struct Error : public std::ostringstream
{
};

template <class RESULT, class... ARGS>
struct Adapter_FunctionTraits;

template <class RESULT, class... ARGS>
struct Adapter_FunctionTraits<RESULT (*)(ARGS...)>
{
    using function        = RESULT (*)(ARGS...);
    using receiver_type   = void;
    using result_type     = RESULT;
    using arguments_tuple = std::tuple<ARGS...>;
    using takes_error     = std::false_type;
};

template <class RESULT, class... ARGS>
struct Adapter_FunctionTraits<RESULT (*)(Error&, ARGS...)>
{
    using function        = RESULT (*)(Error&, ARGS...);
    using receiver_type   = void;
    using result_type     = RESULT;
    using arguments_tuple = std::tuple<ARGS...>;
    using takes_error     = std::true_type;
};

template <class RESULT, class RECEIVER, class... ARGS>
struct Adapter_FunctionTraits<RESULT (RECEIVER::*)(ARGS...)>
{
    using function        = RESULT (RECEIVER::*)(ARGS...);
    using receiver_type   = RECEIVER;
    using result_type     = RESULT;
    using arguments_tuple = std::tuple<ARGS...>;
    using takes_error     = std::false_type;
};

template <class RESULT, class RECEIVER, class... ARGS>
struct Adapter_FunctionTraits<RESULT (RECEIVER::*)(Error&, ARGS...)>
{
    using function        = RESULT (RECEIVER::*)(Error&, ARGS...);
    using receiver_type   = RECEIVER;
    using result_type     = RESULT;
    using arguments_tuple = std::tuple<ARGS...>;
    using takes_error     = std::false_type;
};

template <class CONVERTER,
          class EXCHANGER,
          class CONTEXT,
          class VALUE,
          class ARGUMENTS>
struct Adapter_ArgsConverter
{
    template <int INDEX>
    static int convertHelper(std::ostream&              errorStream,
                             ARGUMENTS                 *arguments,
                             const EXCHANGER&           exchanger,
                             const CONTEXT&             context,
                             const std::vector<VALUE>&  values)
    {
        if (CONVERTER::to(errorStream,
                          &std::get<INDEX>(*arguments),
                          exchanger,
                          context,
                          values[INDEX])) {
            return -1;
        }

        return convertHelper<INDEX - 1>(errorStream,
                                        arguments,
                                        exchanger,
                                        context,
                                        values);
    }

    template <>
    static int convertHelper<-1>(std::ostream&,
                                 ARGUMENTS *,
                                 const EXCHANGER&,
                                 const CONTEXT&,
                                 const std::vector<VALUE>&)
    {
        return 0;
    }

    static int convert(std::ostream&              errorStream,
                       ARGUMENTS                 *arguments,
                       const EXCHANGER&           exchanger,
                       const CONTEXT&             context,
                       const std::vector<VALUE>&  values)
    {
        static const int argumentsSize = std::tuple_size<ARGUMENTS>();
        if (values.size() < argumentsSize) {
            errorStream << "Invocation Error: expected " << argumentsSize
                        << " argument(s) but received " << values.size()
                        << " argument(s)";
            return -1;
        }
        return convertHelper<argumentsSize - 1>(errorStream,
                                                arguments,
                                                exchanger,
                                                context,
                                                values);
    }
};

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RECEIVER,
          class RESULT,
          class TAKES_ERROR>
struct Adapter_Helper;

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RECEIVER,
          class RESULT>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      RECEIVER,
                      RESULT,
                      std::true_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto          *result,
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&         receiver,
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;
            using Result        = typename TRAITS::result_type;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            Error localErrorStream;
            std::tuple<Error&> errorArg { localErrorStream };

            std::tuple<RECEIVER *> selfArg { nullptr };
            if (CONVERTER::to(errorStream,
                              &std::get<0>(selfArg),
                              exchanger,
                              context,
                              receiver)) {
                return -1;
            }

            Args args;
            if (AC::convert(errorStream, &args, exchanger, context, values)) {
                return -1;
            }

            Result nativeResult = std::apply(source,
                                             std::tuple_cat(errorArg,
                                                            selfArg,
                                                            args));

            std::string error = localErrorStream.str();
            if (!error.empty()) {
                errorStream << error;
                return -1;
            }

            if (CONVERTER::from(errorStream,
                                result,
                                exchanger,
                                context,
                                nativeResult)) {
                return -1;
            }

            return 0;
        };
    }
};

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RECEIVER,
          class RESULT>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      RECEIVER,
                      RESULT,
                      std::false_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto          *result,
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&         receiver,
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;
            using Result        = typename TRAITS::result_type;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            std::tuple<RECEIVER *> selfArg { nullptr };
            if (CONVERTER::to(errorStream,
                              &std::get<0>(selfArg),
                              exchanger,
                              context,
                              receiver)) {
                return -1;
            }

            Args args;
            if (AC::convert(errorStream, &args, exchanger, context, values)) {
                return -1;
            }

            Result nativeResult = std::apply(source,
                                             std::tuple_cat(selfArg, args));
            if (CONVERTER::from(errorStream,
                                result,
                                exchanger,
                                context,
                                nativeResult)) {
                return -1;
            }

            return 0;
        };
    }
};

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RECEIVER>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      RECEIVER,
                      void,
                      std::true_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto *,        // result
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&         receiver,
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            Error localErrorStream;
            std::tuple<Error&> errorArg { localErrorStream };

            std::tuple<RECEIVER *> selfArg { nullptr };
            if (CONVERTER::to(errorStream,
                              &std::get<0>(selfArg),
                              exchanger,
                              context,
                              receiver)) {
                return -1;
            }

            Args args;
            if (AC::convert(errorStream,
                            &args,
                            exchanger,
                            context,
                            values)) {
                return -1;
            }

            std::apply(source, std::tuple_cat(errorArg, selfArg, args));

            std::string error = localErrorStream.str();
            if (!error.empty()) {
                errorStream << error;
                return -1;
            }

            return 0;
        };
    }
};

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RECEIVER>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      RECEIVER,
                      void,
                      std::false_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto *,        // result
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&         receiver,
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            std::tuple<RECEIVER *> selfArg { nullptr };
            if (CONVERTER::to(errorStream,
                              &std::get<0>(selfArg),
                              exchanger,
                              context,
                              receiver)) {
                return -1;
            }

            Args args;
            if (AC::convert(errorStream,
                            &args,
                            exchanger,
                            context,
                            values)) {
                return -1;
            }

            std::apply(source, std::tuple_cat(selfArg, args));
            return 0;
        };
    }
};

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RESULT>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      void,
                      RESULT,
                      std::true_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto          *result,
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&,        // receiver
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;
            using Result        = typename TRAITS::result_type;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            Error localErrorStream;
            std::tuple<Error&> errorArg { localErrorStream };

            Args args;
            if (AC::convert(errorStream, &args, exchanger, context, values)) {
                return -1;
            }

            Result nativeResult = std::apply(source,
                                             std::tuple_cat(errorArg, args));

            std::string error = localErrorStream.str();
            if (!error.empty()) {
                errorStream << error;
                return -1;
            }

            if (CONVERTER::from(errorStream,
                                result,
                                exchanger,
                                context,
                                nativeResult)) {
                return -1;
            }

            return 0;
        };
    }
};

template <class TRAITS,
          class DESTINATION,
          class CONVERTER,
          class RESULT>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      void,
                      RESULT,
                      std::false_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto          *result,
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&,        // receiver
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;
            using Result        = typename TRAITS::result_type;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            Args args;
            if (AC::convert(errorStream, &args, exchanger, context, values)) {
                return -1;
            }

            Result nativeResult = std::apply(source, args);
            if (CONVERTER::from(errorStream,
                                result,
                                exchanger,
                                context,
                                nativeResult)) {
                return -1;
            }

            return 0;
        };
    }
};

template <class TRAITS, class DESTINATION, class CONVERTER>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      void,
                      void,
                      std::true_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto *,        // result
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&,        // receiver
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            Error localErrorStream;
            std::tuple<Error&> errorArg { localErrorStream };

            Args args;
            if (AC::convert(errorStream, &args, exchanger, context, values)) {
                return -1;
            }

            std::apply(source, std::tuple_cat(errorArg, args));

            std::string error = localErrorStream.str();
            if (!error.empty()) {
                errorStream << error;
                return -1;
            }

            return 0;
        };
    }
};

template <class TRAITS, class DESTINATION, class CONVERTER>
struct Adapter_Helper<TRAITS,
                      DESTINATION,
                      CONVERTER,
                      void,
                      void,
                      std::false_type>
{
    static DESTINATION adapt(const typename TRAITS::function& source)
    {
        return [source] (std::ostream&  errorStream,
                         auto *,        // result
                         auto&&         exchanger,
                         auto&&         context,
                         auto&&,        // receiver
                         auto&&,        // target
                         auto&&         values)
        {
            using Args          = typename TRAITS::arguments_tuple;

            using ExchangerCRef = decltype(exchanger);
            using ExchangerC    = std::remove_reference_t<ExchangerCRef>;
            using Exchanger     = std::remove_const_t<ExchangerC>;

            using ContextCRef   = decltype(context);
            using ContextC      = std::remove_reference_t<ContextCRef>;
            using Context       = std::remove_const_t<ContextC>;

            using ValuesCRef    = decltype(values);
            using ValuesC       = std::remove_reference_t<ValuesCRef>;
            using Values        = std::remove_const_t<ValuesC>;
            using Value         = typename Values::value_type;

            using AC            = Adapter_ArgsConverter<CONVERTER,
                                                        Exchanger,
                                                        Context,
                                                        Value,
                                                        Args>;

            Args args;
            if (AC::convert(errorStream, &args, exchanger, context, values)) {
                return -1;
            }

            std::apply(source, args);
            return 0;
        };
    }
};

template <class DESTINATION, class CONVERTER>
struct Adapter
{
    template <class SOURCE>
    static DESTINATION adapt(const SOURCE& source)
    {
        using Traits = Adapter_FunctionTraits<SOURCE>;
        using Helper = Adapter_Helper<Traits,
                                      DESTINATION,
                                      CONVERTER,
                                      typename Traits::receiver_type,
                                      typename Traits::result_type,
                                      typename Traits::takes_error>;
        return Helper::adapt(source);
    }
};

}

