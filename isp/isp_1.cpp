#include <iostream>
#include <string>
#include <list>

using namespace std;

int operations(int , int , string );
string paste_operations(string , string , string , int );
list <string> calculations(list <string> );
list <string> reshape(list <string> );

int main(void) {
    list <string> sequence;
    list <string> sequence2;
    cout << "start" << endl;
    int index = 0;
        string line, temp;
        getline(cin, line);
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
        cout<< i << endl;
// end calc

// start reshape
    for(;t > 0; --t){
        sequence = reshape(sequence);
    }
    for(const auto& i : sequence)
        cout<< i << "\t";
// end reshape

return 0;
}

list <string> calculations(list <string> sequence){
    list<string>::iterator pos = sequence.begin();
    list<string>::iterator start;
    list<string>::iterator end;

    string a, b, temp, operation;
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

    a = to_string(operations(stoi(a), stoi(b), operation));

    start = pos;
    end = pos;
    advance(start, -2);
    advance(end, 1);

    pos = sequence.erase(start, end);
    pos = sequence.insert(pos, a);
    
    return sequence;
}

int operations(int a, int b, string operation){
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

list <string> reshape(list <string> sequence){
    list<string>::iterator pos = sequence.begin();
    list<string>::iterator start;
    list<string>::iterator end;

    string a, b, temp, operation;
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

string paste_operations(string a, string b, string operation, int len){
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
