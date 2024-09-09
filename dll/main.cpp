#include <lib1.hpp>

int wmain(int argc, wchar_t *argv[]) {
    Class1 c;
    c.set(5);
    std::wcout << c.get() << L"\n";
    return 0;
}