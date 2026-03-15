#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>
#include <map>
#include <set>
#include <functional>

// 1234567891011131415161718192021
// 345678910111314151617181920221221
std::string testStr;

template<typename T>
auto displayVector(int step, const std::vector<T> &z) -> void {
    std::cout << "Step: " << step << " ====================" << std::endl;
    for (int i = 0; i < z.size(); ++i)
        std::cout << i << "->" << z[i] << std::endl;
}

// shared data
int N = 80;
int Nmax = 123;
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
    testStr = test.str();
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

    // Memoization tree using Recursive DFS
    //
    // EXPLANATION:
    // Instead of generating all permutations of cut positions (which is O(N!) complexity),
    // we use a recursive tree search that tries different ways to split the string.
    //
    // At each position in the string, we try splitting off 1, 2, or 3 digits,
    // check if the resulting number is valid (not used, in range 1..N),
    // and recursively continue from the next position.
    //
    // MEMOIZATION: We cache results for each unique state (position + used numbers + remaining candidates)
    // so if we reach the same state from different paths, we don't recompute.
    //
    // STATE:
    //   - pos: current position in testStr
    //   - usedNumbers: set of numbers already extracted (to avoid duplicates)
    //   - remainingCandidates: set of candidate solutions still possible
    //
    // BASE CASE: When pos == testStr.length(), we check if exactly one candidate remains
    //

    // Define the state as a tuple for memoization
    using State = std::tuple<int, std::set<int>, std::set<int> >;
    std::map<State, bool> memo;
    int maxPossibleDigits = std::to_string(N).length();

    // Recursive solve function
    std::function<bool(
        int,
        const std::set<int> &,
        const std::set<int> &,
        std::vector<int> &)> solve;
    solve = [&](
        int pos,
        const std::set<int> &usedNumbers,
        const std::set<int> &remainingCandidates,
        std::vector<int> &splits) -> bool {
                // BASE CASE: reached end of string
                if (pos == testStr.length()) {
                    if (remainingCandidates.size() == 1) {
                        // Found solution! Report it
                        int solution = *remainingCandidates.begin();
                        std::ostringstream oss;
                        for (int i = 0; i < N; ++i) {
                            if (i > 0) oss << ",";
                            if (usedNumbers.contains(i + 1))
                                oss << i + 1;
                        }
                        std::cout << "Solution: " << solution << std::endl
                                  << "Check: " << oss.str() << std::endl;

                        // Show how we split the string
                        int p = 0;
                        for (int len: splits) {
                            std::cout << testStr.substr(p, len) << ",";
                            p += len;
                        }
                        std::cout << std::endl;
                        return true;
                    }
                    return false;
                }

                // Check memoization cache
                State currentState = std::make_tuple(pos, usedNumbers, remainingCandidates);
                if (memo.contains(currentState)) {
                    return memo[currentState];
                }

                // RECURSIVE CASE: Try different split lengths
                // For N up to 100, numbers can be 1-3 digits
                // We determine max possible digits based on N
                int maxDigits = std::min<int>(maxPossibleDigits, testStr.length() - pos);

                for (int len = 1; len <= maxDigits; ++len) {
                    std::string substr = testStr.substr(pos, len);
                    int val = std::stoi(substr);

                    // Validate the extracted number
                    if (val <= 0 || val > N) continue; // out of range
                    if (usedNumbers.count(val)) continue; // already used (duplicate)

                    // Update state for next recursion
                    std::set<int> newUsed = usedNumbers;
                    newUsed.insert(val);

                    std::set<int> newRemaining = remainingCandidates;
                    newRemaining.erase(val); // if val was a candidate, it's no longer the missing number

                    if (newRemaining.empty()) continue; // all candidates eliminated, dead end

                    // Recurse
                    splits.push_back(len);
                    if (solve(pos + len, newUsed, newRemaining, splits)) {
                        memo[currentState] = true;
                        return true;
                    }
                    splits.pop_back(); // backtrack
                }

                // No valid split found from this state
                memo[currentState] = false;
                return false;
            };

    // Initialize and start search
    std::set<int> initialUsed;
    std::set<int> permutationsFromAvailableDigitsSet(permutationsFromAvailableDigits.begin(), permutationsFromAvailableDigits.end());
    std::vector<int> splits;

    if (!solve(0, initialUsed, permutationsFromAvailableDigitsSet, splits)) {
        std::cout << "No solution" << std::endl;
    }

    return 0;
}
