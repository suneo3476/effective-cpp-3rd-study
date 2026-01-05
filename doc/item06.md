# Item 6: コンパイラ生成関数を明示的に禁止せよ

## 核心
コピーさせたくないクラスがある。でも何もしないとコンパイラが勝手にコピー関数を生成する。

## 問題：コピーさせたくないクラス

```cpp
class HomeForSale {  // 売り出し中の家
    // ...
};

HomeForSale h1;
HomeForSale h2;

HomeForSale h3(h1);  // コピー → させたくない！
h1 = h2;             // 代入 → させたくない！
```

不動産は唯一無二。コピーに意味がない。

## 困ったこと

Item 5 で見た通り、何も書かないとコンパイラがコピーコンストラクタとコピー代入演算子を自動生成する。

- 宣言しない → コンパイラが生成 → コピーできてしまう
- 宣言する → コピーできてしまう

## 解決策1：private に宣言して定義しない

```cpp
class HomeForSale {
public:
    // ...
private:
    // ...
    HomeForSale(const HomeForSale&);             // 宣言のみ、定義なし
    HomeForSale& operator=(const HomeForSale&);  // 宣言のみ、定義なし
};
```

### 外部から呼んだ場合

```cpp
HomeForSale h1;
HomeForSale h2(h1);  // コンパイルエラー！private だからアクセスできない
```

**コンパイル時**にエラー。

### メンバ関数や friend から呼んだ場合

```cpp
class HomeForSale {
public:
    void doSomething(const HomeForSale& other) {
        *this = other;  // private でもメンバ関数からは呼べる
    }
private:
    HomeForSale& operator=(const HomeForSale&);  // 定義なし
};
```

コンパイルは通る。でも定義がないから**リンク時**にエラー。

## 解決策2：基底クラスを使う（より良い）

リンクエラーよりコンパイルエラーの方が早く気づける。

```cpp
class Uncopyable {
protected:
    Uncopyable() {}
    ~Uncopyable() {}
private:
    Uncopyable(const Uncopyable&);
    Uncopyable& operator=(const Uncopyable&);
};

class HomeForSale : private Uncopyable {
    // コピー関連の宣言は不要
};
```

HomeForSale をコピーしようとすると、基底クラス Uncopyable のコピーが必要になる。でも private だからコンパイルエラー。

## 解決策3：C++11 以降は = delete（推奨）

```cpp
class HomeForSale {
public:
    HomeForSale(const HomeForSale&) = delete;
    HomeForSale& operator=(const HomeForSale&) = delete;
};
```

`= delete` で明示的に禁止。これが今の標準的なやり方。

### 感想
`= delete` は楽！意図も明確で分かりやすい。

## 覚えておくこと

> コンパイラが自動生成する機能を禁止するには、対応するメンバ関数を private に宣言して定義を与えない。Uncopyable のような基底クラスを使う方法もある。

（C++11 以降は `= delete` が推奨）
