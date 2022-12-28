/**
* This is example code from cppcon, done by Jonathan Gopel
* link: https://www.youtube.com/watch?v=gTNJXVmuRRA
* 
* Overview
* - Concepts bind interface
* - Deduced class templates provide compile-time configurability
*   of contained objects
* - Runtime configurability can be achieved with <std::variant> if
*   absolutely needed
*
* Downside
* - Increase translation unit size
* - Potential increase to binary size
* - May increase compile time
* - May add complexity
*/

#include <variant>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Binding interface
namespace case_virtual
{
    struct FooInterface {
        FooInterface() = default;
        FooInterface(const FooInterface&) = default;
        FooInterface(FooInterface&&) = default;
        FooInterface& operator=(const FooInterface&) = default;
        FooInterface& operator=(FooInterface&&) = default;
        virtual ~FooInterface() = default;

        [[nodiscard]] virtual auto func() const -> int = 0;
    };

    struct Foo final : public FooInterface {
        Foo() = default;
        Foo(const Foo&) = default;
        Foo(Foo&&) = default;
        Foo& operator=(const Foo&) = default;
        Foo& operator=(Foo&&) = default;
        virtual ~Foo() = default;

        [[nodiscard]] auto func() const -> int override {
            return 42;
        }
    };
}

namespace case_nvirtual
{
    template <typename T>
    concept CFoo = requires(T foo) {
        //{ foo.func() } -> std::same_as<int>;
        { foo.func() } -> std::integral;
    };

    struct Foo {
        [[nodiscard]] auto func() const -> int {
            return 42;
        }
    };

    static_assert(CFoo<Foo>);
}

// Impl -----------------------------------------------------------------------
namespace case_virtual
{
    int bindInterface(std::unique_ptr<FooInterface> foo) {
        // impl here
        return foo->func();
    }

    void caseStudy1() {
        std::unique_ptr<FooInterface> foo = std::make_unique<Foo>();
        auto retVal = bindInterface(std::move(foo));
    }
}

namespace case_nvirtual
{
    int bindInterface(CFoo auto& foo) {
        return foo.func();
    }

    void caseStudy1() {
        Foo foo{};
        auto retVal = bindInterface(foo);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Own polymorphic type
namespace case_virtual
{
    class Bar {
    public:
        Bar(std::unique_ptr<FooInterface> foo)
            : m_foo(std::move(foo)) { }

        void set_foo(std::unique_ptr<FooInterface> foo) {
            m_foo = std::move(foo);
        }
    private:
        std::unique_ptr<FooInterface> m_foo;
    };
}

namespace case_nvirtual
{
    template <typename T, typename... Ts>
    concept same_as_any = (... or std::same_as<T, Ts>);

    template<CFoo... TFoos>
    class Bar {
    public:
        Bar(same_as_any<TFoos...> auto foo)
            : m_foo(foo) {}

        void set_foo(same_as_any<TFoos...> auto foo) {
            m_foo = foo;
        }

    private:
        std::variant<TFoos...> m_foo;
    };
}

// Impl -----------------------------------------------------------------------
namespace case_virtual
{
    struct Foo1 final : public FooInterface {
            [[nodiscard]] int func() const override { return 40; }
    };
    struct Foo2 final : public FooInterface {
            [[nodiscard]] int func() const override { return 41; }
    };
    void caseStudy2() {
        Bar bar(std::make_unique<Foo>());
    }
}

namespace case_nvirtual
{
    struct Foo1 {
        [[nodiscard]] int func() const { return 40; }
    };
    struct Foo2 {
        [[nodiscard]] int func() const { return 41; }
    };

    void caseStudy2()
    {
        Bar<Foo> bar1{ Foo{} };
        Bar<Foo1, Foo2> bar2{ Foo1{} };
    }
}


///////////////////////////////////////////////////////////////////////////////
// Store multiple 
namespace case_virtual
{
    class Baz {
    public:
        void store(std::unique_ptr<FooInterface> value) {
            m_data.push_back(std::move(value));
        }
    private:
        std::vector<std::unique_ptr<FooInterface>> m_data;
    };
}

namespace case_nvirtual
{
    // This lose the order of store between different types
    template <CFoo... TFoos>
    class Baz {
    public:
        template <same_as_any<TFoos...> T>
        void store(T value) {
            return std::get<std::vector<T>>(m_data).push_back(value);
        }
    private:
        std::tuple<std::vector<TFoos>...> m_data;
    };
}

// Impl -----------------------------------------------------------------------
namespace case_virtual
{
    void caseStudy3() {
        Baz baz;
        baz.store(std::make_unique<Foo1>());
        baz.store(std::make_unique<Foo2>());
    }
}

namespace case_nvirtual
{
    using foo_storage_t = Baz<Foo1, Foo2>;
    void caseStudy3()
    {
        foo_storage_t baz;
        baz.store(Foo1{});
        baz.store(Foo2{});
    }
}
