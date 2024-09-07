#include <vector>
#include <algorithm>
#include <fstream>

// BTreeNode class
template<typename T>
class BTreeNode {
public:
    bool isLeaf;
    std::vector<T> keys;
    std::vector<BTreeNode*> children;

    BTreeNode(bool leaf) : isLeaf(leaf) {}

    // Insert a new key into the B-Tree node
    void insertNonFull(const T& key, int t);

    // Split the child node
    void splitChild(int i, BTreeNode* y, int t);

    // Search for a key in the B-Tree node
    BTreeNode* search(const T& key);

    // Remove a key from the B-Tree node
    void remove(const T& key, int t);

    // Find the predecessor of a key
    T getPredecessor(int idx);

    // Find the successor of a key
    T getSuccessor(int idx);

    // Fill the child node
    void fill(int idx, int t);

    // Borrow a key from the previous child
    void borrowFromPrev(int idx);

    // Borrow a key from the next child
    void borrowFromNext(int idx);

    // Merge the child nodes
    void merge(int idx);

    // Serialize the B-Tree node
    void serialize(std::ofstream& file) const;

    // Deserialize the B-Tree node
    void deserialize(std::ifstream& file);
};

// BTree class
template<typename T>
class BTree {
private:
    BTreeNode<T>* root;
    int t; // Minimum degree (defines the range for number of keys)

public:
    // Default constructor
    BTree() : root(nullptr), t(0) {}

    // Parameterized constructor
    BTree(int _t) : t(_t) {
        root = new BTreeNode<T>(true);
    }

    // Copy constructor
    BTree(const BTree& other) : t(other.t) {
        root = new BTreeNode<T>(*other.root);
    }

    // Copy assignment operator
    BTree& operator=(const BTree& other) {
        if (this == &other) {
            return *this;
        }
        t = other.t;
        delete root;
        root = new BTreeNode<T>(*other.root);
        return *this;
    }

    // Method to copy the contents of one BTree to another
    void copyTo(BTree& other) const {
        other.t = t;
        other.root = new BTreeNode<T>(*root);
    }

    // Insert a new key into the B-Tree
    void insert(const T& key);

    // Search for a key in the B-Tree
    BTreeNode<T>* search(const T& key);

    // Remove a key from the B-Tree
    void remove(const T& key);

    // Serialize the B-Tree
    void serialize(std::ofstream& file) const;

    // Deserialize the B-Tree
    void deserialize(std::ifstream& file);

    // Get the degree of the B-Tree
    int getDegree() const {
        return t;
    }
};

template<typename T>
void BTreeNode<T>::insertNonFull(const T& key, int t) {
    int i = keys.size() - 1;
    if (isLeaf) {
        keys.push_back(key);
        std::sort(keys.begin(), keys.end());
    }
    else {
        while (i >= 0 && key < keys[i]) {
            i--;
        }
        i++;
        if (children[i]->keys.size() == 2 * t - 1) {
            splitChild(i, children[i], t);
            if (key > keys[i]) {
                i++;
            }
        }
        children[i]->insertNonFull(key, t);
    }
}

template<typename T>
void BTreeNode<T>::splitChild(int i, BTreeNode* y, int t) {
    BTreeNode* z = new BTreeNode(y->isLeaf);
    for (int j = 0; j < t - 1; j++) {
        z->keys.push_back(y->keys[j + t]);
    }
    if (!y->isLeaf) {
        for (int j = 0; j < t; j++) {
            z->children.push_back(y->children[j + t]);
        }
    }
    y->keys.resize(t - 1);
    children.insert(children.begin() + i + 1, z);
    keys.insert(keys.begin() + i, y->keys[t - 1]);
}

template<typename T>
BTreeNode<T>* BTreeNode<T>::search(const T& key) {
    int i = 0;
    while (i < keys.size() && key > keys[i]) {
        i++;
    }
    if (i < keys.size() && keys[i] == key) {
        return this;
    }
    if (isLeaf) {
        return nullptr;
    }
    return children[i]->search(key);
}

template<typename T>
void BTree<T>::insert(const T& key) {
    if (root->keys.size() == 2 * t - 1) {
        BTreeNode<T>* s = new BTreeNode<T>(false);
        s->children.push_back(root);
        s->splitChild(0, root, t);
        root = s;
    }
    root->insertNonFull(key, t);
}

template<typename T>
BTreeNode<T>* BTree<T>::search(const T& key) {
    return root->search(key);
}

template<typename T>
void BTree<T>::remove(const T& key) {
    if (!root) {
        return;
    }
    root->remove(key, t);
    if (root->keys.size() == 0) {
        BTreeNode<T>* tmp = root;
        if (root->isLeaf) {
            root = nullptr;
        }
        else {
            root = root->children[0];
        }
        delete tmp;
    }
}

