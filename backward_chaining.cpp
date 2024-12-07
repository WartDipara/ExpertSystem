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

struct Rule
{
    set<string> conditions;
    string conclusion;
};

// 辅助函数
bool backwardChainingHelper(const string &goal, const map<string, vector<Rule>> &rules,
                            const set<string> &facts, set<string> &visited, ofstream &outfile, double threshold, map<string, double> &match_score_map);

// 主推理函数
bool backwardChaining(const string &ruleFile, const string &outputFile, const string &inputFile, double threshold = 0.75)
{
    // 读取规则文件
    ifstream infile(ruleFile);
    ifstream userInputFile(inputFile);
    ofstream outfile(outputFile);
    if (!infile.is_open() || !userInputFile.is_open() || !outfile.is_open())
    {
        cerr << "无法打开规则文件、事实文件、输入文件或输出文件！\n";
        return false;
    }

    // 规则库：结论 -> 规则列表
    map<string, vector<Rule>> rulesMap;

    string line;
    while (getline(infile, line))
    {
        if (line.empty())
            continue;
        istringstream iss(line);
        string condition, arrow, conclusion;

        Rule rule;
        while (iss >> condition && condition != "->")
        {
            rule.conditions.insert(condition);
        }
        iss >> conclusion;
        rule.conclusion = conclusion;
        rulesMap[conclusion].push_back(rule);
    }

    // 读取已知事实
    set<string> facts;

    // 读取用户输入的属性，并将其添加到已知事实中
    while (getline(userInputFile, line))
    {
        if (line.empty() || line == "exit")
        {
            continue;
        }
        istringstream iss(line);
        string attr;
        while (iss >> attr)
        {
            facts.insert(attr);
        }
    }

    infile.close();
    userInputFile.close();

    // 找出所有可能的结论（动物）
    set<string> possibleConclusions;
    for (const auto &pair : rulesMap)
    {
        possibleConclusions.insert(pair.first);
    }

    // 过滤出动物种类（假设动物种类不在已知事实中）
    set<string> animals;
    for (const auto &animal : possibleConclusions)
    {
        if (facts.find(animal) == facts.end())
        {
            animals.insert(animal);
        }
    }

    outfile << "已知事实：";
    for (const auto &f : facts)
    {
        outfile << f << " ";
    }
    outfile << "\n";

    // 遍历所有可能的动物，进行后向推理
    vector<string> matchingAnimals;
    map<string, double> match_score_map;
    for (const auto &animal : animals)
    {
        set<string> visited;
        outfile << "尝试证明动物：" << animal << "\n";
        if (backwardChainingHelper(animal, rulesMap, facts, visited, outfile, threshold, match_score_map))
        {
            matchingAnimals.push_back(animal);
            outfile << "成功证明 " << animal << " 符合所有条件。\n";
        }
        else
        {
            outfile << "无法证明 " << animal << " 符合所有条件。\n";
        }
        outfile << "-------------------------\n";
    }

    ifstream tmpfile("facts.txt");
    unordered_map<string, int> check_fact;
    // 读取最终事实
    while (getline(tmpfile, line))
    {
        if (line == "exit")
        {
            break;
        }
        check_fact[line] = 1;
    }
    // 对最终输出做剪枝
    vector<string> ans;
    for (auto &match : matchingAnimals)
    {
        if (check_fact[match] == 1)
        {
            ans.push_back(match);
        }
    }

    // 输出匹配的结论
    outfile << "推理出的可能结论：";
    if (ans.empty())
    {
        outfile << "无匹配的结论\n";
    }
    else
    {
        for (const auto &a : ans)
        {
            outfile << a << "(匹配度：" << match_score_map[a] << ")" << " ";
        }
        outfile << "\n";
    }

    outfile.close();

    cout << "backward chaining finish, check 'output2.txt'";
    return true;
}

bool backwardChainingHelper(const string &goal, const map<string, vector<Rule>> &rules,
                            const set<string> &facts, set<string> &visited, ofstream &outfile, double threshold, map<string, double> &match_score_map)
{
    // 如果目标已经在事实中，返回成功
    if (facts.find(goal) != facts.end())
    {
        outfile << " 事实中存在目标：" << goal << "\n";
        return true;
    }

    // 防止循环推理
    if (visited.find(goal) != visited.end())
    {
        outfile << " 目标 " << goal << " 已经访问过，避免循环。\n";
        return false;
    }
    visited.insert(goal);

    // 查找能够得出目标的所有规则
    auto it = rules.find(goal);
    if (it == rules.end())
    {
        // 没有规则可以得出该目标
        outfile << "  无法找到可以推导出 " << goal << " 的规则。\n";
        return false;
    }

    const vector<Rule> &possibleRules = it->second;
    for (const auto &rule : possibleRules)
    {
        int satisfiedCount = 0;
        int totalConditions = rule.conditions.size();

        outfile << "  尝试使用规则推导出 " << goal << "：";
        for (const auto &cond : rule.conditions)
        {
            outfile << cond << " ";
        }
        outfile << "\n";

        // 对每个条件递归进行后向推理
        for (const auto &cond : rule.conditions)
        {
            if (facts.find(cond) != facts.end() || backwardChainingHelper(cond, rules, facts, visited, outfile, threshold, match_score_map))
            {
                satisfiedCount++;
            }
        }

        double matchScore = static_cast<double>(satisfiedCount) / totalConditions;
        outfile << "  匹配度：" << matchScore << " (阈值：" << threshold << ")\n";

        if (matchScore >= threshold)
        {
            outfile << "  成功推导出 " << goal << "。\n";
            match_score_map[goal] = matchScore;
            return true;
        }
    }

    // 如果所有规则都无法满足目标
    outfile << "  无法成功推导出 " << goal << "。\n";
    return false;
}