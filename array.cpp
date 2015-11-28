#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

std::ifstream in("input.txt");
std::ofstream out("output.txt");

const size_t ALPHABET_SIZE = 1000;
const char ALPHABET_START = 'a';
const char LAST_SYMBOL = '$';

enum SubstringMode
{
    L_MODE, S_MODE
};

std::vector<int> stringToVector(const std::string &input)
{
    std::vector<int> result(input.length() + 1);
    for (size_t i = 0; i < input.length(); i++)
    {
        result[i] = input[i] - ALPHABET_START + 1;
    }
    result.back() = 0;
    return result;
}

int getAlphabetSize(const std::vector<int> &input)
{
    int result = input[0];
    for (size_t i = 0; i < input.size(); i++)
    {
        result = std::max(result, input[i]);
    }
    return result + 1;
}

std::vector<SubstringMode> createSLflagsArray(const std::vector<int> &input)
{
    std::vector<SubstringMode> result(input.size());
    SubstringMode currFlag = SubstringMode::L_MODE;
    for (int i = input.size() - 2; i >= 0; i--)
    {
        if (input[i] < input[i + 1])
        {
            currFlag = SubstringMode::S_MODE;
        }
        if (input[i] > input[i + 1])
        {
            currFlag = SubstringMode::L_MODE;
        }
        result[i] = currFlag;
    }
    size_t numberOfS = 0;
    for (size_t i = 0; i < input.size() - 1; i++)
    {
        if (result[i] == SubstringMode::S_MODE)
        {
            numberOfS++;
        }
    }
    if (numberOfS * 2 <= input.size())
    {
        result.back() = SubstringMode::S_MODE;
    }
    else
    {
        result.back() = SubstringMode::L_MODE;
    }
    return result;
}

std::vector<size_t> createDistanceArray(const std::vector<SubstringMode> &flags)
{
    SubstringMode currMode = flags.back();
    size_t currDistance = 0;
    std::vector<size_t> result(flags.size());
    for (size_t i = 0; i < flags.size(); i++)
    {
        result[i] = currDistance;
        if (flags[i] == currMode)
        {
            currDistance = 1;
        }
        else if (currDistance != 0)
        {
            currDistance++;
        }
    }
    return result;
}

std::vector<std::vector<int> > createLevelsArrays(const std::vector<int> &input, std::vector<size_t> &distances)
{
    size_t maxDistance = 0;
    for (size_t i = 0; i < distances.size(); i++)
    {
        maxDistance = std::max(maxDistance, distances[i]);
    }
    std::vector<std::vector<int> > result(maxDistance + 1);
    std::vector<std::vector<int> > sorting(getAlphabetSize(input));
    for (size_t i = 0; i < input.size(); i++)
    {
        sorting[input[i]].push_back(i);
    }
    for (size_t i = 0; i < sorting.size(); i++)
    {
        for (size_t j = 0; j < sorting[i].size(); j++)
        {
            result[distances[sorting[i][j]]].push_back(sorting[i][j]);
        }
    }
    return result;
}

void putItemToTheCornerOfBucket(int currItem, std::vector<int> &result, 
                               const std::vector<int> &buckets, std::vector<int> &positions, 
                               std::vector<int> &bucketCorner, int delta)
{
    int from = positions[currItem], to = bucketCorner[buckets[from]];
    bucketCorner[buckets[from]] += delta;
    std::swap(positions[currItem], positions[result[to]]);
    std::swap(result[to], result[from]);
}

void compressBuckets(std::vector<int> &buckets)
{
    std::vector<int> result(buckets.size(), 0);
    int curr = 0;
    for (size_t i = 1; i < buckets.size(); i++)
    {
        if (buckets[i] != buckets[i - 1])
        {
            curr++;
        }
        result[i] = curr;
    }
    for (size_t i = 0; i < result.size(); i++)
    {
        buckets[i] = result[i];
    }
}

