#include <iostream>
#include <string>
#include <vector>

struct Node {
  Node* parent = nullptr;
  Node* left = nullptr;
  Node* right = nullptr;
  long long key = 0;
  long long countleft = 0;
  long long countright = 0;
  Node(long long key) : key(key) {}
};
const long long cInt = 1000000001;
struct SplayTree {
 private:
  std::vector<Node*> a_;
  long long count_ = 0;
  Node* vertex_ = nullptr;

  static long long Count(Node* p) {
    if (p == nullptr) {
      return 0;
    }
    return p->countleft + p->countright;
  }

  static void ZigCountLeft(Node* p) {
    Node* q = p->right;
    if (q->left != nullptr) {
      q->countleft = Count(q->left);
    } else {
      q->countleft = 0;
    }
    if (p->right != nullptr) {
      p->countright = Count(p->right) + 1;
    } else {
      p->countright = 1;
    }
  }

  static void ZigCountRight(Node* p) {
    Node* q = p->left;
    if (q->right != nullptr) {
      q->countright = Count(q->right) + 1;
    } else {
      q->countright = 1;
    }
    if (p->left != nullptr) {
      p->countleft = Count(p->left);
    } else {
      p->countleft = 0;
    }
  }

  static void ZigZigCountLeft(Node* p) {
    Node* b = p->right;
    Node* c = b->right;
    if (c->left != nullptr) {
      c->countleft = Count(c->left);
    } else {
      c->countleft = 0;
    }
    if (b->left != nullptr) {
      b->countleft = Count(b->left);
    } else {
      b->countleft = 0;
    }
    if (b->right != nullptr) {
      b->countright = Count(b->right) + 1;
    } else {
      b->countright = 1;
    }
    if (p->right != nullptr) {
      p->countright = Count(p->right) + 1;
    } else {
      p->countright = 1;
    }
  }

  static void ZigZigCountRight(Node* p) {
    Node* b = p->left;
    Node* c = b->left;
    if (c->right != nullptr) {
      c->countright = Count(c->right) + 1;
    } else {
      c->countright = 1;
    }
    if (b->left != nullptr) {
      b->countleft = Count(b->left);
    } else {
      b->countleft = 0;
    }
    if (b->right != nullptr) {
      b->countright = Count(b->right) + 1;
    } else {
      b->countright = 1;
    }
    if (p->left != nullptr) {
      p->countleft = Count(p->left);
    } else {
      p->countleft = 0;
    }
  }

  static void ZigZagCountLeft(Node* p) {
    Node* b = p->right;
    Node* c = p->left;
    if (c->right != nullptr) {
      c->countright = Count(c->right) + 1;
    } else {
      c->countright = 1;
    }
    if (b->left != nullptr) {
      b->countleft = Count(b->left);
    } else {
      b->countleft = 0;
    }
    if (p->left != nullptr) {
      p->countleft = Count(p->left);
    } else {
      p->countleft = 0;
    }
    if (p->right != nullptr) {
      p->countright = Count(p->right) + 1;
    } else {
      p->countright = 1;
    }
  }

  static void ZigZagCountRight(Node* p) {
    Node* b = p->left;
    Node* c = p->right;
    if (c->left != nullptr) {
      c->countleft = Count(c->left);
    } else {
      c->countleft = 0;
    }
    if (b->right != nullptr) {
      b->countright = Count(b->right) + 1;
    } else {
      b->countright = 1;
    }
    if (p->left != nullptr) {
      p->countleft = Count(p->left);
    } else {
      p->countleft = 0;
    }
    if (p->right != nullptr) {
      p->countright = Count(p->right) + 1;
    } else {
      p->countright = 1;
    }
  }

