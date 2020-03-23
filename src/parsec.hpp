#pragma once

#include <any>
#include <tuple>
#include <string>
#include <memory>
#include <variant>
#include <optional>
#include <exception>
#include <functional>
#include <type_traits>

namespace parsec {
#if __cplusplus > 201703L

template<typename T> struct lambda_traits_base;
template<typename ClassType, typename R, typename ...Args>
struct lambda_traits_base<R(ClassType::*)(Args...) const> {
    using result_type = R;
    using args_type = std::tuple<Args...>;
    template<size_t index>
    using arg_type_at = std::tuple_element_t<index, args_type>;
    static constexpr size_t arity = sizeof...(Args);
};

template<typename T, typename = std::void_t<>>
struct call_which {
    using type = decltype(&T::operator());
};
template<typename T>
struct call_which<T, std::void_t<decltype(&T::template operator()<>)>> {
    using type = decltype(&T::template operator()<>);
};
template<typename T>
using call_which_t = typename call_which<T>::type;

template<typename T, typename ...Args>
struct lambda_traits : lambda_traits_base<decltype(&T::template operator()<Args...>)> {};
template<typename T>
struct lambda_traits<T> : lambda_traits_base<call_which_t<T>> {};

#else
template<typename T> struct lambda_traits_base;
template<typename ClassType, typename R, typename ...Args>
struct lambda_traits_base<R(ClassType::*)(Args...) const> {
    using result_type = R;
    using args_type = std::tuple<Args...>;
    template<size_t index>
    using arg_type_at = std::tuple_element_t<index, args_type>;
    static constexpr size_t arity = sizeof...(Args);
};

template<typename T>
struct lambda_traits : lambda_traits_base<decltype(&T::operator())> {};

#endif

template<typename T> struct is_tuple {
    static constexpr bool value = false;
};
template<typename ...Ts> struct is_tuple<std::tuple<Ts...>> {
    static constexpr bool value = true;
};
template<typename T> constexpr bool is_tuple_v = is_tuple<T>::value;

template<typename T1, typename T2> struct concat;
template<typename T1, typename T2> struct addition;
template<typename T1, typename T2> struct product;

template<typename T1, typename T2> using concat_t = typename concat<T1, T2>::type;
template<typename T1, typename T2> using addition_t = typename addition<T1, T2>::type;
template<typename T1, typename T2> using product_t = typename product<T1, T2>::type;

template<typename T1, typename ...T2s> struct concat<T1, std::variant<T2s...>> {
    using type = std::variant<T1, T2s...>;
};
template<typename ...T1s, typename ...T2s> struct concat<std::variant<T1s...>, std::variant<T2s...>> {
    using type = std::variant<T1s..., T2s...>;
};

template<typename T1, typename T2> struct addition {
    using type = std::conditional_t<std::is_same_v<T1, T2>, T1, std::variant<T1, T2>>;
};
template<typename T1, typename T2> struct addition<std::variant<T1>, T2> {
    using type = std::conditional_t<std::is_same_v<T1, T2>, std::variant<T1>, std::variant<T1 ,T2>>;
};
template<typename T1, typename T2> struct addition<T1, std::variant<T2>> {
    using type = std::conditional_t<std::is_same_v<T1, T2>, std::variant<T1>, std::variant<T1 ,T2>>;
};
template<typename T1, typename ...T1s, typename T2> struct addition<std::variant<T1, T1s...>, T2> {
    using type = std::conditional_t<std::is_same_v<T1, T2>, std::variant<T1, T1s...>, concat_t<T1, addition_t<std::variant<T1s...>, T2>>>;
};
template<typename T1, typename T2, typename ...T2s> struct addition<T1, std::variant<T2, T2s...>> {
    using type = std::conditional_t<std::is_same_v<T1, T2>, std::variant<T2, T2s...>, concat_t<T2, addition_t<std::variant<T2s...>, T1>>>;
};
template<typename T1, typename T2> struct addition<std::variant<T1>, std::variant<T2>> {
    using type = std::conditional_t<std::is_same_v<T1, T2>, std::variant<T1>, std::variant<T1, T2>>;
};
template<typename T1, typename T2, typename ...T2s> struct addition<std::variant<T1>, std::variant<T2, T2s...>> {
    using type = std::conditional_t<std::is_same_v<T1, T2>, std::variant<T2, T2s...>, concat_t<T2, addition_t<T1, std::variant<T2s...>>>>;
};
template<typename T1, typename ...T1s, typename ...T2s> struct addition<std::variant<T1, T1s...>, std::variant<T2s...>> {
    using type = concat_t<addition_t<T1, std::variant<T2s...>>, addition_t<std::variant<T1s...>, std::variant<T2s...>>>;
};

template<typename T1, typename T2> struct product {
    using type = std::tuple<T1, T2>;
};
template<typename ...T1s, typename T2> struct product<std::tuple<T1s...>, T2> {
    using type = std::tuple<T1s..., T2>;
};
template<typename T1, typename ...T2s> struct product<T1, std::tuple<T2s...>> {
    using type = std::tuple<T1, T2s...>;
};
template<typename ...T1s, typename ...T2s> struct product<std::tuple<T1s...>, std::tuple<T2s...>> {
    using type = std::tuple<T1s..., T2s...>;
};
template<typename T1, typename T2> struct product<std::variant<T1>, T2> {
    using type = std::variant<std::tuple<T1, T2>>;
};
template<typename T1, typename T2> struct product<T1, std::variant<T2>> {
    using type = std::variant<std::tuple<T1, T2>>;
};
template<typename T1, typename ...T1s, typename T2> struct product<std::variant<T1, T1s...>, T2> {
    using type = concat_t<product_t<T1, T2>, product_t<std::variant<T1s...>, T2>>;
};
template<typename T1, typename T2, typename ...T2s> struct product<T1, std::variant<T2, T2s...>> {
    using type = concat_t<product_t<T1, T2>, product_t<T1, std::variant<T2s...>>>;
};
template<typename T1, typename ...T2s> struct product<std::variant<T1>, std::variant<T2s...>> {
    using type = product_t<T1, std::variant<T2s...>>;
};
template<typename T1, typename ...T1s, typename ...T2s> struct product<std::variant<T1, T1s...>, std::variant<T2s...>> {
    using type = addition_t<product_t<T1, std::variant<T2s...>>, product_t<std::variant<T1s...>, std::variant<T2s...>>>;
};

struct Placeholder {
    Placeholder() = default;
    template<typename T>
    Placeholder(const T &&) {}
};

class ParsecComponentBase {
  public:
    virtual ~ParsecComponentBase() = default;
};

template<typename Result>
class ParsecComponent;

template<typename Result>
class Parsec;

template<typename Result, typename Func1, typename Func2>
inline std::optional<Result> callback_template(Func1 exec, Func2 callback, const std::string &str, std::size_t &index) {
    auto anchor = index;
    auto result = exec(str, index);
    if (result) {
        auto v = result.value();
        if constexpr (is_tuple_v<decltype(v)>) {
            return std::apply(callback, std::move(v));
        } else {
            return callback(std::move(v));
        }
    } else {
        index = anchor;
        return {};
    }
}

template<typename Result, typename Func1, typename Func2>
inline std::optional<Result> alternate_template(Func1 lexec, Func2 rexec, const std::string &str, std::size_t &index) {
    auto anchor = index;
    auto lr = lexec(str, index);
    if (lr) {
        return lr;
    } else {
        index = anchor;
        auto rr = rexec(str, index);
        if (rr) {
            return rr;
        } else {
            index = anchor;
            return {};
        }
    }
}

template<typename Result, typename Func1, typename Func2>
inline std::optional<Result> connect_template(Func1 lexec, Func2 rexec, const std::string &str, std::size_t &index) {
    auto anchor = index;
    auto lr = lexec(str, index);
    if (lr) {
        anchor = index;
        auto rr = rexec(str, index);
        if (rr) {
            auto lv = lr.value();
            auto rv = rr.value();
            if constexpr (is_tuple_v<decltype(lv)> and is_tuple_v<decltype(rv)>) {
                return std::tuple_cat(lv, rv);
            } else if constexpr (is_tuple_v<decltype(lv)>) {
                return std::tuple_cat(lv, std::make_tuple(rv));
            } else if constexpr (is_tuple_v<decltype(rv)>) {
                return std::tuple_cat(std::make_tuple(lv), rv);
            } else {
                return std::make_tuple(lv, rv);
            };
        } else {
            index = anchor;
            return {};
        }
    } else {
        index = anchor;
        return {};
    }
}

// Result must be a single type, Tuple or Variant
template<typename Result>
class ParsecComponent : public ParsecComponentBase {
  public:
    using ResultType = std::optional<Result>;

