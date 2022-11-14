#include <iostream>
#include <string>

int main(void) {

std::string instring = "(, ), 1, +, 2, *, 4";

std::size_t pos = 0;
int parentheses = 0;
while(instring.find_first_of("(", pos) != -1){
    parentheses += 1;
}

std::cout << parentheses << std::endl;




return 0;
}