void sortSubstringsInBuckets(int firstInit, int firstStart, int firstFinish, int firstDelta, int secondMode,
    std::vector<int> &buckets, std::vector<int> &result, const std::vector<int> &input, 
    const std::vector<std::vector<int> > &levels, std::vector<int> &positions)
{
    std::vector<int> bucketCorner;
    std::vector<bool> bucketCornerFlags(result.size(), false);
    int currBucket = 0;
    bucketCornerFlags[firstInit] = true;
    bucketCorner.push_back(firstInit);
    buckets[firstInit] = currBucket;
    for (int i = firstStart; i != firstFinish; i += firstDelta)
    {
        if (input[result[i]] != input[result[i - firstDelta]])
        {
            currBucket++;
            bucketCorner.push_back(i);
            bucketCornerFlags[i] = true;
        }
        buckets[i] = currBucket;
    }

    for (size_t currLevel = 1; currLevel < levels.size(); currLevel++)
    {
        int secondDelta = (secondMode == 0 ? 1 : -1);
        int secondStart = (secondMode == 0 ? 0 : levels[currLevel].size() - 1);
        int secondFinish = (secondMode == 0 ? levels[currLevel].size() : -1);
        for (int i = secondStart; i != secondFinish; i += secondDelta)
        {
            putItemToTheCornerOfBucket(levels[currLevel][i] - currLevel, result,
                buckets, positions, bucketCorner, secondDelta);
        }

        for (int i = secondStart; i != secondFinish; i += secondDelta)
        {
            int curr = positions[levels[currLevel][i] - currLevel];
            if (bucketCornerFlags[curr] 
                || input[result[curr] + currLevel] != input[result[curr - secondDelta] + currLevel])
            {
                currBucket++;
                bucketCorner.push_back(curr);
                buckets[curr] = currBucket;
                bucketCornerFlags[curr] = true;
            }
            else
            {
                buckets[curr] = buckets[curr - secondDelta];
            }
        }
    }
}

std::vector<std::pair<int, int> > sortSubstrings(const std::vector<int> &input, 
                                                 const std::vector<std::vector<int> > &levels, 
                                                 const std::vector<SubstringMode> &flags)
{   
    std::vector<int> result;
    SubstringMode currMode = flags.back();
    std::vector<std::vector<int> > sorted(getAlphabetSize(input));
    for (size_t i = 0; i < input.size(); i++)
    {
        if (flags[i] == currMode)
        {
            sorted[input[i]].push_back(i);
        }
    }
    for (size_t i = 0; i < sorted.size(); i++)
    {
        for (size_t j = 0; j < sorted[i].size(); j++)
        {
            result.push_back(sorted[i][j]);
        }
    }

    std::vector<int> buckets(result.size());
    std::vector<int> positions(input.size());
    for (size_t i = 0; i < result.size(); i++)
    {
        positions[result[i]] = i;
    }

    if (flags.back() == SubstringMode::S_MODE)
    {
        sortSubstringsInBuckets(0, 1, result.size(), 1, 0, buckets, result, input, levels, positions);
    }
    else
    {
        sortSubstringsInBuckets(result.size() - 1, result.size() - 2, -1, -1, 1, buckets, result, input, levels, positions);
    }

    compressBuckets(buckets);
    std::vector<std::pair<int, int> > resultPairs;
    for (size_t i = 0; i < result.size(); i++)
    {
        resultPairs.push_back(std::make_pair(result[i], buckets[i]));
    }
    return resultPairs;
}

std::pair<std::vector<int>, std::vector<int> > createShorterString(const std::vector<int> &input, 
                                                                   const std::vector<SubstringMode> &slFlags)
{
    int currAlphabetSize = getAlphabetSize(input);
    std::vector<size_t> distances = createDistanceArray(slFlags);
    std::vector<std::vector<int> > levelsOfChars = createLevelsArrays(input, distances);
    std::vector<std::pair<int, int> > sortedSubstrings = sortSubstrings(input, levelsOfChars, slFlags);
    std::vector<int> newInput(input.size(), -1), result;

    for (size_t i = 0; i < sortedSubstrings.size(); i++)
    {
        newInput[sortedSubstrings[i].first] = sortedSubstrings[i].second;
    }
    for (size_t i = 0; i < newInput.size(); i++)
    {
        if (newInput[i] != -1)
        {
            result.push_back(newInput[i]);
        }
    }
    return std::make_pair(result, newInput);
}

