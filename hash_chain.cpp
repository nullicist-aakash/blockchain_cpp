#include <iostream>
#include <vector>

using namespace std;

class SegmentTree
{
    vector<int> tree;
    const int n;

    // return the computed value for the nodes
    static int compute(int a, int b)
    {
        return a + b;
    }

public:
    explicit SegmentTree(vector<int> &arr) : n { (int)arr.size() }, tree{ }
    {
        // Fill Leaf nodes
        tree.resize(2 * n - 1);
        for (int i = 0; i < n; ++i)
            tree[i + arr.size()] = arr[i];

        // Fill internal nodes
        for (auto i = n - 1; i > 0; --i)
            tree[i] = compute(tree[i << 1], tree[i << 1 | 1]);
    }

    // set value at position p
    void modify(int p, int value)
    {
        for (tree[p += n] = value; p > 1; p >>= 1)
            tree[p >> 1] = compute(tree[p], tree[p ^ 1]);
    }

    // sum on interval [l, r)
    int query(int l, int r)
    {
        int res = 0;

        // go up the tree till left and right computations meet
        for (l += n, r += n; l < r; l >>= 1, r >>= 1)
        {
            if (l & 1) res = compute(res, tree[l++]);
            if (r & 1) res = compute(res, tree[--r]);
        }

        // return the result
        return res;
    }
};


int main2()
{
    vector<int> arr {5, -3, 6, 1, 0, -4, 11, 6, 2, 7};
    SegmentTree st(arr);

    // Compute the answer for [2, 6]. This means we compute the answer for [2, 7)
    cout << "Sum for range [2, 6] is: " << st.query(2, 7) << endl;

    // Compute the answer for [0, 3]. This means we compute the answer for [0, 4)
    cout << "Sum for range [0, 3] is: " << st.query(0, 4) << endl;

    // Change the element at index 4 to 36
    cout << "Updating the value at index 4 to 36" << endl;
    st.modify(4, 36);

    // Compute the answer for [1, 6]. This means we compute the answer for [1, 7)
    cout << "Sum for range [1, 6] is: " << st.query(1, 7) << endl;
}