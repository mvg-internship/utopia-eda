#include <iostream>
#include <string>

void sum_operators(std::string, std::string, int&);


int main(void) {
std::cout << "start programm" << std::endl;

std::string instring = "), (, (, 1, +, 2, *, 4, -";

std::size_t pos = 0;

//parentheses check
int parentheses = 0;
while((pos = instring.find_first_of("(", pos) + 1) != 0){
    parentheses += 1;
}

while((pos = instring.find_first_of(")", pos) + 1) != 0){
    parentheses -= 1;
}

if(parentheses > 0){
    std::cout << "INPUT ERROR: extra opening parentheses" << std::endl;
}

if(parentheses < 0){
    std::cout << "INPUT ERROR: extra closing parentheses" << std::endl;
}
//parentheses check

int operators = 0;
// while((pos = instring.find_first_of("(", pos) + 1) != 0){
//     operators += 1;
// }

std::cout << "operators: " << operators << std::endl;



sum_operators(instring, "+", operators);

std::cout << "operator: " << operators << std::endl;

return 0;
}

void sum_operators(std::string instring, std::string types, int &operators){
    int pos = 0;
    while(pos = instring.find_first_of(types, pos) + 1 != 0){
        operators += 1;
    }   
}



// std::cout << "pos: " << pos << std::endl;
