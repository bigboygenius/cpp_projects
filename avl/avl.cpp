#include <iostream>
#include <string>
#include <vector>

struct Node {
  Node* parent = nullptr;
  Node* left = nullptr;
  Node* right = nullptr;
  long long key = 0;
  long long height = 1;
  Node(long long k) : key(k) {}
};

const long long cInt = 1000000000;

struct AVL {
 private:
  Node* vertex_ = nullptr;
  std::vector<Node*> a_;

  static long long Height(Node* inter) {
    if (inter == nullptr) {
      return 0;
    }
    return inter->height;
  }

  static long long Difference(Node* inter) {
    if (inter == nullptr) {
      return 0;
    }
    return Height(inter->right) - Height(inter->left);
  }

  static void Fix(Node* inter) {
    long long hl = Height(inter->left);
    long long hr = Height(inter->right);
    inter->height = (hl > hr ? hl : hr) + 1;
  }

  Node* RotateRight(Node* p) {
    Node* q = p->left;
    if (q != nullptr) {
      p->left = q->right;
      q->right = p;
      q->parent = p->parent;
      p->parent = q;
      Fix(p);
      Fix(q);
      if (q->parent != nullptr) {
        if (q->parent->left == p) {
          q->parent->left = q;
        } else if (q->parent->right == p) {
          q->parent->right = q;
        }
        Fix(q->parent);
      }
      if (q->parent == nullptr) {
        vertex_ = q;
      }
      return q;
    }
    return q;
  }

  Node* RotateLeft(Node* q) {
    Node* p = q->right;
    if (p != nullptr) {
      q->right = p->left;
      p->left = q;
      p->parent = q->parent;
      q->parent = p;
      Fix(q);
      Fix(p);
      if (p->parent != nullptr) {
        if (p->parent->right == q) {
          p->parent->right = p;
        } else if (p->parent->left == q) {
          p->parent->left = p;
        }
        Fix(p->parent);
      }
      if (p->parent == nullptr) {
        vertex_ = p;
      }
      return p;
    }
    return p;
  }

  Node* Balance(Node* p) {
    Fix(p);
    if (Difference(p) == 2) {
      if (Difference(p->right) < 0) {
        p->right = RotateRight(p->right);
      }
      return RotateLeft(p);
    }
    if (Difference(p) == -2) {
      if (Difference(p->left) > 0) {
        p->left = RotateLeft(p->left);
      }
      return RotateRight(p);
    }
    return p;
  }

  Node* Add(Node* p, long long x, Node* prev) {
    if (p == nullptr) {
      Node* t = new Node(x);
      a_.push_back(t);
      if (prev != nullptr) {
        t->parent = prev;
      }
      return t;
    }
    if (x == p->key) {
      return p;
    }
    if (x < p->key) {
      p->left = Add(p->left, x, p);
    } else {
      p->right = Add(p->right, x, p);
    }
    return Balance(p);
  }

 public:
  void Insert(long long x) { vertex_ = Add(vertex_, x, nullptr); }

  long long LowerBound(long long x) {
    Node* inter = vertex_;
    long long ans = cInt;
    if (inter == nullptr) {
      return -1;
    }
    while (true) {
      if (inter->key == x) {
        return x;
      }
      if (inter->key > x) {
        if (inter->left != nullptr) {
          ans = (ans > inter->key) ? inter->key : ans;
          inter = inter->left;
          continue;
        }
        if (inter->right != nullptr) {
          ans = (ans > inter->key) ? inter->key : ans;
          inter = inter->right;
          continue;
        }
        ans = (ans > inter->key) ? inter->key : ans;
        return ans;
      }
      if (inter->key < x) {
        if (inter->right != nullptr) {
          inter = inter->right;
          continue;
        }
        if (ans == cInt) {
          return -1;
        }
        return ans;
      }
    }
  }

  ~AVL() {
    for (auto& i : a_) {
      delete i;
    }
  }
};

int main() {
  int q;
  std::cin >> q;
  long long x;
  long long ans = 0;
  AVL tree;
  std::string s;
  std::string s1;
  bool flag = false;
  std::getline(std::cin, s);
  for (int i = 0; i < q; ++i) {
    std::cin >> s;
    if (s == "+") {
      std::cin >> x;
      if (s1 == "?" and flag) {
        tree.Insert((x + ans) % cInt);
      } else {
        tree.Insert(x);
      }
      flag = false;
    } else if (s == "?") {
      std::cin >> x;
      ans = tree.LowerBound(x);
      std::cout << ans << '\n';
      if (ans != -1) {
        flag = true;
      }
    }
    s1 = s;
  }
}
