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
// 123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899100101102103104105106107108109110111112
std::string testStr;
// = "215118776150704414374672273928623127583062110985807698749010652260104536883409542254520487399791169318259361396849296655759989781932176747281094108384100638852271022465938111642665456334810311011243347138791053569101571541107";
bool needAllSolutions = true;

template<typename T>
auto displayVector(int step, const std::vector<T> &z) -> void {
    std::cout << "Step: " << step << " ====================" << std::endl;
    for (int i = 0; i < z.size(); ++i)
        std::cout << i << "->" << z[i] << std::endl;
}

// shared data
int N = 130;
int Nmax = N + 1;
int absentNumber = 23;
std::vector<int> numbers1toN;
std::string fullS;

void init() {
    numbers1toN.resize(N);
    for (int i = 0; i < N; ++i)
        numbers1toN[i] = i + 1;

    std::ostringstream full;
    for (int i: numbers1toN) {
        auto numAsStr = std::to_string(i);
        full << numAsStr;
    }
    fullS = full.str();
    std::cout << N << ": String with element:\n" << fullS << std::endl;
}

void prepareTest() {
    // Shuffle
    std::srand(std::time(nullptr));
    for (int i = N - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        int t = numbers1toN[i];
        numbers1toN[i] = numbers1toN[j];
        numbers1toN[j] = t;
    }

    std::ostringstream test;
    std::cout << "absentNumber = " << absentNumber << std::endl;
    for (int i: numbers1toN) {
        if (i != absentNumber) {
            test << i;
        }
    }
    testStr = test.str();
    std::cout << N << ": String without element:\n" << testStr << std::endl;
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

int mainDFS() {
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

    if (digsToCombineLostElement.empty()) {
        std::cout << "No solution" << std::endl;
        return 1;
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
        std::vector<int> &)> solve = [&](
        int pos,
        const std::set<int> &usedNumbers,
        const std::set<int> &remainingCandidates,
        std::vector<int> &splits) -> bool
    {
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
            if (val <= 0 || val > N)
                continue; // out of range

            if (usedNumbers.contains(val))
                continue; // already used (duplicate)

            // Update state for next recursion
            std::set<int> newUsed = usedNumbers;
            newUsed.insert(val);

            std::set<int> newRemaining = remainingCandidates;
            newRemaining.erase(val); // if val was a candidate, it's no longer the missing number

            if (newRemaining.empty())
                continue; // all candidates eliminated, dead end

            // Recurse
            splits.push_back(len);
            if (solve(pos + len, newUsed, newRemaining, splits)) {
                memo[currentState] = true;
                if (!needAllSolutions)
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
    std::set<int> permutationsFromAvailableDigitsSet(permutationsFromAvailableDigits.begin(),
                                                     permutationsFromAvailableDigits.end());
    std::vector<int> splits;

    if (!solve(0, initialUsed, permutationsFromAvailableDigitsSet, splits)) {
        std::cout << "No solution?" << std::endl;
    }

    return 0;
}

int mainDLX() {
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

    if (digsToCombineLostElement.empty()) {
        std::cout << "No solution" << std::endl;
        return 1;
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

    // Dancing Links (DLX) approach
    //
    // EXPLANATION:
    // We model this as an exact cover problem:
    // - Columns: Each position in testStr must be covered exactly once
    // - Rows: Each possible way to extract a number (position + length + value)
    //
    // Dancing Links is Knuth's Algorithm X implementation that uses doubly-linked
    // circular lists to efficiently cover/uncover columns during backtracking.
    //
    // STRUCTURE:
    //   - Each node represents a "1" in the sparse matrix
    //   - Nodes link Left/Right within a row, Up/Down within a column
    //   - Column headers track the count of nodes in each column
    //   - We can "cover" and "uncover" columns in O(1) time per link
    //

    // Build the DLX structure
    // Dancing Links (DLX) approach structures
    struct DLXNode {
        DLXNode *left = nullptr;
        DLXNode *right = nullptr;
        DLXNode *up = nullptr;
        DLXNode *down = nullptr;
        DLXNode *column = nullptr;
        int rowId = 0; // which number extraction this represents
        int value = 0; // the actual number value extracted
        int pos = 0; // starting position in testStr
        int len = 0; // length of extraction
        int size = 0; // for column headers: count of nodes in column
    };

    struct Extraction {
        int pos, len, value;
    };

    int strLen = testStr.length();
    int maxPossibleDigits = std::to_string(N).length();

    // Create column headers (one per position in testStr)
    DLXNode *header = new DLXNode();
    header->left = header->right = header->up = header->down = header;
    header->column = header;
    header->size = 0;

    std::vector<DLXNode *> colHeaders(strLen);
    DLXNode *prev = header;
    for (int i = 0; i < strLen; ++i) {
        DLXNode *col = new DLXNode();
        col->size = 0;
        col->up = col->down = col->column = col;
        col->left = prev;
        col->right = header;
        prev->right = col;
        header->left = col;
        colHeaders[i] = col;
        prev = col;
    }

    // Generate all possible extractions and build rows
    std::vector<Extraction> extractions;
    std::map<int, std::vector<DLXNode *> > rowNodes; // rowId -> nodes in that row

    int rowId = 0;
    for (int pos = 0; pos < strLen; ++pos) {
        int maxDigits = std::min<int>(maxPossibleDigits, strLen - pos);
        for (int len = 1; len <= maxDigits; ++len) {
            std::string substr = testStr.substr(pos, len);
            int val = std::stoi(substr);

            if (val <= 0 || val > N) continue;

            extractions.push_back({pos, len, val});

            // Create nodes for positions [pos, pos+len)
            std::vector<DLXNode *> rowNodeList;
            for (int p = pos; p < pos + len; ++p) {
                DLXNode *node = new DLXNode();
                node->rowId = rowId;
                node->value = val;
                node->pos = pos;
                node->len = len;
                node->column = colHeaders[p];

                // Insert into column
                node->down = colHeaders[p];
                node->up = colHeaders[p]->up;
                colHeaders[p]->up->down = node;
                colHeaders[p]->up = node;
                colHeaders[p]->size++;

                rowNodeList.push_back(node);
            }

            // Link row nodes horizontally
            for (size_t i = 0; i < rowNodeList.size(); ++i) {
                rowNodeList[i]->right = rowNodeList[(i + 1) % rowNodeList.size()];
                rowNodeList[i]->left = rowNodeList[(i + rowNodeList.size() - 1) % rowNodeList.size()];
            }

            rowNodes[rowId] = rowNodeList;
            rowId++;
        }
    }

    // Cover/Uncover operations
    auto cover = [](DLXNode *col) {
        col->right->left = col->left;
        col->left->right = col->right;
        for (DLXNode *row = col->down; row != col; row = row->down) {
            for (DLXNode *node = row->right; node != row; node = node->right) {
                node->down->up = node->up;
                node->up->down = node->down;
                node->column->size--;
            }
        }
    };

    auto uncover = [](DLXNode *col) {
        for (DLXNode *row = col->up; row != col; row = row->up) {
            for (DLXNode *node = row->left; node != row; node = node->left) {
                node->column->size++;
                node->down->up = node;
                node->up->down = node;
            }
        }
        col->right->left = col;
        col->left->right = col;
    };

    // Solution tracking
    std::vector<int> solution;
    std::set<int> usedNumbers;
    bool foundSolution = false;

    // Recursive solve function
    std::function<bool()> solve = [&]() -> bool {
        if (header->right == header) {
            // All columns covered - check if we have exactly one candidate remaining
            std::set<int> permutationsSet(permutationsFromAvailableDigits.begin(),
                                          permutationsFromAvailableDigits.end());
            for (int num: usedNumbers) {
                permutationsSet.erase(num);
            }

            if (permutationsSet.size() == 1) {
                int missing = *permutationsSet.begin();
                std::ostringstream oss;
                for (int i = 0; i < N; ++i) {
                    if (i > 0) oss << ",";
                    if (usedNumbers.contains(i + 1))
                        oss << i + 1;
                }
                std::cout << "Solution: " << missing << std::endl
                        << "Check: " << oss.str() << std::endl;

                // Show extractions
                for (int rid: solution) {
                    const auto &ext = extractions[rid];
                    std::cout << testStr.substr(ext.pos, ext.len) << ",";
                }
                std::cout << std::endl;
                return true;
            }
            return false;
        }

        // Choose column with minimum size (heuristic)
        DLXNode *col = nullptr;
        int minSize = INT_MAX;
        for (DLXNode *c = header->right; c != header; c = c->right) {
            if (c->size < minSize) {
                minSize = c->size;
                col = c;
            }
        }

        if (col->size == 0) return false; // No way to cover this column

        cover(col);

        for (DLXNode *row = col->down; row != col; row = row->down) {
            solution.push_back(row->rowId);

            // Check if this value is already used
            if (usedNumbers.contains(row->value)) {
                solution.pop_back();
                continue;
            }

            usedNumbers.insert(row->value);

            // Cover all columns in this row
            for (DLXNode *node = row->right; node != row; node = node->right) {
                cover(node->column);
            }

            if (solve()) {
                if (!needAllSolutions) {
                    uncover(col);
                    return true;
                }
                foundSolution = true;
            }

            // Uncover all columns in this row
            for (DLXNode *node = row->left; node != row; node = node->left) {
                uncover(node->column);
            }

            usedNumbers.erase(row->value);
            solution.pop_back();
        }

        uncover(col);
        return false;
    };

    if (!solve() && !foundSolution) {
        std::cout << "No solution?" << std::endl;
    }

    return 0;
}

int main() {
    //return mainDFS();
    return mainDLX();
}
