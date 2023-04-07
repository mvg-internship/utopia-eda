#include <iostream>
#include <vector>

struct myStruct {
    char* first;
    int second;
};

std::vector<myStruct> vectors;

int main() {

    std::cout << "ENTER" << std::endl;

    char* text1 = "aaa";
    char* text2 = "aaa";
    if(text1 == text2) 
        std::cout << "EQUAL" << std::endl;



    // for(int i = 0; i < 10; i++) {
    //     char* text = "iqw";
    //     std::cout << sizeof(text);
    //     myStruct ms {text, i};
    //     vectors.push_back(ms);
    // }
    // for(auto i : vectors) {
    //     std::cout << i.first << " " << i.second << std::endl;
    // }

    return 0;
}