void moveSuffixToTheCornerOfTheBucket(int currSuffix, std::vector<int> &result, std::vector<int> &positions,
                                   std::vector<int> &bucketCorner, const std::vector<int> &buckets, int delta)
{
    int from = positions[currSuffix], to = bucketCorner[buckets[from]];
    std::swap(positions[currSuffix], positions[result[to]]);
    std::swap(result[to], result[from]);
    bucketCorner[buckets[from]] += delta;
}

void sortSuffixesInBuckets(int firstStart, int firstFinish, int firstDelta, int secondStart, int secondFinish, int secondDelta,
    SubstringMode currMode, std::vector<int> &sortedModeSuffix, std::vector<int> &result, std::vector<int> &positions, 
    std::vector<int> &bucketFirstCorners, std::vector<int> &bucketSecondCorners, const std::vector<int> &buckets, 
    const std::vector<SubstringMode> &slFlags)
{
    for (int i = firstStart; i != firstFinish; i += firstDelta)
    {
        moveSuffixToTheCornerOfTheBucket(sortedModeSuffix[i], result, positions, bucketFirstCorners, buckets, firstDelta);
    }
    for (int i = secondStart; i != secondFinish; i += secondDelta)
    {
        if (result[i] != 0 && slFlags[result[i] - 1] == currMode)
        {
            moveSuffixToTheCornerOfTheBucket(result[i] - 1, result, positions, bucketSecondCorners, buckets, secondDelta);
        }
    }
}

std::vector<int> sortSuffixesWithShortResult(const std::vector<int> &input, const std::vector<int> &shortResult,
                                             const std::pair<std::vector<int>, const std::vector<int> > &shortInput, 
                                             const std::vector<SubstringMode> &slFlags)
{
    std::vector<int> sortedModeSuffix;
    std::vector<int> positionsOfShortInLong;
    for (size_t i = 0; i < shortInput.second.size(); i++)
    {
        if (shortInput.second[i] != -1)
        {
            positionsOfShortInLong.push_back(i);
        }
    }

    for (size_t i = 0; i < shortResult.size(); i++)
    {
        sortedModeSuffix.push_back(positionsOfShortInLong[shortResult[i]]);
    }

    std::vector<std::vector<int> > sortedByFirstChar(getAlphabetSize(input));
    for (size_t i = 0; i < input.size(); i++)
    {
        sortedByFirstChar[input[i]].push_back(i);
    }

    std::vector<int> result;
    std::vector<int> buckets;
    std::vector<int> bucketBegins;
    std::vector<int> bucketEnds;
    std::vector<int> positions(input.size());
    int currBucket = -1;

    for (size_t i = 0; i < sortedByFirstChar.size(); i++)
    {
        if (!sortedByFirstChar[i].empty())
        {
            currBucket++;
            bucketBegins.push_back(result.size());
            if (currBucket != 0)
            {
                bucketEnds.push_back(result.size() - 1);
            }
        }
        for (size_t j = 0; j < sortedByFirstChar[i].size(); j++)
        {
            positions[sortedByFirstChar[i][j]] = result.size();
            result.push_back(sortedByFirstChar[i][j]);
            buckets.push_back(currBucket);
        }
    }
    bucketEnds.push_back(result.size() - 1);

    if (slFlags.back() == SubstringMode::S_MODE)
    {
        sortSuffixesInBuckets(sortedModeSuffix.size() - 1, -1, -1, 0, result.size(), 1, SubstringMode::L_MODE,
            sortedModeSuffix, result, positions, bucketEnds, bucketBegins, buckets, slFlags);
    }
    else
    {
        sortSuffixesInBuckets(0, sortedModeSuffix.size(), 1, result.size() - 1, -1, -1, SubstringMode::S_MODE,
            sortedModeSuffix, result, positions, bucketBegins, bucketEnds, buckets, slFlags);
    }

    return result;
}

void printSuffix(const std::vector<int> &input, int suffixBegin)
{
    std::vector<int> s = input;
    for (size_t i = suffixBegin; i < s.size(); i++)
        out << s[i] << ' ';
}

