# Item 7: ポリモーフィックな基底クラスでは仮想デストラクタを宣言せよ

## 前提知識：仮想関数とは

### 仮想関数（virtual）

```cpp
class TimeKeeper {
public:
    virtual void showTime() {  // 仮想関数
        std::cout << "TimeKeeper::showTime" << std::endl;
    }
};

class AtomicClock : public TimeKeeper {
public:
    void showTime() override {  // オーバーライド
        std::cout << "AtomicClock::showTime" << std::endl;
    }
};
```

```cpp
TimeKeeper* ptk = new AtomicClock();
ptk->showTime();  // AtomicClock::showTime が呼ばれる
```

`virtual` がついてると、**実際のオブジェクトの型**（AtomicClock）の関数が呼ばれる。

### 非仮想関数

```cpp
class TimeKeeper {
public:
    void showTime() {  // virtual なし = 非仮想
        std::cout << "TimeKeeper::showTime" << std::endl;
    }
};

class AtomicClock : public TimeKeeper {
public:
    void showTime() {
        std::cout << "AtomicClock::showTime" << std::endl;
    }
};
```

```cpp
TimeKeeper* ptk = new AtomicClock();
ptk->showTime();  // TimeKeeper::showTime が呼ばれる
```

`virtual` がないと、**ポインタの型**（TimeKeeper*）の関数が呼ばれる。

### まとめ

| | 仮想（virtual） | 非仮想 |
|--|----------------|--------|
| 呼ばれる関数 | 実際のオブジェクトの型 | ポインタの型 |

## 問題：非仮想デストラクタで派生クラスを削除

```cpp
class TimeKeeper {
public:
    TimeKeeper() {}
    ~TimeKeeper() {}  // 非仮想デストラクタ
};

class AtomicClock : public TimeKeeper {
public:
    AtomicClock() {
        data = new int[1000];  // 派生クラス固有のリソース
    }
    ~AtomicClock() {
        delete[] data;  // このデストラクタが呼ばれないと...
    }
private:
    int* data;
};

class WaterClock : public TimeKeeper {
    // ...
};

class WristWatch : public TimeKeeper {
    // ...
};

// ファクトリ関数：派生クラスのオブジェクトを new して返す
TimeKeeper* getTimeKeeper() {
    return new AtomicClock();  // 例えば AtomicClock を返す
}
```

使う側：

```cpp
TimeKeeper* ptk = getTimeKeeper();  // 実際は AtomicClock
// ... 使う ...
delete ptk;  // 問題！
```

非仮想だから、ポインタの型（TimeKeeper*）のデストラクタだけが呼ばれる。

結果：
- `~TimeKeeper()` → 呼ばれる（空なので何もしない）
- `~AtomicClock()` → **呼ばれない！**

`AtomicClock::data` が解放されない。**メモリリーク**。未定義動作。

### Q: ~TimeKeeper() が呼ばれたとき何が起きる？

エラーは出ない。`~TimeKeeper()` 自体は正常に動く。でもその後の「AtomicClock 部分の後始末」が一切行われない。エラーにならないのが怖いところ。

## メモリリークとは

プログラムが確保したメモリを解放し忘れること。「借用と返却」で考えると分かりやすい。

```cpp
void example() {
    int* data = new int[1000];  // メモリを借用
    // ... 使う ...
    // delete[] data; を忘れた！ → 返却忘れ
}  // 関数終了。data ポインタは消えるが、メモリは借用されたまま
```

### イメージ：図書館の本 / 駐車場

```
借りる(new) → 使う → 返す(delete) → 他の人が使える
借りる → 使う → 返し忘れ → 本棚に戻らない → 誰も使えない
```

「leak（漏れ）」より「lost（失われた）」の方が実態に近いかも。

## 解決策：仮想デストラクタ

```cpp
class TimeKeeper {
public:
    TimeKeeper() {}
    virtual ~TimeKeeper() {}  // 仮想デストラクタ
};
```

これで `delete ptk` すると、実際のオブジェクト（AtomicClock）のデストラクタが呼ばれる。

### `delete ptk` で起きること（仮想デストラクタの場合）

1. `~AtomicClock()` が呼ばれる → `delete[] data` が実行される
2. `~TimeKeeper()` が呼ばれる（自動で基底クラスのデストラクタが呼ばれる）
3. AtomicClock オブジェクト全体のメモリが解放される

