#include <type_traits>
#include <iostream>

class Obj
{
    public:
    Obj() {}
    virtual ~Obj() {}

// Uncommenting this line causes is_convertible to not work
//    Obj(const Obj&) = delete;
};
class Val : public Obj
{
    public:
    ~Val() override;
};

template <typename T,
          typename std::enable_if_t<(!std::is_convertible_v<T, Val>), bool> = true>
void f()
{
    std::cout << "not convertible" << std::endl;
}

template <typename T,
          typename std::enable_if_t<(std::is_convertible_v<T, Val>), bool> = true>
void f()
{
    std::cout << "convertible" << std::endl;
}

int main(int argc, char** argv)
{
    std::cout << std::is_convertible_v<Val, Obj> << std::endl;
    std::cout << std::is_convertible_v<int, Val> << std::endl;

    f<Val>();
    f<int>();
}
