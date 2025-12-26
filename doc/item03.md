# Item 3: const を可能な限り使え

## 核心
const は「変更しない」という約束。コンパイラが強制してくれる。

## ポインタと const

```cpp
const char* p;        // 中身が const
char* const p;        // ポインタが const
const char* const p;  // 両方 const
```

**覚え方：右から左に読む**
- `const char* p` → pointer to char const → 中身が const
- `char* const p` → const pointer to char → ポインタが const

## イテレータと const

```cpp
const std::vector<int>::iterator iter;   // イテレータ自体が const（≒ int* const）
std::vector<int>::const_iterator cIter;  // 中身が const（≒ const int*）
```

**実用上は `const_iterator` がほとんど。** 読み取り専用ループで使う。

## 参照とは

エイリアス（別名）。勝手に `*` がつくポインタと思っていい。

```cpp
char& c = text[0];  // c は text[0] の別名
c = 'J';            // text[0] が 'J' になる
```

ポインタより安全：
```cpp
char* p = nullptr;  // OK
char& r;            // コンパイルエラー！必ず何かを指す
```

## const メンバ関数

```cpp
class TextBlock {
public:
    const char& operator[](std::size_t pos) const;  // const オブジェクト用
    char& operator[](std::size_t pos);              // 非 const オブジェクト用
};
```

末尾の `const` = 「この関数はオブジェクトを変更しません」

**両方必要な理由：**
- const 版だけ → 書き換えできない
- 非 const 版だけ → const オブジェクトで使えない

**`char& operator[](...) const` は論理的にバグ：**
「変更しません」と言いつつ、書き換え可能な参照を返してる。

## bitwise vs logical constness

**bitwise constness:** 1ビットも変えない（コンパイラが強制）
**logical constness:** 外から見て変わってなければOK

```cpp
class CTextBlock {
    mutable std::size_t textLength;    // const関数内でも変更可
    mutable bool lengthIsValid;

    std::size_t length() const {
        if (!lengthIsValid) {
            textLength = std::strlen(pText);  // キャッシュ更新
            lengthIsValid = true;
        }
        return textLength;
    }
};
```

**mutable = プログラマの意図しない副作用に相当しない副作用**

## 重複を避ける

非 const 版から const 版を呼ぶ。逆はダメ。

```cpp
char& operator[](std::size_t pos) {
    return const_cast<char&>(
        static_cast<const TextBlock&>(*this)[pos]
    );
}
```

**なぜ逆がダメか：**
- const 関数は「変更しない」約束
- 非 const 関数は何するか分からない
- 約束を守れない相手に任せるのは危険

## 覚えておくこと
> const を宣言することで、コンパイラが使い方のミスを検出してくれる。

> コンパイラは bitwise constness を強制する。でもプログラマは logical constness で考えるべき。

> const 版と非 const 版が同じ実装なら、非 const 版から const 版を呼んで重複を避ける。
