#include <iostream>
#include <string>
#include <list>

int operations(int , int , std::string );
std::string paste_operations(std::string , std::string , std::string , int );
std::list <std::string> calculations(std::list <std::string> );
std::list <std::string> reshape(std::list <std::string> );

int main(void) {
    std::list <std::string> sequence;
    std::list <std::string> sequence2;
    std::cout << "start" << std::endl;
    int index = 0;
        std::string line, temp;
        getline(std::cin, line);
        while(line.size() > 1){
            index = line.find(" ");
            temp = line.substr(0, index);
            sequence.push_back(temp);
            line = line.substr(index + 1);
            index = 0;
            }
    sequence.push_back(line);
    sequence2 = sequence;

// start calc
    int t = 0;
    while(distance(sequence2.begin(), sequence2.end()) > 1){
        sequence2 = calculations(sequence2);
        t++;
    }
    for(const auto& i : sequence2)
        std::cout << i << std::endl;
// end calc

// start reshape
    for(;t > 0; --t){
        sequence = reshape(sequence);
    }
    for(const auto& i : sequence)
        std::cout << i << "\t";
// end reshape

return 0;
}

std::list <std::string> calculations(std::list <std::string> sequence){
    std::list<std::string>::iterator pos = sequence.begin();
    std::list<std::string>::iterator start;
    std::list<std::string>::iterator end;

    std::string a, b, temp, operation;
    for(const auto& i : sequence){
        if(i == "+" || i == "-" || i == "/" || i == "*"){
            operation = i;
            break;
        }
        ++pos;        
    }
    int counter = 0;
    for(const auto& i : sequence){
        if(counter == distance(sequence.begin(), pos) - 1){
            b = i;
            break;
        }
        counter += 1;        
    }
    counter = 0;
    
    for(const auto& i : sequence){
        if(counter == distance(sequence.begin(), pos) - 2){
            a = i;
            break;
        }
        counter += 1;        
    }
    counter = 0;

    a = std::to_string(operations(stoi(a), stoi(b), operation));

    start = pos;
    end = pos;
    advance(start, -2);
    advance(end, 1);

    pos = sequence.erase(start, end);
    pos = sequence.insert(pos, a);
    
    return sequence;
}

int operations(int a, int b, std::string operation){
    if(operation == "+"){
            a = a + b;
        } else if(operation == "-"){
            a = a - b;
        } else if(operation == "*"){
            a = a * b;
        } else{
            a = a / b;
        }
    return a;
}

std::list <std::string> reshape(std::list <std::string> sequence){
    std::list<std::string>::iterator pos = sequence.begin();
    std::list<std::string>::iterator start;
    std::list<std::string>::iterator end;

    std::string a, b, temp, operation;
    for(const auto& i : sequence){
        if(i == "+" || i == "-" || i == "/" || i == "*"){
            operation = i;
            break;
        }
        ++pos;        
    }
    int counter = 0;
    for(const auto& i : sequence){
        if(counter == distance(sequence.begin(), pos) - 1){
            b = i;
            break;
        }
        counter += 1;        
    }
    counter = 0;
    
    for(const auto& i : sequence){
        if(counter == distance(sequence.begin(), pos) - 2){
            a = i;
            break;
        }
        counter += 1;        
    }
    counter = 0;

    a = paste_operations(a, b, operation, distance(sequence.begin(), sequence.end()));

    start = pos;
    end = pos;
    advance(start, -2);
    advance(end, 1);

    pos = sequence.erase(start, end);
    pos = sequence.insert(pos, a);
    return sequence;
}

std::string paste_operations(std::string a, std::string b, std::string operation, int len){
    if(operation == "+" && len == 3)
        a =  a + " + " + b;
    else if(operation == "-" && len == 3)
        a =  a + " - " + b;
    else if(operation == "+")
        a = "(" + a + " + " + b + ")";
    else if(operation == "-")
        a = "(" + a + " - " + b + ")";
    else if(operation == "*")
        a = a + " * " + b;
    else
        a = a +  " / " + b;
    return a;
}
