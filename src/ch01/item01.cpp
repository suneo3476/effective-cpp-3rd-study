// Item 1: View C++ as a federation of languages
// C++を言語の連合体と見なす
//
// C++は以下の4つのサブ言語から成る：
// 1. C - ブロック、文、プリプロセッサ、組み込み型、配列、ポインタなど
// 2. Object-Oriented C++ - クラス、カプセル化、継承、ポリモーフィズム
// 3. Template C++ - ジェネリックプログラミング、テンプレートメタプログラミング
// 4. STL - コンテナ、イテレータ、アルゴリズム、関数オブジェクト

#include <iostream>
#include <vector>
#include <algorithm>

// 1. C style
void c_style_example() {
    int arr[] = {1, 2, 3, 4, 5};
    int sum = 0;
    for (int i = 0; i < 5; ++i) {
        sum += arr[i];
    }
    std::cout << "C style sum: " << sum << std::endl;
}

// 2. Object-Oriented C++
class Shape {
public:
    virtual ~Shape() = default;
    virtual void draw() const = 0;
};

class Circle : public Shape {
public:
    void draw() const override {
        std::cout << "Drawing a circle" << std::endl;
    }
};

// 3. Template C++
template<typename T>
T add(T a, T b) {
    return a + b;
}

// 4. STL
void stl_example() {
    std::vector<int> v = {5, 2, 8, 1, 9};
    std::sort(v.begin(), v.end());
    std::cout << "STL sorted: ";
    for (int x : v) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Item 1: C++ as a federation of languages ===" << std::endl;
    std::cout << std::endl;

    std::cout << "[1] C style:" << std::endl;
    c_style_example();
    std::cout << std::endl;

    std::cout << "[2] Object-Oriented C++:" << std::endl;
    Circle c;
    c.draw();
    std::cout << std::endl;

    std::cout << "[3] Template C++:" << std::endl;
    std::cout << "add(3, 4) = " << add(3, 4) << std::endl;
    std::cout << "add(3.5, 4.5) = " << add(3.5, 4.5) << std::endl;
    std::cout << std::endl;

    std::cout << "[4] STL:" << std::endl;
    stl_example();

    return 0;
}