  void Splay(Node* p) {
    while (p != vertex_) {
      if (p != nullptr && p->parent != nullptr &&
          p->parent->parent != nullptr) {
        if ((p->parent->parent->left == p->parent && p->parent->right == p) ||
            (p->parent->parent->right == p->parent && p->parent->left == p)) {
          ZigZag(p);
        } else {
          ZigZig(p);
        }
      } else {
        Zig(p);
      }
      if (p == nullptr) {
        vertex_ = p;
      }
    }
  }

  void Zig(Node* p) {
    if (p->parent == nullptr) {
      vertex_ = p;
      return;
    }
    if (p->parent->left == p) {
      Node* q = p->parent;
      q->left = p->right;
      if (q->left != nullptr) {
        q->left->parent = q;
      }
      p->parent = q->parent;
      p->right = q;
      q->parent = p;
      if (p->parent == nullptr) {
        vertex_ = p;
      } else {
        if (p->parent->left == q) {
          p->parent->left = p;
        } else if (p->parent->right == q) {
          p->parent->right = p;
        }
      }
      ZigCountLeft(p);
    } else if (p->parent->right == p) {
      Node* q = p->parent;
      q->right = p->left;
      if (q->right != nullptr) {
        q->right->parent = q;
      }
      p->parent = q->parent;
      p->left = q;
      q->parent = p;
      if (p->parent == nullptr) {
        vertex_ = p;
      } else {
        if (p->parent->right == q) {
          p->parent->right = p;
        } else if (p->parent->left == q) {
          p->parent->left = p;
        }
      }
      ZigCountRight(p);
    }
  }

  void ZigZig(Node* p) {
    if (p->parent != nullptr && p->parent->parent != nullptr) {
      if (p == p->parent->left && p->parent == p->parent->parent->left) {
        Node* b = p->parent;
        Node* c = b->parent;
        b->left = p->right;
        if (b->left != nullptr) {
          b->left->parent = b;
        }
        p->right = b;
        p->parent = c->parent;
        b->parent = p;
        c->parent = b;
        c->left = b->right;
        if (c->left != nullptr) {
          c->left->parent = c;
        }
        b->right = c;
        if (p->parent == nullptr) {
          vertex_ = p;
        } else {
          if (p->parent->left == c) {
            p->parent->left = p;
          } else if (p->parent->right == c) {
            p->parent->right = p;
          }
        }
        ZigZigCountLeft(p);
      } else if (p == p->parent->right &&
                 p->parent == p->parent->parent->right) {
        Node* b = p->parent;
        Node* c = b->parent;
        b->right = p->left;
        if (b->right != nullptr) {
          b->right->parent = b;
        }
        p->left = b;
        p->parent = c->parent;
        b->parent = p;
        c->parent = b;
        c->right = b->left;
        if (c->right != nullptr) {
          c->right->parent = c;
        }
        b->left = c;
        if (p->parent == nullptr) {
          vertex_ = p;
        } else {
          if (p->parent->right == c) {
            p->parent->right = p;
          } else if (p->parent->left == c) {
            p->parent->left = p;
          }
        }
        ZigZigCountRight(p);
      }
    }
  }

  void ZigZag(Node* p) {
    if (p->parent != nullptr && p->parent->parent != nullptr) {
      if (p == p->parent->left && p->parent == p->parent->parent->right) {
        Node* b = p->parent;
        Node* c = b->parent;
        p->parent = c->parent;
        b->parent = p;
        c->parent = p;
        b->left = p->right;
        c->right = p->left;
        if (b->left != nullptr) {
          b->left->parent = b;
        }
        if (c->right != nullptr) {
          c->right->parent = c;
        }
        p->right = b;
        p->left = c;
        if (p->parent == nullptr) {
          vertex_ = p;
        } else {
          if (p->parent->left == c) {
            p->parent->left = p;
          } else if (p->parent->right == c) {
            p->parent->right = p;
          }
        }
        ZigZagCountLeft(p);
      } else if (p == p->parent->right &&
                 p->parent == p->parent->parent->left) {
        Node* b = p->parent;
        Node* c = b->parent;
        p->parent = c->parent;
        b->parent = p;
        c->parent = p;
        b->right = p->left;
        c->left = p->right;
        if (b->right != nullptr) {
          b->right->parent = b;
        }
        if (c->left != nullptr) {
          c->left->parent = c;
        }
        p->left = b;
        p->right = c;
        if (p->parent == nullptr) {
          vertex_ = p;
        } else {
          if (p->parent->left == c) {
            p->parent->left = p;
          } else if (p->parent->right == c) {
            p->parent->right = p;
          }
        }
        ZigZagCountRight(p);
      }
    }
  }