    ParsecComponent() = default;
    ParsecComponent(ParsecComponent &&component)
      : exec_(std::move(component.exec_)) {}
    ParsecComponent(std::function<ResultType(const std::string &, std::size_t &)> exec)
      : exec_(std::move(exec)) {}
    virtual ~ParsecComponent() = default;

    ResultType operator()(const std::string &str, std::size_t &index) const {
        return exec_(str, index);
    }

    template<typename Func,
        typename NewResult = typename lambda_traits<Func>::result_type>
    ParsecComponent<NewResult> operator>>(Func &&callback) const {
        return ParsecComponent<NewResult>([exec = this->exec(), cb = std::move(callback)](const std::string &str, std::size_t &index) {
            return callback_template<NewResult>(exec, cb, str, index);
        });
    }

    template<typename RhsResult,
        typename NewResult = addition_t<Result, RhsResult>>
    ParsecComponent<NewResult> operator|(const ParsecComponent<RhsResult> &rhs) const {
        return ParsecComponent<NewResult>([lexec = this->exec(), rexec = rhs.exec()](const std::string &str, std::size_t &index) {
            return alternate_template<NewResult>(lexec, rexec, str, index);
        });
    }

    template<typename RhsResult,
        typename NewResult = product_t<Result, RhsResult>>
    ParsecComponent<NewResult> operator+(const ParsecComponent<RhsResult> &rhs) const {
        return ParsecComponent<NewResult>([lexec = this->exec(), rexec = rhs.exec()](const std::string &str, std::size_t &index) {
            return connect_template(lexec, rexec, str, index);
        });
    }

