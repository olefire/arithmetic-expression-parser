#include <string>
#include <iostream>
#include <utility>
#include <vector>
#include <functional>
#include <cassert>
#include <cmath>
#include <list>

class Parser {
public:
    using BinaryOperation = std::function<double(double, double)>;
private:
    std::unordered_map<char, BinaryOperation> ops{
            {'+', [](double x, double y) { return x + y; }},
            {'-', [](double x, double y) { return x - y; }},
            {'*', [](double x, double y) { return x * y; }},
            {'/', [](double x, double y) { return x / y; }},
    };

    std::unordered_map<char, int> priorityByOperation{
            {'+', 1},
            {'-', 1},
            {'*', 2},
            {'/', 2},
    };

    std::unordered_map<int, std::vector<char>> operationsByPriority{
            {1, {'+', '-'}},
            {2, {'*', '/'}},
    };

public:
    Parser(std::string expr) : expression(std::move(expr)), pos(0) {
    }

    double parse() {
        return parseExpression();
    }

    void addOperation(const std::pair<char, BinaryOperation> &operationDefinition, char predOperationPriority) {
        auto &[operationSymbol, operation] = operationDefinition;
        auto currentPriority = priorityByOperation.at(predOperationPriority) + 1;
        priorityByOperation.insert({
                                           operationSymbol,
                                           currentPriority
                                   });
        if (auto priorityArray = operationsByPriority.find(currentPriority); priorityArray !=
                                                                             operationsByPriority.end()) {
            priorityArray->second.push_back(operationSymbol);
        } else {
            operationsByPriority.insert({currentPriority, {operationSymbol}});
        }
        ops.insert({operationSymbol, operation});
    }

private:
    bool test(char c) {
        return expression[pos] == c;
    }

    double parseExpression(int priority = 1) {
        if (priority == 4) {
            return parseValue();
        }
        if (std::isdigit(expression.at(pos)) || test('(')) {
            double res = parseExpression(priority + 1);
            auto &opsWithCurrentPriority = operationsByPriority[priority];
            while (std::any_of(opsWithCurrentPriority.begin(), opsWithCurrentPriority.end(),
                               [this](auto c) { return test(c); })) {
                BinaryOperation operation = ops.at(expression.at(pos));
                ++pos;
                res = operation(res, parseExpression(priority + 1));
            }
            return res;
        }
        throw std::runtime_error(
                "parseExpression: can't parse at pos " + std::to_string(pos) + " with symbol: " + expression[pos]
        );
    }

    double parseValue() {
        if (std::isdigit(expression.at(pos))) {
            return parseNumber();
        }
        if (test('(')) {
            ++pos;
            auto res = parse();
            ++pos;
            return res;
        }
        throw std::runtime_error(
                "parseMultiplicator: can't parse at pos " + std::to_string(pos) + " with symbol: " + expression[pos]
        );
    }

    double parseNumber() {
        size_t startPos = pos;
        while (pos < expression.length() && std::isdigit(expression.at(pos))) {
            pos++;
        }
        std::string numberInString = expression.substr(startPos, pos - startPos);
        return std::stoi(numberInString);
    }

    const std::string expression;
    size_t pos;
};


void test(const std::string &string, double expected) {
    Parser parser(string);
    assert(parser.parse() == expected);
}

void testWithOps(const std::string &string, double expected,
                 const std::vector<std::pair<std::pair<char, Parser::BinaryOperation>, char>> &operationsToAdd) {
    Parser parser(string);
    for (const auto &[opDef, after]: operationsToAdd) {
        parser.addOperation(opDef, after);
    }
    assert(parser.parse() == expected);
}

int main() {
    test("1", 1);
    test("1+1", 2);
    test("2*2+2", 6);
    test("2+2*2", 6);
    test("2*(2+2)", 8);
    test("2*(2/2)", 2);
    testWithOps("2*3^2", 18, {
            {
                    {
                            '^',
                            [](double x, double y) { return std::pow(x, y); }
                    },
                    '*'
            }
    });
    testWithOps("9@2*3", 9, {
            {
                    {
                            '@',
                            [](double x, double y) { return std::max(x, y); }
                    },
                    '+'
            }
    });

    std::cout << "tests were passed\n";

    return 0;
}