## メモリマップ（64bit アーキテクチャ）

```cpp
class TimeKeeper {
public:
    virtual ~TimeKeeper() {}
};

class AtomicClock : public TimeKeeper {
public:
    AtomicClock() { data = new int[1000]; }
    ~AtomicClock() { delete[] data; }
private:
    int* data;
};

AtomicClock* ac = new AtomicClock();
```

```
AtomicClock オブジェクト（ヒープ上）
┌─────────────────────────────────┐
│ vptr (8 bytes)                  │ ← TimeKeeper 部分（仮想関数テーブルへのポインタ）
├─────────────────────────────────┤
│ data (8 bytes)                  │ ← AtomicClock 部分（int* ポインタ）
└─────────────────────────────────┘
         │
         │ data が指す先
         ▼
┌─────────────────────────────────────────────────────┐
│ int[1000] (4000 bytes)                              │ ← 別のヒープ領域
└─────────────────────────────────────────────────────┘
```

### 非仮想デストラクタの場合のリーク

```
delete ptk; で起きること:

┌─────────────────────────────────┐
│ vptr (8 bytes)                  │ ← 解放される？（未定義動作）
├─────────────────────────────────┤
│ data (8 bytes)                  │ ← ポインタは消えるが...
└─────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────┐
│ int[1000] (4000 bytes)                              │ ← 解放されない！リーク！
└─────────────────────────────────────────────────────┘
```

`data` ポインタ自体は消えるけど、それが指してた 4000 bytes は誰も解放しない。

## プログラムのメモリ領域

```
高位アドレス
┌─────────────────────────────┐
│ スタック (Stack)            │ ← ローカル変数（家の細長い駐車場）
├─────────────────────────────┤
│ ヒープ (Heap)               │ ← new で確保（ホームセンターの広い駐車場）
├─────────────────────────────┤
│ データ領域                  │ ← グローバル変数、static 変数
├─────────────────────────────┤
│ テキスト領域                │ ← 機械語（ソースコードではない）
└─────────────────────────────┘
低位アドレス
```

- **スタック**：積み重ね。関数呼び出しで積み、戻ると下ろす。自動管理。
- **ヒープ**：自由領域。好きな時に借用、好きな時に返却。手動管理。

## vptr（仮想テーブルポインタ）とは

仮想関数を呼ぶとき、実行時にどの関数を呼ぶか決めるためのポインタ。

```
vtbl（仮想テーブル）- クラスごとに1つ

Dog の vtbl:
┌─────────────────────────┐
│ &Dog::speak             │
└─────────────────────────┘

Dog オブジェクト:
┌─────────┐      ┌─────────────────────────┐
│ vptr    │ ──→  │ &Dog::speak             │
└─────────┘      └─────────────────────────┘
```

## 仮想デストラクタのコスト

```cpp
class Point {
public:
    Point(int xCoord, int yCoord);
    ~Point();  // 非仮想
private:
    int x, y;
};
```

virtual なし: 64bit（int 2つ）
virtual あり: 128bit（vptr 64bit + int 2つ）

サイズが2倍になる！

### なぜ問題か

- **レジスタに収まらない**：x86-64 のレジスタは 64bit。128bit は2つ必要。
- **レジスタ枯渇**：汎用レジスタは16個しかない。足りなくなるとメモリ退避で遅くなる。
- **C言語との互換性**：vptr があると C の構造体と互換性がなくなる。

## いつ virtual をつけるべきか

| クラスの性質 | 仮想デストラクタ |
|-------------|-----------------|
| ポリモーフィックな基底クラス（継承して使う、virtual 関数あり） | **必要** |
| 基底クラスとして使わない | **不要** |
| std::string、STL コンテナ | **継承するな！** |

### 危険な例

```cpp
class SpecialString : public std::string {  // ダメ！
    // std::string は非仮想デストラクタ
};

SpecialString* pss = new SpecialString("Doom");
std::string* ps = pss;
delete ps;  // 未定義動作！SpecialString 部分がリーク
```

### 簡単なルール

> 仮想関数が1つでもあれば、仮想デストラクタを持つべき

## 覚えておくこと

> ポリモーフィックな基底クラスでは仮想デストラクタを宣言せよ。クラスに仮想関数があれば、仮想デストラクタも持つべき。

> 基底クラスとして設計されていない、またはポリモーフィックに使う意図がないクラスでは、仮想デストラクタを宣言すべきでない。
