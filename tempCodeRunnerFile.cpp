#include <iostream>

int main() {

    int *t = new int [10];
    for(int i = 0; i < 10; i++) {
        t[i] = i * 1.01;
    }
    for(int i = 0; i < 10; i++) {
        std::cout << t[i] << std::endl;
    }

    return 0;
}