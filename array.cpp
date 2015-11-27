#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

std::ifstream in("input.txt");
std::ofstream out("output.txt");

const size_t ALPHABET_SIZE = 27;
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

std::vector<SubstringMode> createSLflagsArray(std::vector<int> &input)
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

std::vector<size_t> createDistanceArray(std::vector<SubstringMode> &flags)
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

std::vector<std::vector<int> > createLevelsArrays(std::vector<int> &input, std::vector<size_t> &distances)
{
    size_t maxDistance = 0;
    for (size_t i = 0; i < distances.size(); i++)
    {
        maxDistance = std::max(maxDistance, distances[i]);
    }
    std::vector<std::vector<int> > result(maxDistance + 1);
    std::vector<std::vector<int> > sorting(ALPHABET_SIZE);
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

void putItemToTheStartOfBucket(int currItem, std::vector<int> &result, 
                               std::vector<int> &buckets, std::vector<int> &positions, 
                               std::vector<int> &bucketBegin)
{
    int from = positions[currItem], to = bucketBegin[buckets[from]];
    bucketBegin[buckets[from]]++;
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

std::vector<std::pair<int, int> > sortSubstrings(std::vector<int> &input, 
                                                 std::vector<std::vector<int> > &levels, 
                                                 std::vector<SubstringMode> &flags)
{   
    std::vector<int> result;
    SubstringMode currMode = flags.back();
    std::vector<std::vector<int> > sorted(ALPHABET_SIZE);
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
    std::vector<int> bucketBeginnings;
    std::vector<bool> bucketBeginningsFlags(result.size(), false);
    int currBucket = 0;

    bucketBeginningsFlags[0] = true;
    bucketBeginnings.push_back(0);
    buckets[0] = currBucket;
    for (size_t i = 1; i < result.size(); i++)
    {
        if (input[result[i]] != input[result[i - 1]])
        {
            currBucket++;
            bucketBeginnings.push_back(i);
            bucketBeginningsFlags[i] = true;
        }
        buckets[i] = currBucket;
    }

    for (size_t currLevel = 1; currLevel < levels.size(); currLevel++)
    {
        putItemToTheStartOfBucket(levels[currLevel][0] - currLevel, result, 
                                  buckets, positions, bucketBeginnings);
        for (size_t i = 1; i < levels[currLevel].size(); i++)
        {
            putItemToTheStartOfBucket(levels[currLevel][i] - currLevel, result,
                                      buckets, positions, bucketBeginnings);
        }

        currBucket++;
        int curr = positions[levels[currLevel][0] - currLevel];
        bucketBeginnings.push_back(curr);
        buckets[curr] = currBucket;
        for (size_t i = 1; i < levels[currLevel].size(); i++)
        {
            curr = positions[levels[currLevel][i] - currLevel];
            if (bucketBeginningsFlags[curr] || input[result[curr] + currLevel] != input[result[curr - 1] + currLevel])
            {
                currBucket++;
                bucketBeginnings.push_back(curr);
                buckets[curr] = currBucket;
                bucketBeginningsFlags[curr] = true;
            }
            else
            {
                buckets[curr] = buckets[curr - 1];
            }
        }
    }

    compressBuckets(buckets);
    std::vector<std::pair<int, int> > resultPairs;
    for (size_t i = 0; i < result.size(); i++)
    {
        resultPairs.push_back(std::make_pair(result[i], buckets[i]));
    }
    return resultPairs;
}

std::pair<std::vector<int>, std::vector<int> > createShorterString(std::vector<int> &input, 
                                                                   std::vector<SubstringMode> &slFlags)
{
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

void moveSuffixToTHeBeginOfTHeBucket(int currSuffix, std::vector<int> &result, std::vector<int> &positions,
                                     std::vector<int> &bucketBegins, std::vector<int> &buckets)
{
    int from = positions[currSuffix], to = bucketBegins[buckets[from]];
    std::swap(positions[currSuffix], positions[result[to]]);
    std::swap(result[to], result[from]);
    bucketBegins[buckets[from]]++;
}

void moveSuffixToTheEndOfTheBucket(int currSuffix, std::vector<int> &result, std::vector<int> &positions,
                                   std::vector<int> &bucketEnds, std::vector<int> &buckets)
{
    int from = positions[currSuffix], to = bucketEnds[buckets[from]];
    std::swap(positions[currSuffix], positions[result[to]]);
    std::swap(result[to], result[from]);
    bucketEnds[buckets[from]]--;
}

std::vector<int> sortSuffixesWithShortResult(std::vector<int> &input, std::vector<int> &shortResult,
                                             std::pair<std::vector<int>, std::vector<int> > &shortInput, 
                                             std::vector<SubstringMode> &slFlags)
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

    std::vector<std::vector<int> > sortedByFirstChar(ALPHABET_SIZE);
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
        for (int i = sortedModeSuffix.size() - 1; i >= 0; i--)
        {
            moveSuffixToTheEndOfTheBucket(sortedModeSuffix[i], result, positions, bucketEnds, buckets);
        }
        for (size_t i = 0; i < result.size(); i++)
        {
            if (result[i] != 0 && slFlags[result[i] - 1] == SubstringMode::L_MODE)
            {
                moveSuffixToTHeBeginOfTHeBucket(result[i] - 1, result, positions, bucketBegins, buckets);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < sortedModeSuffix.size(); i++)
        {
            moveSuffixToTHeBeginOfTHeBucket(sortedModeSuffix[i], result, positions, bucketBegins, buckets);
        }
        for (int i = result.size() - 1; i >= 0; i--)
        {
            if (result[i] != 0 && slFlags[result[i] - 1] == SubstringMode::S_MODE)
            {
                moveSuffixToTheEndOfTheBucket(result[i] - 1, result, positions, bucketEnds, buckets);
            }
        }
    }

    return result;
}

std::vector<int> createSuffixArrayFromIntArray(std::vector<int> &input)
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

void printSuffix(const std::string &input, int suffixBegin)
{
    std::string s = input + LAST_SYMBOL;
    for (size_t i = suffixBegin; i < s.length(); i++)
        out << s[i];
    out << std::endl;
}

int main()
{
    std::string input;
    in >> input;

    std::vector<int> sufArr = createSuffixArray(input);
    for (size_t i = 0; i < sufArr.size(); i++)
    {
        printSuffix(input, sufArr[i]);
    }

    return 0;
}
