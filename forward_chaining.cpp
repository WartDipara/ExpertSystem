#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <unordered_map>
using namespace std;

// 主推理函数
void forwardChaining(const string &ruleFile, const string &outputFile, const string &factsFile, const string &inputFile, double threshold = 0.75)
{
    ifstream infile(ruleFile);
    ifstream facts_input(factsFile);
    ifstream userInputFile(inputFile);
    ofstream outfile(outputFile);
    if (!infile.is_open() || !userInputFile.is_open() || !outfile.is_open())
    {
        cerr << "无法打开规则文件、输入文件或输出文件！\n";
        return;
    }

    // 规则库：条件 -> 结论
    map<set<string>, string> rules;
    unordered_map<string, int> check_fact;

    // 读取规则文件
    string line;
    while (getline(infile, line))
    {
        istringstream iss(line);
        string condition, arrow, conclusion;

        // 将条件进行分解
        set<string> conditions;
        while (iss >> condition && condition != "->")
        {
            conditions.insert(condition);
        }
        iss >> conclusion;
        rules[conditions] = conclusion;
    }

    // 读取最终事实
    while (getline(facts_input, line))
    {
        if (line == "exit")
        {
            break;
        }
        check_fact[line] = 1;
    }

    // 从文件读取用户输入的初始事实
    set<string> facts;
    if (getline(userInputFile, line))
    {
        istringstream iss(line);
        string fact;
        while (iss >> fact)
        {
            facts.insert(fact);
        }
    }

    outfile << "初始事实：";
    for (const auto &f : facts)
    {
        outfile << f << " ";
    }
    outfile << "\n";

    // 前向推理开始
    bool newFactAdded = true;
    map<string,double> match_score_map;
    while (newFactAdded)
    {
        newFactAdded = false;

        for (const auto &rule : rules)
        {
            const auto &conditions = rule.first;
            const auto &conclusion = rule.second;

            // 模糊推理：检查规则条件满足程度
            int satisfiedCount = 0;
            for (const auto &cond : conditions)
            {
                if (facts.find(cond) != facts.end())
                {
                    satisfiedCount++;
                }
            }

            double matchScore = static_cast<double>(satisfiedCount) / conditions.size();

            // 如果条件满足比例超过阈值，且结论不在事实中，则添加新事实
            if (matchScore >= threshold && facts.find(conclusion) == facts.end())
            {
                facts.insert(conclusion);
                newFactAdded = true;
                outfile << "推导出新事实：" << conclusion << " (匹配度：" << matchScore << ")\n";
                match_score_map[conclusion] = matchScore;
            }
        }
    }

    // 对最终输出做剪枝
    vector<string> ans;
    for (auto &fact : facts)
    {
        if (check_fact[fact] == 1)
        {
            ans.push_back(fact);
        }
    }

    outfile << "最终结果:";
    if (ans.size() == 0)
    {
        outfile << "无最终推演结果";
    }
    else
    {
        for (const auto &a : ans)
        {
            outfile << a <<"(匹配度："<<match_score_map[a] << ")" << " ";
        }
    }
    outfile << "\n";
    outfile << "最终事实集合：";
    for (const auto &f : facts)
    {
        outfile << f <<" ";
    }

    outfile << "\n";

    cout << "forward chaining finish, check 'output.txt' " << "\n";
}