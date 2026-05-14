#include <iostream>
#include <stack>
#include <vector>
#include <sstream>
using namespace std;

struct Quad {
    string op, arg1, arg2, result;
};

bool isOperator(const string &s) {
    return s == "+" || s == "-" || s == "*" ||
           s == "/" || s == "%" || s == "=";
}

int main() {
    string expr;
    cout << "Enter postfix expression (space separated):\n";
    getline(cin, expr);

    stack<string> st;
    vector<Quad> table;
    int tempCount = 1;

    stringstream ss(expr);
    string tok;

    while (ss >> tok) {
        if (isOperator(tok)) {
            if (st.size() < 2) {
                cout << "Error: Invalid expression\n";
                return 1;
            }

            string arg2 = st.top(); st.pop();
            string arg1 = st.top(); st.pop();

            string temp = "t" + to_string(tempCount++);

            table.push_back({tok, arg1, arg2, temp});

            st.push(temp);
        } else {
            st.push(tok);
        }
    }

    cout << "\nQuadruple Representation:\n";
    cout << "Operator\tArg1\tArg2\tResult\n";

    for (auto &q : table) {
        cout << q.op << "\t\t"
             << q.arg1 << "\t"
             << q.arg2 << "\t"
             << q.result << "\n";
    }

    return 0;
}