  void Add(Node* p, long long x, Node* prev) {
    if (vertex_ == nullptr) {
      Node* t = new Node(x);
      a_.push_back(t);
      vertex_ = t;
      ++count_;
      t->countright = 1;
      return;
    }
    if (p == nullptr) {
      Node* t = new Node(x);
      a_.push_back(t);
      t->countright = 1;
      if (prev != nullptr) {
        t->parent = prev;
        if (prev->key > x) {
          prev->left = t;
        } else if (prev->key < x) {
          prev->right = t;
        }
      }
      ++count_;
      Splay(t);
      return;
    }
    if (p->key == x) {
      Splay(p);
      return;
    }
    if (p->key > x) {
      Add(p->left, x, p);
    }
    if (p->key < x) {
      Add(p->right, x, p);
    }
  }

  void Merge(Node* p, Node* q) {
    Node* m = q;
    Node* prev = nullptr;
    if (q == nullptr && p == nullptr) {
      vertex_ = nullptr;
      return;
    }
    if (q == nullptr && p != nullptr) {
      vertex_ = p;
      p->parent = nullptr;
      if (p->left != nullptr) {
        p->countleft = Count(p->left);
      } else {
        p->countleft = 0;
      }
      if (p->right != nullptr) {
        p->countright = Count(p->right) + 1;
      } else {
        p->countright = 1;
      }
      return;
    }
    if (q != nullptr && p == nullptr) {
      vertex_ = q;
      q->parent = nullptr;
      if (q->left != nullptr) {
        q->countleft = Count(q->left);
      } else {
        q->countleft = 0;
      }
      if (q->right != nullptr) {
        q->countright = Count(q->right) + 1;
      } else {
        q->countright = 1;
      }
      return;
    }
    q->parent = nullptr;
    while (true) {
      if (m == nullptr) {
        m = prev;
        break;
      }
      prev = m;
      m = m->left;
    }
    m->left = p;
    p->parent = m;
    if (m->parent == nullptr) {
      if (m->left != nullptr) {
        m->countleft = Count(m->left);
      } else {
        m->countleft = 0;
      }
      if (m->right != nullptr) {
        m->countright = Count(m->right) + 1;
      } else {
        m->countright = 1;
      }
      vertex_ = m;
      return;
    }
    Splay(m);
    if (m->left != nullptr) {
      m->countleft = Count(m->left);
    } else {
      m->countleft = 0;
    }
    if (m->right != nullptr) {
      m->countright = Count(m->right) + 1;
    } else {
      m->countright = 1;
    }
    vertex_ = m;
  }

 public:
  void Insert(long long x) { Add(vertex_, x, nullptr); }

  void Delete(long long x) {
    Node* p = vertex_;
    Node* prev = nullptr;
    while (true) {
      if (p == nullptr) {
        if (prev != nullptr) {
          Splay(prev);
        }
        return;
      }
      if (p->key == x) {
        Splay(p);
        Merge(p->left, p->right);
        --count_;
        return;
      }
      if (p->key > x) {
        prev = p;
        p = p->left;
        continue;
      }
      if (p->key < x) {
        prev = p;
        p = p->right;
        continue;
      }
    }
  }