    template<typename RhsResult>
    ParsecComponent<addition_t<Result, RhsResult>> operator|(const Parsec<RhsResult> &rhs) const;

    template<typename RhsResult>
    ParsecComponent<product_t<Result, RhsResult>> operator+(const Parsec<RhsResult> &rhs) const;

    std::function<ResultType(const std::string &, std::size_t &)> exec() const {
        return exec_;
    }

    friend ParsecComponent<char> operator""_T(char ch);

  private:
    std::function<ResultType(const std::string &, std::size_t &)> exec_;
};

template<typename Result>
class Parsec {
  public:
    Parsec() : component_(nullptr) {};
    Parsec(ParsecComponent<Result> &&component)
      : component_(std::make_shared<ParsecComponent<Result>(std::move(component))>) {}

    std::shared_ptr<ParsecComponent<Result>> component() const {
        return std::static_pointer_cast<ParsecComponent<Result>>(component_);
    }

    Parsec &operator=(ParsecComponent<Result> &&component) {
        component_ = std::make_shared<ParsecComponent<Result>>(std::move(component));
        return *this;
    }

    Result operator()(const std::string &str) const {
        std::size_t index = 0;
        return operator()(str, index);
    }

    Result operator()(const std::string &str, std::size_t &index) const {
        if (component_) {
            auto r = component()->operator()(str, index);
            if (r) {
                return r.value();
            } else {
                throw std::runtime_error("parse error at " + std::to_string(index));
            }
        } else {
            throw std::runtime_error("not give definition");
        }
    }

    template<typename Func,
        typename NewResult = typename lambda_traits<Func>::result_type>
    ParsecComponent<NewResult> operator>>(Func &&callback) {
        return ParsecComponent<NewResult>([this, cb = std::move(callback)](const std::string &str, std::size_t &index) {
            return callback_template<NewResult>(this->component()->exec(), cb, str, index);
        });
    }

    template<typename RhsResult,
        typename NewResult = addition_t<Result, RhsResult>>
    ParsecComponent<NewResult> operator|(const Parsec<RhsResult> &rhs) {
        return ParsecComponent<NewResult>([this, &rhs](const std::string &str, std::size_t &index) {
            return alternate_template<NewResult>(this->component()->exec(), rhs.component()->exec(), str, index);
        });
    }

