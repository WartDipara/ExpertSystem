#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include "forward_chaining.cpp"
#include "backward_chaining.cpp"
int main() {
    string ruleFile = "rules.txt";
    string factFile = "facts.txt";
    string outputFile = "output.txt";
    string outputFile2 = "output2.txt";
    string inputFile = "userIn.txt";
    forwardChaining(ruleFile, outputFile,factFile,inputFile);
    backwardChaining(ruleFile, outputFile2,inputFile);
    return 0;
}