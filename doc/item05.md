# Item 5: C++が自動生成する関数を知れ

## 核心
空のクラスでも、コンパイラが勝手に関数を生成する。

## 空のクラスは空じゃない

```cpp
class Empty {};
```

これを書くと、コンパイラがこうする：

```cpp
class Empty {
public:
    Empty() { ... }                            // デフォルトコンストラクタ
    Empty(const Empty& rhs) { ... }            // コピーコンストラクタ
    ~Empty() { ... }                           // デストラクタ
    Empty& operator=(const Empty& rhs) { ... } // コピー代入演算子
};
```

全部 public で inline。

## いつ生成される？

使われたとき。

```cpp
Empty e1;       // デフォルトコンストラクタ、デストラクタ
Empty e2(e1);   // コピーコンストラクタ
e2 = e1;        // コピー代入演算子
```

## 自動生成の中身

メンバを持つクラスの例：

```cpp
template<typename T>
class NamedObject {
public:
    NamedObject(const std::string& name, const T& value);
private:
    std::string nameValue;
    T objectValue;
};
```

コンストラクタを1つ定義したので：
- デフォルトコンストラクタ → **生成されない**
- コピーコンストラクタ → **生成される**（各メンバをコピー）
- コピー代入演算子 → **生成される**（各メンバをコピー）
- デストラクタ → **生成される**

### Q: なぜデフォルトコンストラクタが生成されない？

「このクラスは引数なしで作っちゃダメ」という設計意図を尊重するため。名前と値がないと意味がない NamedObject を、空っぽで作れたら困る。

## 自動生成されないケース

参照メンバや const メンバがあると、コピー代入演算子は生成されない：

```cpp
template<typename T>
class NamedObject {
public:
    NamedObject(std::string& name, const T& value);
private:
    std::string& nameValue;   // 参照
    const T objectValue;      // const
};

std::string newDog("Persephone");
std::string oldDog("Satch");

NamedObject<int> p(newDog, 2);   // p.nameValue は newDog を指す
NamedObject<int> s(oldDog, 36);  // s.nameValue は oldDog を指す

NamedObject<int> copy(p);  // コピーコンストラクタ → OK
p = s;                     // コピー代入演算子 → エラー！
```

### Q: なぜコピーコンストラクタは OK で代入はダメ？

**コピーコンストラクタ**：新しいオブジェクトを「作る」。参照も const も初期化できる。

**代入**：既にあるものを「変える」。
- 参照先を変える？ → C++ では参照先は変えられない
- 参照先の中身を変える？ → newDog が "Satch" に書き換わる。意図してる？
- const を変える？ → 不可能

コンパイラは判断できないから、自動生成を拒否する。

### Q: 参照や const があっても代入を可能にするには？

自分でコピー代入演算子を定義する。ただし const は変えられないので、現実的には：

```cpp
class NamedObject {
public:
    NamedObject& operator=(const NamedObject&) = delete;  // 代入禁止
};
```

参照や const をメンバに持つ = 「一度作ったら変わらない」設計意図。代入を許す方が不自然。

## 自動生成のルールまとめ

| 種類 | プログラマが定義した | コンパイラは |
|------|---------------------|-------------|
| コンストラクタ | 1つでも定義した | デフォルトコンストラクタを生成しない |
| コピーコンストラクタ | 定義した | 生成しない |
| コピー代入演算子 | 定義した | 生成しない |
| デストラクタ | 定義した | 生成しない |

「自分で書いたなら任せる、書いてないなら代わりに作る」

## 覚えておくこと

> コンパイラは、デフォルトコンストラクタ、コピーコンストラクタ、コピー代入演算子、デストラクタを暗黙に生成することがある。

ただし：
- コンストラクタを1つでも書いたら、デフォルトコンストラクタは生成されない
- 参照メンバや const メンバがあると、コピー代入演算子は生成されない
