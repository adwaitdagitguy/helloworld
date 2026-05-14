#include <bits/stdc++.h>
using namespace std;

struct Statement {
    string text;
    string label;
    string target;
    bool hasLabel = false;
    bool isCond = false;
    bool isGoto = false;
};

int main() {
    int n;
    cout << "Enter number of statements: ";
    cin >> n;
    cin.ignore();

    vector<Statement> stmts(n);
    unordered_map<string, int> labelMap;

    cout << "Enter TAC statements:\n";
    for (int i = 0; i < n; i++) {
        getline(cin, stmts[i].text);

        string s = stmts[i].text;

        // Check label
        if (s.find(':') != string::npos) {
            int pos = s.find(':');
            stmts[i].label = s.substr(0, pos);
            stmts[i].hasLabel = true;
            labelMap[stmts[i].label] = i;
            s = s.substr(pos + 1);
        }

        // Check conditional goto
        if (s.find("if") != string::npos && s.find("goto") != string::npos) {
            stmts[i].isCond = true;
            int pos = s.find("goto");
            stmts[i].target = s.substr(pos + 5);
        }
        // Check unconditional goto
        else if (s.find("goto") != string::npos) {
            stmts[i].isGoto = true;
            int pos = s.find("goto");
            stmts[i].target = s.substr(pos + 5);
        }
    }

    // Identify leaders
    vector<bool> isLeader(n, false);
    isLeader[0] = true;

    for (int i = 0; i < n; i++) {
        if (stmts[i].isCond || stmts[i].isGoto) {
            if (labelMap.count(stmts[i].target))
                isLeader[labelMap[stmts[i].target]] = true;
            if (i + 1 < n)
                isLeader[i + 1] = true;
        }
    }

    // Form basic blocks
    vector<vector<int>> blocks;
    vector<int> current;

    for (int i = 0; i < n; i++) {
        if (isLeader[i] && !current.empty()) {
            blocks.push_back(current);
            current.clear();
        }
        current.push_back(i);
    }
    if (!current.empty()) blocks.push_back(current);

    // Map statement to block
    vector<int> stmtToBlock(n);
    for (int i = 0; i < blocks.size(); i++) {
        for (int idx : blocks[i]) {
            stmtToBlock[idx] = i;
        }
    }

    // Build CFG edges
    vector<vector<int>> graph(blocks.size());

    for (int i = 0; i < blocks.size(); i++) {
        int lastStmt = blocks[i].back();

        if (stmts[lastStmt].isCond) {
            // true branch
            int target = labelMap[stmts[lastStmt].target];
            graph[i].push_back(stmtToBlock[target]);

            // false branch
            if (i + 1 < blocks.size())
                graph[i].push_back(i + 1);
        }
        else if (stmts[lastStmt].isGoto) {
            int target = labelMap[stmts[lastStmt].target];
            graph[i].push_back(stmtToBlock[target]);
        }
        else {
            if (i + 1 < blocks.size())
                graph[i].push_back(i + 1);
        }
    }

    // Output basic blocks
    cout<< "Number of basic blocks: " << blocks.size() << "\n";
    cout << "\nBasic Blocks:\n";
    for (int i = 0; i < blocks.size(); i++) {
        cout << "B" << i + 1 << ":\n";
        for (int idx : blocks[i]) {
            cout << "  " << stmts[idx].text << "\n";
        }
    }

    // Output CFG
    // Output CFG in DOT format for Graphviz
cout << "\nDOT format for Graphviz:\n";
cout << "digraph CFG {\n";
cout << "  node [shape=box];\n";  // Optional: Make nodes rectangular for better readability

// Define nodes with labels (basic block contents)
for (int i = 0; i < blocks.size(); i++) {
    cout << "  B" << i + 1 << " [label=\"";
    for (size_t j = 0; j < blocks[i].size(); j++) {
        cout << stmts[blocks[i][j]].text;
        if (j < blocks[i].size() - 1) cout << "\\n";  // Newline in label
    }
    cout << "\"];\n";
}

// Define edges
for (int i = 0; i < graph.size(); i++) {
    for (int j : graph[i]) {
        cout << "  B" << i + 1 << " -> B" << j + 1 << ";\n";
    }
}

cout << "}\n";


    return 0;
}