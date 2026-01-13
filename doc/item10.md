# Item 10: 代入演算子は *this への参照を返せ

## 代入の連鎖

こんなコードを見たことありますか？

```cpp
int x, y, z;
x = y = z = 15;  // 代入の連鎖
```

これは1行で複数の代入を行っています。

## 右結合（Right-Associative）

代入演算子は**右結合**なので、上のコードはこう解釈されます：

```cpp
x = (y = (z = 15));
```

実行順序：
```
1. z = 15    → zに15を代入
2. y = (z)   → yに(1の結果)を代入
3. x = (y)   → xに(2の結果)を代入
```

## 代入式の戻り値

**`z = 15` という式は、「更新されたz」への参照を返します**

つまり：
```cpp
z = 15        // zに15を代入して、zへの参照を返す
y = (z)       // 上の結果（z）をyに代入して、yへの参照を返す
x = (y)       // 上の結果（y）をxに代入して、xへの参照を返す
```

「15という値」ではなく、**「15が代入されたz」そのものへの参照**が返されます。

## C++の標準型の代入演算子

組み込み型（int, doubleなど）の代入演算子は、**左辺への参照**を返します：

```cpp
int& operator=(int& left, int right) {  // 概念的には
    left = right;
    return left;  // 左辺への参照を返す
}
```

## this と *this

### this とは

**`this` = 現在のオブジェクトへのポインタ**

```cpp
class Widget {
public:
    void func() {
        // this のタイプは Widget*（Widgetへのポインタ）
    }
};
```

`this`は、メンバ関数の中で「このオブジェクト自身」を指すポインタです。

### *this とは

**`*this` = ポインタをデリファレンス（間接参照）して、オブジェクトの実体にアクセス**

```cpp
class Widget {
public:
    Widget& operator=(const Widget& rhs) {
        // this    → Widget* 型（ポインタ）
        // *this   → Widget& 型（オブジェクトの実体への参照）
        return *this;
    }
};
```

例えで言うと：
- `this` = 「家の住所」（ポインタ）
- `*this` = 「その住所にある家そのもの」（実体）

## 自作クラスの代入演算子

```cpp
class Widget {
public:
    Widget& operator=(const Widget& rhs) {  // 戻り値の型は Widget& への参照
        // ...
        return *this;  // 左辺のオブジェクトを返す
    }
};
```

完全な例：

```cpp
class Widget {
public:
    int value;

    Widget& operator=(const Widget& rhs) {
        // w1 = w2 の場合：
        // this == &w1  （左辺のアドレス）
        // rhs == w2    （右辺への参照）

        this->value = rhs.value;  // w2の内容をw1にコピー

        return *this;  // w1への参照を返す
    }
};

Widget w1, w2;
w1 = w2;  // w1.operator=(w2) が呼ばれる
```

### 左辺と右辺

```cpp
Widget w1, w2;
w1 = w2;  // w1.operator=(w2) が呼ばれる
```

**代入演算子は、左辺のオブジェクトのメンバ関数として呼ばれます**

これは実際には：
```cpp
w1.operator=(w2);
```

つまり：
- メンバ関数を呼び出すのは **`w1`**（左辺）
- `this` は、メンバ関数を呼び出したオブジェクトを指す
- だから **`this == &w1`**（w1のアドレス）
- 引数の `rhs` が `w2` への参照

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    // w1 = w2 の場合：
    // this == &w1  （左辺のアドレス）
    // rhs == w2    （右辺への参照）

    // w2 の内容を *this（w1）にコピー

    return *this;  // w1 への参照を返す
}
```

- **左辺（w1）** = `*this`（自分自身）
- **右辺（w2）** = `rhs`（引数）

## this は読み取り専用

`this`ポインタは、C++の言語仕様として**読み取り専用**です。書き換えることはできません。

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    // this は暗黙的に Widget* const 型（定数ポインタ）
    // つまり、this の値（アドレス）は変更できない

    this = &rhs;  // コンパイルエラー！this は変更できない

    return *this;
}
```

