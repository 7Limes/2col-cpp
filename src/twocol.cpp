// By Miles Burkart
// 3-10-22

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

using namespace std;

string get_file_text(string file) {
  ifstream t(file);
  stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

vector<string> split(string text, char delim) {
  string line;
  vector<string> vec;
  stringstream ss(text);
  while(getline(ss, line, delim)) {
    vec.push_back(line);
  }
  return vec;
}

// trim functions from https://stackoverflow.com/a/217605
static void ltrim(string &s) {
  s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
}

static void rtrim(string &s) {
  s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
}

static string trim(string s) {
  ltrim(s);
  rtrim(s);
  return s;
}

void print_vec(vector<int> vec) {
  cout << "[";
  for (int i = 0; i < vec.size(); i++) {
    cout << vec[i];
    if (i < vec.size()-1)
      cout << ", ";
  }
  cout << "]" << endl;
}

void print_map(map<int, int> m) {
  cout << "{";
  for (const auto& n : m) {
    cout << n.first << ": " << n.second << ", ";
  }
  cout << "}" << endl;
}

// from https://stackoverflow.com/a/21995693
template <
  class result_t   = chrono::microseconds,
  class clock_t    = chrono::steady_clock,
  class duration_t = chrono::microseconds
>
auto since(chrono::time_point<clock_t, duration_t> const& start) {
  return chrono::duration_cast<result_t>(clock_t::now() - start);
}

int popfront(vector<int> *stack) {
  int e = (*stack)[0];
  (*stack).erase((*stack).begin());
  return e;
}

void pushfront(vector<int> *stack, int e) {
  (*stack).insert((*stack).begin(), e);
}
 
int format_value(vector<int> *stack, string value) {
  if (value.at(0) == 'p') {
    return popfront(stack);
  }
  if (value.at(0) == 'c') {
    return (*stack)[0];
  }
  if (value.at(0) == 'i') {
    int input;
    cin >> input;
    return input;
  }
  return stoi(value);
}

void add_label(vector<int> *stack, map<int, int> *labels, string value, int lineNum) {
  if (value.at(0) != '.') {
    int fvalue = format_value(stack, value);
    (*labels)[fvalue] = lineNum;
  }
}

void cmd_push(vector<int> *stack, string value) {
  int fvalue = format_value(stack, value);
  pushfront(stack, fvalue);
}

void cmd_pop(vector<int> *stack) {
  popfront(stack);
}

void cmd_add(vector<int> *stack) {
  int a = popfront(stack);
  int b = popfront(stack);
  pushfront(stack, a+b);
}

void cmd_sub(vector<int> *stack) {
  int a = popfront(stack);
  int b = popfront(stack);
  pushfront(stack, b-a);
}

void cmd_mul(vector<int> *stack) {
  int a = popfront(stack);
  int b = popfront(stack);
  pushfront(stack, a*b);
}

void cmd_div(vector<int> *stack) {
  int a = popfront(stack);
  int b = popfront(stack);
  pushfront(stack, b/a);
}

void cmd_mod(vector<int> *stack) {
  int a = popfront(stack);
  int b = popfront(stack);
  pushfront(stack, b % a);
}

void cmd_if(vector<int> *stack, string value, int *lineNum) {
  if (value.empty()) {
    if ((*stack)[0] == (*stack)[1])
      *lineNum += 1;
  }
  else if (value.at(0) != '.') {
    int fvalue = format_value(stack, value);
    if ((*stack)[0] == fvalue)
      *lineNum += 1;
  }
}

void cmd_print(vector<int> *stack, string value) {
  if (value.at(0) != '.') {
    int fvalue = format_value(stack, value);
    cout << fvalue;
  }
  else {
    value.erase(0,1);
    putchar(stoi(value));
  }
}

void cmd_jump(vector<int> *stack, map<int, int> *labels, string value, int *lineNum) {
  if (value.at(0) != '.') {
    int fvalue = format_value(stack, value);
    vector<int> labelKeys;
    for (auto &kv : *labels) {
      labelKeys.push_back(kv.first);
    }
    if (find(labelKeys.begin(), labelKeys.end(), fvalue) != labelKeys.end()) {
      *lineNum = (*labels)[fvalue];
    }
  }
}

void cmd_swap(vector<int> *stack, string value) {
  if (value.empty()) {
    int temp = (*stack)[0];
    (*stack)[0] = (*stack)[1];
    (*stack)[1] = temp;
  }
  else if (value.at(0) != '.') {
    int fvalue = format_value(stack, value);
    int temp = (*stack)[0];
    (*stack)[0] = (*stack)[fvalue];
    (*stack)[fvalue] = temp;
  }
}

void tc_interpret(string file, bool debug=false) {
  auto startTime = chrono::steady_clock::now();
  string fileText = get_file_text(file);
  vector<string> lines = split(fileText, '\n');
  
  vector<int> stack;
  map<int, int> labels;

  vector<string> newLines;
  // remove empty / commented lines and remove indents
  for (int i = 0; i < lines.size(); i++) {
    string line = trim(lines[i]);
    if (line.empty() || line.at(0) == '~') {
      lines.erase(lines.begin() + i);
      i--;
    }
    else
      newLines.push_back(line);
  }
  lines = newLines;

  // init labels
  for (int i = 0; i < lines.size(); i++) {
    if (lines[i].at(0) == '@') {
      vector<string> sp = split(lines[i], ' ');
      add_label(&stack, &labels, sp[1], i);
    }
  }
  
  int lineNum = 0;
  while (lineNum < lines.size()) {
    vector<string> cmdAndValue = split(lines[lineNum], ' ');
    char cmd = cmdAndValue[0].at(0);
    string value = "";
    if (cmdAndValue.size() > 1)
      value = cmdAndValue[1];

    // commands
    switch (cmd) {
      case '!':
        cmd_print(&stack, value);
        break;
      case '#':
        cmd_push(&stack, value);
        break;
      case '&':
        cmd_pop(&stack);
        break;
      case '+':
        cmd_add(&stack);
        break;
      case '-':
        cmd_sub(&stack);
        break;
      case '*':
        cmd_mul(&stack);
        break;
      case '/':
        cmd_div(&stack);
        break;
      case '%':
        cmd_mod(&stack);
        break;
      case '?':
        cmd_if(&stack, value, &lineNum);
        break;
      case '^':
        cmd_jump(&stack, &labels, value, &lineNum);
        break;
      case '$':
        cmd_swap(&stack, value);
        break;
      
    }
    
    lineNum++;
  }
  if (debug) {
    cout << "\n";
    print_vec(stack);
    print_map(labels);
    cout << "interpreted in " << since(startTime).count() / 1000.0 << " ms";
  }
}
