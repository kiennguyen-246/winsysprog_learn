#include "lib1.hpp"

Class1::Class1() {
    ix = 0;
}

Class1::~Class1() {
    ix = 0;
}

int Class1::get() {
    return ix;
}

bool Class1::set(int x) {
    ix = x;
    return 1;
}