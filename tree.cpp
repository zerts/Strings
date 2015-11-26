#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>

std::ifstream in("input.txt");
std::ofstream out("output.txt");

const size_t ALPHABET_SIZE = 27;
const char ALPHABET_START = 'a';
const char LAST_SYMBOL = '$';

size_t getIndexByChar(char curr)
{
    if (curr == LAST_SYMBOL)
        return 0;
    else
        return curr - ALPHABET_START + 1;
}

class Node
{
public:
    Node *parent, *suffix;
    std::vector<Node*> children;
    int start, finish, depth;

    Node(Node *p, int s, int f, int d)
    {
        parent = p;
        suffix = NULL;
        start = s;
        finish = f;
        depth = d;
        children.assign(ALPHABET_SIZE, NULL);
    }

    Node(){};

    int getEdgeLength()
    {
        return finish - start;
    }

    void addChild(Node *newChild, char ch)
    {
        children[getIndexByChar(ch)] = newChild;
    }
};

Node *addNodeToEdge(Node *currNode, Node *child, int edgePosition, const std::string &input)
{
    Node *middleNode = new Node(currNode, child->start, child->start + edgePosition,
        currNode->depth + edgePosition);
    child->start = middleNode->finish;
    child->parent = middleNode;
    currNode->addChild(middleNode, input[middleNode->start]);
    middleNode->addChild(child, input[middleNode->finish]);
    return middleNode;
}

Node *slowScan(Node *startNode, size_t start, const std::string &input)
{
    Node *currNode = startNode;
    int currPosition = start + startNode->depth;
    while (currNode->children[getIndexByChar(input[currPosition])] != NULL)
    {
        Node *child = currNode->children[getIndexByChar(input[currPosition])];
        int edgePosition = 0;
        while (edgePosition < child->getEdgeLength() && 
               input[child->start + edgePosition] == input[currPosition])
        {
            currPosition++;
            edgePosition++;
        }
        if (edgePosition == child->getEdgeLength())
        {
            currNode = child;
        }
        else
        {
            currNode = addNodeToEdge(currNode, child, edgePosition, input);
            break;
        }
    }
    return currNode;

}

Node *fastScan(Node *root, Node *startNode, const std::string &input)
{
    if (startNode == root)
    {
        return startNode;
    }
    if (startNode->suffix == NULL)
    {
        int skipped = startNode->getEdgeLength();
        int currPosition = startNode->start;
        if (startNode->parent == root)
        {
            skipped--;
            currPosition++;
        }
        Node *currNode = startNode->parent->suffix;
        while (skipped > 0)
        {
            currNode = currNode->children[getIndexByChar(input[currPosition])];
            skipped -= currNode->getEdgeLength();
            currPosition += currNode->getEdgeLength();
        }
        if (skipped < 0)
        {
            currNode = addNodeToEdge(currNode->parent, currNode, currNode->getEdgeLength() + skipped, input);
        }
        startNode->suffix = currNode;
    }
    return startNode->suffix;
}

Node *insertSuffix(Node *root, Node *head, size_t sufStart, const std::string &input)
{
    Node *newHead = slowScan(fastScan(root, head, input), sufStart, input);
    Node *newLeaf = new Node(newHead, sufStart + newHead->depth, input.length(), input.length());
    newHead->addChild(newLeaf, input[newLeaf->start]);
    return newHead;
}

Node *createSuffixTree(const std::string &input)
{
    std::string s = input + LAST_SYMBOL;
    Node *root = new Node(NULL, 0, 0, 0);
    root->suffix = root;
    Node *tree = new Node(root, 0, 0, 0);
    tree->suffix = root;
    root->addChild(tree, LAST_SYMBOL);
    for (size_t i = 0; i < ALPHABET_SIZE - 1; i++)
        root->addChild(tree, ALPHABET_START + i);
    Node *head = tree;
    for (size_t i = 0; i < s.length(); i++)
        head = insertSuffix(tree, head, i, s);
    return tree;
}

void dfs(Node *curr, long long &ans)
{
    ans += 1ll * (curr->getEdgeLength());
    for (size_t i = 0; i < ALPHABET_SIZE; i++)
    {
        if (curr->children[i] != NULL)
            dfs(curr->children[i], ans);
    }
}

int main()
{
    std::string input;
    in >> input;
    //std::cin >> input;
    Node *tree = createSuffixTree(input);
    long long ans = 0;
    dfs(tree, ans);
    out << ans - input.length() - 1;
    //std::cout << ans - input.length() - 1;
    in.close();
    out.close();
    return 0;
}
