#ifndef LIB1_GUARD
#define LIB1_GUARD

#ifdef __WIN32__
#ifdef BUILD_LIB
#define LIB_CLASS __declspec(dllexport)
#else
#define LIB_CLASS __declspec(dllimport)
#endif
#else
#define LIB_CLASS
#endif

#include <iostream>

class LIB_CLASS Class1 {
    public:
        Class1();
        ~Class1();

        int get();
        bool set(int x);

    private:
        int ix;
};

#endif