#include <algorithm>
#include <iostream>
#include <vector>
#include <ctime>
#include <format>
#include <sstream>

template<typename T>
auto displayVector(int step, const std::vector<T> &z) -> void {
    std::cout << "Step: " << step << " ====================" << std::endl;
    for (int i = 0; i < z.size(); ++i)
        std::cout << i << "->" << z[i] << std::endl;
}

int main() {
    int N = 30;
    int absentNumber = 21;
    // 2615913192237251118245121208146416232281017302927
    // 12 21 - ok
    std::vector<int> digits(N);

    // init
    for (int i = 0; i < digits.size(); ++i)
        digits[i] = i + 1;

    // Shuffle
    std::srand(std::time(nullptr));
    for (int i = digits.size() - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        int t = digits[i];
        digits[i] = digits[j];
        digits[j] = t;
    }

    std::vector<int> countOfNumbersLengthI;
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
    std::string fullS = full.str();
    std::cout << "String with element:\n" << fullS << std::endl;

    std::ostringstream mstr;
    std::cout << "absentNumber = " << absentNumber << std::endl;
    for (int i: digits) {
        if (i != absentNumber) {
            mstr << i;
        }
    }
    std::string mstrS = /*mstr.str()*/ "2615913192237251118245121208146416232281017302927";
    std::cout << "String without element:\n" << mstrS << std::endl;

    std::vector<int> z(10);
    for (int i = 0; i < fullS.length(); ++i) {
        int k = fullS[i] - '0';
        ++z[k];
    }
    for (int i = 0; i < mstrS.length(); ++i) {
        int k = mstrS[i] - '0';
        --z[k];
    }
    //displayVector(2, z);

    std::vector<int> digsToCombineLostElement;
    for (int i = 0; i < z.size(); ++i) {
        for (int k = 0; k < z[i]; ++k) {
            digsToCombineLostElement.push_back(i);
        }
    }
    //displayVector(11, digsToCombineLostElement);

    if (digsToCombineLostElement.empty() || digsToCombineLostElement.size() > countOfNumbersLengthI.size()) {
        std::cout << "No solution" << std::endl;
        return 1;
    }
    --countOfNumbersLengthI[digsToCombineLostElement.size() - 1];
    //displayVector(13, countOfNumbersLengthI);

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
        if (mstrS.find(std::to_string(permutationsFromAvailableDigits[i])) != std::string::npos) {
            ++directInclusion;
        } else {
            solutionIndex = i;
        }
    }
    if (directInclusion == permutationsFromAvailableDigits.size() - 1) {
        std::cout << "Fast Solution #2:" << permutationsFromAvailableDigits[solutionIndex] << std::endl;
        return 0;
    }
    if (N > 45) {
        std::cout << "Too complicated !!!" << std::endl;
        return 0;
    }


    do {
        // check permutations of cuts
        int pos = 0;
        bool validCut = true;
        std::vector invalids(digits.size(), 0);
        std::vector permutations = permutationsFromAvailableDigits;
        for (int numOfDig: orderedRelatedOffsetsToCutInputString) {
            int val = std::stoi(mstrS.substr(pos, numOfDig));
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
            for (size_t i = 0; i < digits.size(); ++i) {
                if (i > 0) oss << ",";
                if (invalids[i])
                    oss << i + 1;
            }
            std::cout << "Solution: " << *permutations.begin() << std::endl << "Check: " << oss.str() << std::endl;
            pos = 0;
            for (int numOfDig: orderedRelatedOffsetsToCutInputString) {
                std::cout << mstrS.substr(pos, numOfDig) << ",";
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
