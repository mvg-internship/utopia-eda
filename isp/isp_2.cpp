#include <iostream>
#include <string>

void sum_operators(const std::string, std::string, int&);


int main(void) {
std::cout << "start programm" << std::endl;

std::string instring = "), (, (, 1, +, 2, *, 4, -, 4, -";

std::size_t pos = 0;
int parentheses_sum = 0;
int parentheses = 0;
int operators = 0;
int comma = 0;

//parentheses check start
while((pos = instring.find_first_of("(", pos) + 1) != 0)
    parentheses += 1;

parentheses_sum = parentheses;

while((pos = instring.find_first_of(")", pos) + 1) != 0)
    parentheses -= 1;

parentheses_sum = 2 * parentheses_sum - parentheses;

if(parentheses > 0)
    std::cout << "INPUT ERROR: extra opening parentheses" << std::endl;

if(parentheses < 0)
    std::cout << "INPUT ERROR: extra closing parentheses" << std::endl;
//parentheses check end

//operators check start
while((pos = instring.find_first_of(",", pos) + 1) != 0)
        comma += 1;

sum_operators(instring, "+", operators);
sum_operators(instring, "-", operators);
sum_operators(instring, "*", operators);
sum_operators(instring, "/", operators);

if(comma - parentheses_sum - 2 * operators > 0)
        std::cout << "INPUT ERROR: extra digit" << std::endl;
if(comma - parentheses_sum - 2 * operators < 0)
        std::cout << "INPUT ERROR: extra operators" << std::endl;
//operators check end

return 0;
}









void sum_operators(const std::string instring, std::string types, int &operators){
    std::size_t pos = 0;
    while((pos = instring.find_first_of(types, pos) + 1) != 0){
        operators += 1;
    }   
}
