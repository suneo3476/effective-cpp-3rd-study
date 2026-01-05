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
- `~TimeKeeper()` → 呼ばれる
- `~AtomicClock()` → **呼ばれない！**

`AtomicClock::data` が解放されない。**メモリリーク**。未定義動作。

## 解決策：仮想デストラクタ

```cpp
class TimeKeeper {
public:
    TimeKeeper() {}
    virtual ~TimeKeeper() {}  // 仮想デストラクタ
};
```

これで `delete ptk` すると、実際のオブジェクト（AtomicClock）のデストラクタが呼ばれる。

## 補足：将来学ぶこと

メモリ上で virtual がどう実装されているか（vptr, vtbl）は興味深いトピック。今は「virtual をつけると実際の型の関数が呼ばれる」と覚えておけば OK。

## 覚えておくこと

> ポリモーフィックな基底クラスでは仮想デストラクタを宣言せよ。クラスに仮想関数があれば、仮想デストラクタも持つべき。

> 基底クラスとして設計されていない、またはポリモーフィックに使う意図がないクラスでは、仮想デストラクタを宣言すべきでない。