template<typename T>
void BTreeNode<T>::remove(const T& key, int t) {
    int idx = std::lower_bound(keys.begin(), keys.end(), key) - keys.begin();
    if (idx < keys.size() && keys[idx] == key) {
        if (isLeaf) {
            keys.erase(keys.begin() + idx);
        }
        else {
            if (children[idx]->keys.size() >= t) {
                T pred = getPredecessor(idx);
                keys[idx] = pred;
                children[idx]->remove(pred, t);
            }
            else if (children[idx + 1]->keys.size() >= t) {
                T succ = getSuccessor(idx);
                keys[idx] = succ;
                children[idx + 1]->remove(succ, t);
            }
            else {
                merge(idx);
                children[idx]->remove(key, t);
            }
        }
    }
    else {
        if (isLeaf) {
            return;
        }
        bool flag = (idx == keys.size());
        if (children[idx]->keys.size() < t) {
            fill(idx,t);
        }
        if (flag && idx > keys.size()) {
            children[idx - 1]->remove(key, t);
        }
        else {
            children[idx]->remove(key, t);
        }
    }
}

template<typename T>
T BTreeNode<T>::getPredecessor(int idx) {
    BTreeNode* cur = children[idx];
    while (!cur->isLeaf) {
        cur = cur->children[cur->keys.size()];
    }
    return cur->keys[cur->keys.size() - 1];
}

template<typename T>
T BTreeNode<T>::getSuccessor(int idx) {
    BTreeNode* cur = children[idx + 1];
    while (!cur->isLeaf) {
        cur = cur->children[0];
    }
    return cur->keys[0];
}

template<typename T>
void BTreeNode<T>::fill(int idx, int t) {
    if (idx != 0 && children[idx - 1]->keys.size() >= t) {
        borrowFromPrev(idx);
    }
    else if (idx != keys.size() && children[idx + 1]->keys.size() >= t) {
        borrowFromNext(idx);
    }
    else {
        if (idx != keys.size()) {
            merge(idx);
        }
        else {
            merge(idx - 1);
        }
    }
}

template<typename T>
void BTreeNode<T>::borrowFromPrev(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx - 1];
    child->keys.insert(child->keys.begin(), keys[idx - 1]);
    if (!child->isLeaf) {
        child->children.insert(child->children.begin(), sibling->children[sibling->keys.size()]);
    }
    keys[idx - 1] = sibling->keys[sibling->keys.size() - 1];
    sibling->keys.pop_back();
    if (!sibling->isLeaf) {
        sibling->children.pop_back();
    }
}

template<typename T>
void BTreeNode<T>::borrowFromNext(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx + 1];
    child->keys.push_back(keys[idx]);
    if (!child->isLeaf) {
        child->children.push_back(sibling->children[0]);
    }
    keys[idx] = sibling->keys[0];
    sibling->keys.erase(sibling->keys.begin());
    if (!sibling->isLeaf) {
        sibling->children.erase(sibling->children.begin());
    }
}

template<typename T>
void BTreeNode<T>::merge(int idx) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = children[idx + 1];
    child->keys.push_back(keys[idx]);
    for (int i = 0; i < sibling->keys.size(); ++i) {
        child->keys.push_back(sibling->keys[i]);
    }
    if (!child->isLeaf) {
        for (int i = 0; i < sibling->children.size(); ++i) {
            child->children.push_back(sibling->children[i]);
        }
    }
    keys.erase(keys.begin() + idx);
    children.erase(children.begin() + idx + 1);
    delete sibling;
}

template<typename T>
void BTreeNode<T>::serialize(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&isLeaf), sizeof(isLeaf));
    size_t keysSize = keys.size();
    file.write(reinterpret_cast<const char*>(&keysSize), sizeof(keysSize));
    for (const auto& key : keys) {
        file.write(reinterpret_cast<const char*>(&key), sizeof(key));
    }
    if (!isLeaf) {
        size_t childrenSize = children.size();
        file.write(reinterpret_cast<const char*>(&childrenSize), sizeof(childrenSize));
        for (const auto& child : children) {
            child->serialize(file);
        }
    }
}

template<typename T>
void BTreeNode<T>::deserialize(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&isLeaf), sizeof(isLeaf));
    size_t keysSize;
    file.read(reinterpret_cast<char*>(&keysSize), sizeof(keysSize));
    keys.resize(keysSize);
    for (auto& key : keys) {
        file.read(reinterpret_cast<char*>(&key), sizeof(key));
    }
    if (!isLeaf) {
        size_t childrenSize;
        file.read(reinterpret_cast<char*>(&childrenSize), sizeof(childrenSize));
        children.resize(childrenSize);
        for (auto& child : children) {
            child = new BTreeNode(isLeaf);
            child->deserialize(file);
        }
    }
}

template<typename T>
void BTree<T>::serialize(std::ofstream& file) const {
    root->serialize(file);
}

template<typename T>
void BTree<T>::deserialize(std::ifstream& file) {
    root->deserialize(file);
}