`this`は、概念的には以下のように**定数ポインタ**として扱われます：

```cpp
Widget* const this;  // this 自体は変更できない（定数ポインタ）
```

### 変わるのは「内容」

代入演算子が変えるのは、**w1の内容（メンバ変数の値）**であって、**w1のアドレス**ではありません：

```cpp
class Widget {
public:
    int value;

    Widget& operator=(const Widget& rhs) {
        // this のアドレスは変わらない
        // でも *this の内容（value）は変わる
        this->value = rhs.value;  // w1.value = w2.value
        return *this;
    }
};

Widget w1, w2;
w1.value = 10;
w2.value = 20;

std::cout << &w1 << std::endl;  // 例: 0x1000
w1 = w2;                         // w1.value が 20 になる
std::cout << &w1 << std::endl;  // 例: 0x1000 （変わらない！）
std::cout << w1.value << std::endl;  // 20（内容は変わった）
```

`this`は、**メンバ関数を呼び出したオブジェクトのアドレス**を保持し、その値は絶対に変わりません。

## 他の代入演算子も同じ

この規約は、`=` だけでなく、**すべての代入演算子**に適用されます：

```cpp
class Widget {
public:
    Widget& operator=(const Widget& rhs) {  // =
        // ...
        return *this;
    }

    Widget& operator+=(const Widget& rhs) {  // +=
        // ...
        return *this;
    }

    Widget& operator-=(const Widget& rhs) {  // -=
        // ...
        return *this;
    }

    Widget& operator*=(const Widget& rhs) {  // *=
        // ...
        return *this;
    }

    Widget& operator=(int rhs) {  // パラメータの型が違っても
        // ...
        return *this;
    }
};
```

これで連鎖代入ができます：

```cpp
Widget w1, w2, w3;
w1 = w2 = w3;      // OK
w1 += w2 += w3;    // OK
```

## これは規約（Convention）

重要なポイント：

> これは**規約**であり、従わなくてもコンパイルは通る。しかし、標準ライブラリのすべての型（string, vector, complex, shared_ptr など）がこの規約に従っている。特別な理由がない限り、従うべき。

従わない例：

```cpp
class Widget {
public:
    void operator=(const Widget& rhs) {  // void を返す（規約違反）
        // ...
        // return文なし
    }
};

Widget w1, w2, w3;
w1 = w2;      // OK（単独の代入は動く）
w1 = w2 = w3; // エラー！連鎖代入ができない
```

## 覚えておくこと

> 代入演算子は `*this` への参照を返せ。

## Q&A サマリ

1. **代入の連鎖とは？** → `x = y = z = 15` のように、複数の代入を1行で書くこと。

2. **代入は右結合とは？** → `x = y = z = 15` は `x = (y = (z = 15))` と解釈される。右から順に評価される。

3. **代入式の戻り値は？** → 左辺への参照。`z = 15` は「15が代入されたz」への参照を返す。

4. **`this` とは？** → 現在のオブジェクトへのポインタ（`Widget*` 型）。

5. **`*this` とは？** → `this` ポインタをデリファレンスして、オブジェクトの実体にアクセスする（`Widget&` 型）。

6. **`w1 = w2` で `this` は何を指す？** → `&w1`（左辺のアドレス）。`w1.operator=(w2)` と呼ばれるため。

7. **`this` は変更できる？** → いいえ。`this` は定数ポインタ（`Widget* const`）で、読み取り専用。

8. **代入で変わるのは？** → オブジェクトの内容（メンバ変数の値）。アドレスは変わらない。

9. **他の代入演算子も同じ？** → はい。`+=`, `-=`, `*=` などすべての代入演算子で `*this` への参照を返すべき。

10. **従わないとどうなる？** → コンパイルは通るが、連鎖代入ができなくなる。標準ライブラリとの一貫性も失われる。

## 参考文献

- Scott Meyers『Effective C++ 第3版』項目10
