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

プログラムが確保したメモリを解放し忘れること。

```cpp
void example() {
    int* data = new int[1000];  // メモリを確保
    // ... 使う ...
    // delete[] data; を忘れた！
}  // 関数終了。data ポインタは消えるが、メモリは確保されたまま
```

### イメージ：図書館の本

```
借りる(new) → 使う → 返す(delete) → 他の人が使える
借りる → 使う → 返し忘れ → 本棚に戻らない → 誰も使えない
```

本は消えてないけど、図書館から見たら「行方不明」。貸し出し中のまま永遠に戻ってこない。

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

## 覚えておくこと

> ポリモーフィックな基底クラスでは仮想デストラクタを宣言せよ。クラスに仮想関数があれば、仮想デストラクタも持つべき。

> 基底クラスとして設計されていない、またはポリモーフィックに使う意図がないクラスでは、仮想デストラクタを宣言すべきでない。