    template<typename RhsResult,
        typename NewResult = product_t<Result, RhsResult>>
    ParsecComponent<NewResult> operator+(const Parsec<RhsResult> &rhs) {
        return ParsecComponent<NewResult>([this, &rhs](const std::string &str, std::size_t &index) {
            return connect_template<NewResult>(this->component()->exec(), rhs.component()->exec(), str, index);
        });
    }

    template<typename RhsResult,
        typename NewResult = addition_t<Result, RhsResult>>
    ParsecComponent<NewResult> operator|(const ParsecComponent<RhsResult> &rhs) {
        return ParsecComponent<NewResult>([this, rexec = rhs.exec()](const std::string &str, std::size_t &index) {
            return alternate_template<NewResult>(this->component()->exec(), rexec, str, index);
        });
    }

    template<typename RhsResult,
        typename NewResult = product_t<Result, RhsResult>>
    ParsecComponent<NewResult> operator+(const ParsecComponent<RhsResult> &rhs) {
        return ParsecComponent<NewResult>([this, rexec = rhs.exec()](const std::string &str, std::size_t &index) {
            return connect_template<NewResult>(this->component()->exec(), rexec, str, index);
        });
    }

  private:
    std::shared_ptr<ParsecComponentBase> component_;
};

template<typename Result>
template<typename RhsResult>
inline ParsecComponent<addition_t<Result, RhsResult>>
ParsecComponent<Result>::operator|(const Parsec<RhsResult> &rhs) const {
    using NewResult = addition_t<Result, RhsResult>;
    return ParsecComponent<NewResult>([lexec = this->exec(), &rhs](const std::string &str, std::size_t &index) {
        return alternate_template<NewResult>(lexec, rhs.component()->exec(), str, index);
    });
}

template<typename Result>
template<typename RhsResult>
inline ParsecComponent<product_t<Result, RhsResult>>
ParsecComponent<Result>::operator+(const Parsec<RhsResult> &rhs) const {
    using NewResult = product_t<Result, RhsResult>;
    return ParsecComponent<NewResult>([lexec = this->exec(), &rhs](const std::string &str, std::size_t &index) {
        return connect_template<NewResult>(lexec, rhs.component()->exec(), str, index);
    });
}

struct Token {
    static ParsecComponent<char> by(const std::function<bool(char)> &cond) {
        return ParsecComponent<char>([cond](const std::string &str, std::size_t &index) -> std::optional<char> {
            if (cond(str[index])) {
                return str[index++];
            } else {
                return {};
            }
        });
    }

    template<typename R>
    static ParsecComponent<R> epsilon() {
        return ParsecComponent<R>([](const std::string &str, std::size_t &index) -> std::optional<R> {
            return R();
        });
    }

    template<typename Func,
        typename R = typename lambda_traits<Func>::result_type>
    static ParsecComponent<R> epsilon(Func &&func) {
        if constexpr (lambda_traits<Func>::arity == 0) {
            return ParsecComponent<R>([f = std::move(func)](const std::string &str, std::size_t &index) -> std::optional<R> {
                return f();
            });
        } else if constexpr (lambda_traits<Func>::arity == 2 &&
            std::is_same_v<lambda_traits<Func>::template arg_type_at<0>, const std::string &> &&
            std::is_same_v<lambda_traits<Func>::template arg_type_at<1>, std::size_t &>) {
            return ParsecComponent<R>([f = std::move(func)](const std::string &str, std::size_t &index) -> std::optional<R> {
                return f(str, index);
            });
        } else {
            return;
        }
    }
};

inline ParsecComponent<char> operator""_T(char ch) {
    return ParsecComponent<char>([ch](const std::string &str, std::size_t &index) -> std::optional<char> {
        if (str[index] == ch) {
            return str[index++];
        } else {
            return {};
        }
    });
}

inline ParsecComponent<std::string> operator""_T(const char *str, std::size_t len) {
    return ParsecComponent<std::string>([pattern = std::string(str)](const std::string &str, std::size_t &index) -> std::optional<std::string> {
        for (auto c : pattern) {
            if (str[index] == c) {
                ++index;
            } else {
                return {};
            }
        }
        return pattern;
    });
}
}
