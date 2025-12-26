# Item 2: #define より const, enum, inline を使え

## 核心
プリプロセッサよりコンパイラを使え。

## 問題1: 定数マクロ

```cpp
#define ASPECT_RATIO 1.653  // ダメ
const double AspectRatio = 1.653;  // 良い
```

**なぜダメか：**
- プリプロセッサが置換 → シンボルテーブルに残らない
- エラーメッセージに `1.653` と出て意味不明
- デバッガでも見えない

## 問題2: #define はスコープを無視

```cpp
// 他のライブラリ
#define MAX 100

// 自分のコード
class MyClass {
    int MAX;  // コンパイルエラー！「int 100」になる
};
```

**const なら安全：**
```cpp
class GamePlayer {
private:
    static const int NumTurns = 5;  // クラスごとに別、private可
};
```

## enum hack

```cpp
class GamePlayer {
private:
    enum { NumTurns = 5 };
    int scores[NumTurns];
};
```

**使う理由：**
1. アドレスを取らせたくない時
2. TMPで基本テクニック（古いコードで見かける）

## 問題3: 関数マクロ

```cpp
#define CALL_WITH_MAX(a, b) f((a) > (b) ? (a) : (b))

int a = 5, b = 0;
CALL_WITH_MAX(++a, b);     // a が2回インクリメント！
CALL_WITH_MAX(++a, b+10);  // a は1回だけ
```

**inline テンプレートで解決：**
```cpp
template<typename T>
inline void callWithMax(const T& a, const T& b) {
    f(a > b ? a : b);
}
```

## 覚えておくこと
> 単純な定数には、#define より const か enum を使え。
> 関数マクロには、#define より inline 関数を使え。