  bool Exists(long long x) {
    Node* p = vertex_;
    Node* prev = nullptr;
    while (true) {
      if (p == nullptr) {
        if (prev != nullptr) {
          Splay(prev);
        }
        return false;
      }
      if (p->key == x) {
        Splay(p);
        return true;
      }
      if (p->key > x) {
        prev = p;
        p = p->left;
        continue;
      }
      if (p->key < x) {
        prev = p;
        p = p->right;
        continue;
      }
    }
  }

  long long Next(long long x) {
    Node* inter = vertex_;
    Node* p = nullptr;
    long long ans = cInt;
    while (true) {
      if (inter == nullptr) {
        if (ans == cInt) {
          return ans;
        }
        Splay(p);
        return ans;
      }
      if (inter->key > x) {
        if (inter->left != nullptr) {
          if (inter->key < ans) {
            ans = inter->key;
            p = inter;
          }
          inter = inter->left;
          continue;
        }
        if (inter->right != nullptr) {
          if (inter->key < ans) {
            ans = inter->key;
            p = inter;
          }
          inter = inter->right;
          continue;
        }
        if (inter->key < ans) {
          ans = inter->key;
          p = inter;
        }
        if (ans == cInt) {
          return ans;
        }
        Splay(p);
        return ans;
      }
      if (inter->key <= x) {
        if (inter->right != nullptr) {
          inter = inter->right;
          continue;
        }
        if (ans == cInt) {
          return ans;
        }
        Splay(p);
        return ans;
      }
    }
  }

  long long Prev(long long x) {
    Node* inter = vertex_;
    Node* p = nullptr;
    long long ans = -cInt;
    while (true) {
      if (inter == nullptr) {
        if (ans == -cInt) {
          return ans;
        }
        Splay(p);
        return ans;
      }
      if (inter->key >= x) {
        if (inter->left != nullptr) {
          inter = inter->left;
          continue;
        }
        if (ans == -cInt) {
          return ans;
        }
        Splay(p);
        return ans;
      }
      if (inter->key < x) {
        if (inter->right != nullptr) {
          if (inter->key > ans) {
            ans = inter->key;
            p = inter;
          }
          inter = inter->right;
          continue;
        }
        if (inter->left != nullptr) {
          if (inter->key > ans) {
            ans = inter->key;
            p = inter;
          }
          inter = inter->left;
          continue;
        }
        if (ans == -cInt) {
          return ans;
        }
        Splay(p);
        return ans;
      }
    }
  }

  long long Kth(long long k) {
    Node* p = vertex_;
    Node* prev = nullptr;
    while (true) {
      if (p == nullptr) {
        if (prev != nullptr) {
          Splay(prev);
        }
        return cInt;
      }
      if (k == p->countleft) {
        Splay(p);
        return p->key;
      }
      if (p->countleft > k) {
        prev = p;
        p = p->left;
        continue;
      }
      if (p->countleft < k) {
        prev = p;
        k = k - p->countleft - 1;
        p = p->right;
        continue;
      }
    }
  }

  ~SplayTree() {
    for (auto& i : a_) {
      delete i;
    }
  }
};

int main() {
  std::string s;
  SplayTree tree;
  long long ans;
  long long x;
  while (std::cin >> s) {
    std::cin >> x;
    if (s == "insert") {
      tree.Insert(x);
    } else if (s == "delete") {
      tree.Delete(x);
    } else if (s == "exists") {
      if (tree.Exists(x)) {
        std::cout << "true\n";
      } else {
        std::cout << "false\n";
      }
    } else if (s == "next") {
      ans = tree.Next(x);
      if (ans == cInt) {
        std::cout << "none\n";
      } else {
        std::cout << ans << '\n';
      }
    } else if (s == "prev") {
      ans = tree.Prev(x);
      if (ans == cInt) {
        std::cout << "none\n";
      } else {
        std::cout << ans << '\n';
      }
    } else if (s == "kth") {
      ans = tree.Kth(x);
      if (ans == cInt) {
        std::cout << "none\n";
      } else {
        std::cout << ans << '\n';
      }
    }
  }
}
