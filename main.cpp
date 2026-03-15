#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <ctime>
#include <format>
#include <sstream>

// 1234567891011131415161718192021
// 345678910111314151617181920221221
std::string testStr = "345678910111314151617181920221221";

template<typename T>
auto displayVector(int step, const std::vector<T> &z) -> void {
    std::cout << "Step: " << step << " ====================" << std::endl;
    for (int i = 0; i < z.size(); ++i)
        std::cout << i << "->" << z[i] << std::endl;
}

// shared data
int N = 30;
int Nmax = 45;
int absentNumber = 21;
std::vector<int> digits;
std::vector<int> countOfNumbersLengthI;
std::string fullS;

void init() {
    digits.resize(N);
    for (int i = 0; i < N; ++i)
        digits[i] = i + 1;

    std::ostringstream full;
    for (int i: digits) {
        auto numAsStr = std::to_string(i);
        std::size_t length = numAsStr.length();
        if (length >= countOfNumbersLengthI.size()) {
            countOfNumbersLengthI.resize(length);
        }
        ++countOfNumbersLengthI[length - 1];
        full << numAsStr;
    }
    fullS = full.str();
    std::cout << "String with element:\n" << fullS << std::endl;
}

void prepareTest() {
    // Shuffle
    std::srand(std::time(nullptr));
    for (int i = N - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        int t = digits[i];
        digits[i] = digits[j];
        digits[j] = t;
    }

    std::ostringstream test;
    std::cout << "absentNumber = " << absentNumber << std::endl;
    for (int i: digits) {
        if (i != absentNumber) {
            test << i;
        }
    }
    std::cout << "String without element:\n" << testStr << std::endl;
}

bool findN() {
    N = 1;
    int L = 0;
    while (N < Nmax) {
        L += std::to_string(N).length();
        if (L > testStr.length()) {
           return true;
        }
        ++N;
    }
    return false;
}

int main() {
    if (!testStr.empty()) {
      if (!findN())
          return 1;
    }
    init();
    if (testStr.empty()) {
        prepareTest();
    }

    std::vector<int> countAbsentDigsInStr(10);
    for (int i = 0; i < fullS.length(); ++i) {
        int k = fullS[i] - '0';
        ++countAbsentDigsInStr[k];
    }
    for (int i = 0; i < testStr.length(); ++i) {
        int k = testStr[i] - '0';
        --countAbsentDigsInStr[k];
    }

    std::vector<int> digsToCombineLostElement;
    for (int i = 0; i < countAbsentDigsInStr.size(); ++i) {
        for (int k = 0; k < countAbsentDigsInStr[i]; ++k) {
            digsToCombineLostElement.push_back(i);
        }
    }

    if (digsToCombineLostElement.empty() || digsToCombineLostElement.size() > countOfNumbersLengthI.size()) {
        std::cout << "No solution" << std::endl;
        return 1;
    }
    --countOfNumbersLengthI[digsToCombineLostElement.size() - 1];

    std::vector<int> orderedRelatedOffsetsToCutInputString;
    for (int i = 0; i < countOfNumbersLengthI.size(); ++i) {
        for (int k = 0; k < countOfNumbersLengthI[i]; ++k) {
            orderedRelatedOffsetsToCutInputString.push_back(i + 1);
        }
    }

    std::vector<int> permutationsFromAvailableDigits;
    do {
        if (digsToCombineLostElement[0] == 0) continue;
        std::ostringstream ins;
        for (int n: digsToCombineLostElement) {
            ins << n;
        }
        int valPerm = std::stoi(ins.str());
        if (valPerm <= 0 || valPerm > N) continue;
        permutationsFromAvailableDigits.push_back(valPerm);
    } while (std::next_permutation(digsToCombineLostElement.begin(), digsToCombineLostElement.end()));

    // Fast check
    if (permutationsFromAvailableDigits.empty()) {
        std::cout << "Fast no solution" << std::endl;
        return 1;
    }
    if (permutationsFromAvailableDigits.size() == 1) {
        std::cout << "Fast Solution #1:" << permutationsFromAvailableDigits[0] << std::endl;
        return 0;
    }
    int directInclusion = 0;
    int solutionIndex = -1;
    for (int i = 0; i < permutationsFromAvailableDigits.size(); ++i) {
        if (testStr.find(std::to_string(permutationsFromAvailableDigits[i])) != std::string::npos) {
            ++directInclusion;
        } else {
            solutionIndex = i;
        }
    }
    if (directInclusion == permutationsFromAvailableDigits.size() - 1) {
        std::cout << "Fast Solution #2:" << permutationsFromAvailableDigits[solutionIndex] << std::endl;
        return 0;
    }
    if (N > Nmax) {
        std::cout << "Too complicated !!!" << std::endl;
        return 0;
    }

    do {
        // check permutations of cuts
        int pos = 0;
        bool validCut = true;
        std::vector invalids(N, 0);
        std::vector permutations = permutationsFromAvailableDigits;
        for (int numOfDig: orderedRelatedOffsetsToCutInputString) {
            int val = std::stoi(testStr.substr(pos, numOfDig));
            if (auto it = std::find(permutations.begin(), permutations.end(), val); it != permutations.end()) {
                permutations.erase(it);
                if (permutations.empty()) {
                    validCut = false;
                    break;
                }
            }
            if (val <= 0 || val > N || invalids[val - 1] > 0) {
                validCut = false;
                break;
            }
            invalids[val - 1] = 1;
            pos += numOfDig;
        }

        if (validCut) {
            // Report the result:
            std::ostringstream oss;
            for (size_t i = 0; i < N; ++i) {
                if (i > 0) oss << ",";
                if (invalids[i])
                    oss << i + 1;
            }
            std::cout << "Solution: " << *permutations.begin() << std::endl << "Check: " << oss.str() << std::endl;
            pos = 0;
            for (int numOfDig: orderedRelatedOffsetsToCutInputString) {
                std::cout << testStr.substr(pos, numOfDig) << ",";
                pos += numOfDig;
            }
            std::cout << std::endl;
            //return 0;
        }
    } while (std::next_permutation(orderedRelatedOffsetsToCutInputString.begin(),
                                   orderedRelatedOffsetsToCutInputString.end()));

    std::cout << "No solution" << std::endl;
    return 0;
}