void compareSuffix(const std::vector<int> &s, int first, int second)
{
    std::vector<int> input = s;
    std::vector<int> firstS;
    std::vector<int> secondS;
    for (size_t i = first; i < input.size(); i++)
    {
        firstS.push_back(input[i]);
    }
    for (size_t i = second; i < input.size(); i++)
    {
        secondS.push_back(input[i]);
    }

    if (firstS < secondS)
    {
        out << " OK";
    }
    else
    {
        out << " BAD";
    }
}

std::vector<int> createSuffixArrayFromIntArray(const std::vector<int> &input)
{
    if (input.size() == 1)
    {
        return std::vector<int>({ 0 });
    }
    if (input.size() == 2)
    {
        return std::vector<int>({ 1, 0 });
    }
    std::vector<SubstringMode> slFlags = createSLflagsArray(input);
    std::pair<std::vector<int>, std::vector<int> > shortInput = createShorterString(input, slFlags);
    std::vector<int> shortResult = createSuffixArrayFromIntArray(shortInput.first);
    std::vector<int> result = sortSuffixesWithShortResult(input, shortResult, shortInput, slFlags);
    return result;
}

std::vector<int> createSuffixArray(const std::string &s)
{
    std::vector<int> input = stringToVector(s);
    std::vector<int> result = createSuffixArrayFromIntArray(input);
    return result;
}

std::vector<int> createLCParray(const std::string &s, std::vector<int> &sufArr)
{
    std::string input = s + LAST_SYMBOL;
    std::vector<int> result(sufArr.size());
    std::vector<int> positions(sufArr.size());
    for (size_t i = 0; i < sufArr.size(); i++)
    {
        positions[sufArr[i]] = i;
    }
    int currCommonPreffix = 0;
    for (int i = 0; i < input.length(); i++)
    {
        if (currCommonPreffix > 0)
        {
            currCommonPreffix--;
        }
        if (positions[i] == input.length() - 1)
        {
            result[input.length() - 1] = -1;
            currCommonPreffix = 0;
        }
        else
        {
            int j = sufArr[positions[i] + 1];
            while (std::max(i + currCommonPreffix, j + currCommonPreffix) < input.length()
                && input[i + currCommonPreffix] == input[j + currCommonPreffix])
            {
                currCommonPreffix++;
            }
            result[positions[i]] = currCommonPreffix;
        }
    }
    return result;
}

void printSuffix(const std::string &input, int suffixBegin)
{
    std::string s = input + LAST_SYMBOL;
    for (size_t i = suffixBegin; i < s.length(); i++)
        out << s[i];
}

void compareSuffix(const std::string &s, int first, int second)
{
    std::string input = s + LAST_SYMBOL;
    std::string firstS = "";
    std::string secondS = "";
    for (size_t i = first; i < input.length(); i++)
    {
        firstS += input[i];
    }
    for (size_t i = second; i < input.length(); i++)
    {
        secondS += input[i];
    }

    if (firstS < secondS)
    {
        out << " OK";
    }
    else
    {
        out << " BAD";
    }
}

std::string createString(const std::string &input, int k, int start)
{
    std::string result = "";
    for (int i = start; i < start + k; i++)
    {
        result += input[i % input.length()];
    }
    return result;
}

int main()
{
    std::string input;
    int k;
    //in >> k;
    in >> input;
    //std::cin >> k;
    //std::cin >> input;

    //for (int i = 0; i < input.length(); i++)
    //{
        std::string currInput = createString(input, input.length(), 0);

        std::vector<int> sufArr = createSuffixArray(currInput);
        std::vector<int> lcp = createLCParray(currInput, sufArr);
        /*for (size_t i = 0; i < sufArr.size(); i++)
        {
        printSuffix(input, sufArr[i]);
        out << " " << lcp[i];
        if (i < sufArr.size() - 1)
        {
        compareSuffix(input, sufArr[i], sufArr[i + 1]);
        }
        out << std::endl;
        }*/

        long long ans = 0ll;
        for (size_t i = 0; i < sufArr.size() - 1; i++)
        {
            ans += (currInput.length() - sufArr[i + 1] - lcp[i]) * 1ll;
        }
        out << ans << ' ';
        //std::cout << ans << ' ';
    //}

    return 0;
}
