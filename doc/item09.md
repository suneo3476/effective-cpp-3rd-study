# Item 9: コンストラクタやデストラクタで仮想関数を呼ぶな

## 問題：アンチパターン

株式取引のクラスで、すべての取引をログに記録したい：

```cpp
class Transaction {  // すべての取引の基底クラス
public:
    Transaction() {
        // ...
        logTransaction();  // 最後にログを記録
    }

    virtual void logTransaction() const = 0;  // 純粋仮想関数
};

class BuyTransaction : public Transaction {
public:
    virtual void logTransaction() const {
        std::cout << "Buy transaction logged" << std::endl;
    }
};

class SellTransaction : public Transaction {
public:
    virtual void logTransaction() const {
        std::cout << "Sell transaction logged" << std::endl;
    }
};
```

このコードを実行：

```cpp
BuyTransaction b;
```

**Q: どの`logTransaction`が呼ばれるか？**

答え：`Transaction::logTransaction()`が呼ばれる（`BuyTransaction::logTransaction()`ではない！）

## 純粋仮想関数とは

### `= 0` の意味

```cpp
virtual void logTransaction() const = 0;  // 純粋仮想関数
```

**純粋仮想関数 = 実装を持たない仮想関数**

意味：
- この関数は基底クラスでは実装しない
- 派生クラスで**必ず**実装しなければならない
- この関数を持つクラスは**抽象クラス**になる

### 抽象クラス

純粋仮想関数を1つでも持つクラスは**抽象クラス**：

```cpp
class Transaction {  // 抽象クラス
public:
    virtual void logTransaction() const = 0;  // 純粋仮想関数
};

Transaction t;  // エラー！抽象クラスのオブジェクトは作れない
```

派生クラスで実装すれば、オブジェクトを作れる：

```cpp
class BuyTransaction : public Transaction {
public:
    virtual void logTransaction() const {  // 実装を提供
        std::cout << "Buy transaction logged" << std::endl;
    }
};

BuyTransaction b;  // OK！
```

## コンストラクタの呼び出し順序

```cpp
BuyTransaction b;
```

実行順序：
```
1. Transaction のコンストラクタ ← 先に呼ばれる！
2. BuyTransaction のコンストラクタ ← その後
```

**基底クラスのコンストラクタが先、派生クラスのコンストラクタが後**

これはItem 4でも出てきた、C++の基本的な仕様です。

## なぜ派生クラスの関数が呼ばれないのか

**`Transaction`のコンストラクタ実行中、オブジェクトの型は`Transaction`型として扱われる**

理由：
- この時点では`BuyTransaction`固有のメンバ変数はまだ初期化されていない
- まだ`BuyTransaction`オブジェクトとして完成していない

もし`BuyTransaction::logTransaction()`が呼ばれたら：

```cpp
class BuyTransaction : public Transaction {
public:
    BuyTransaction() : price_(100.0) {}  // ← まだ実行されていない

    virtual void logTransaction() const {
        std::cout << "Buy at price: " << price_ << std::endl;
        // ↑ price_ を使っている
    }
private:
    double price_;  // ← まだ初期化されていない！
};
```

- `price_`はまだ初期化されていない
- 未定義動作！ゴミの値が読まれる

**だからC++は、コンストラクタ実行中は派生クラスの仮想関数を呼ばない**

## vptr の変化

Item 7で学んだvptrも、コンストラクタ実行中に変化します：

```
1. Transaction コンストラクタ開始
   ↓ vptr → Transaction の vtable を指す
2. Transaction コンストラクタ終了
   ↓
3. BuyTransaction コンストラクタ開始
   ↓ vptr → BuyTransaction の vtable に更新される
4. BuyTransaction コンストラクタ終了
```

コンストラクタ実行中、オブジェクトは「現在構築中のクラスの型」として扱われます。

## デストラクタでも同じ問題

デストラクタは**派生クラス→基底クラスの順**で呼ばれます（コンストラクタと逆順）。

```cpp
#include <iostream>

class Transaction {
public:
    Transaction() {
        std::cout << "Transaction constructor" << std::endl;
    }

    virtual ~Transaction() {
        std::cout << "Transaction destructor" << std::endl;
        logTransaction();  // 仮想関数を呼ぶ
    }

    virtual void logTransaction() const {
        std::cout << "Transaction::logTransaction()" << std::endl;
    }
};

class BuyTransaction : public Transaction {
public:
    BuyTransaction() : price_(100.0) {
        std::cout << "BuyTransaction constructor" << std::endl;
    }

    virtual ~BuyTransaction() {
        std::cout << "BuyTransaction destructor" << std::endl;
    }

    virtual void logTransaction() const {
        std::cout << "BuyTransaction::logTransaction() - price: " << price_ << std::endl;
    }

private:
    double price_;
};

int main() {
    {
        BuyTransaction b;
    }  // ここでbが破棄される

    return 0;
}
```

実行結果：
```
Transaction constructor
BuyTransaction constructor
BuyTransaction destructor
Transaction destructor
Transaction::logTransaction()  ← BuyTransaction版ではない！
```

順序：
```
1. BuyTransaction のデストラクタ実行
2. Transaction のデストラクタ実行
   ↓ この時点で派生クラス部分（price_）はもう破棄されている
   ↓ だから Transaction::logTransaction() が呼ばれる
```

もし`BuyTransaction::logTransaction()`が呼ばれたら、`price_`はすでに破棄済みで未定義動作になります。

## 検出しやすいケース vs 検出しにくいケース

### 検出しやすいケース

```cpp
class Transaction {
public:
    Transaction() {
        logTransaction();  // 直接呼んでいる
    }
    virtual void logTransaction() const = 0;  // 純粋仮想関数
};
```

- コンパイラが警告を出すことがある
- `logTransaction()`が純粋仮想（`= 0`）なので、リンクエラーになる可能性が高い

### 検出しにくいケース

```cpp
class Transaction {
public:
    Transaction() {
        init();  // ヘルパー関数を呼ぶ
    }

    virtual void logTransaction() const = 0;

private:
    void init() {  // 非仮想のヘルパー関数
        // ...
        logTransaction();  // ← ここで仮想関数を呼んでいる！
    }
};
```

この場合：
- コンストラクタから**間接的に**仮想関数を呼んでいる
- コンパイラは警告を出さないことが多い
- 純粋仮想関数なら実行時にアボートする
- 普通の仮想関数なら、間違ったバージョンが呼ばれるだけで気づきにくい

`init`を`virtual`にしても解決しません。なぜなら、`Transaction`のコンストラクタ実行中は、オブジェクトの型は`Transaction`なので、`Transaction::init()`が呼ばれるからです。

## 解決策：ベストプラクティス

**発想を逆転させる：「基底クラスから派生クラスに降りる」のではなく、「派生クラスから基底クラスに情報を渡す」**

```cpp
#include <string>
#include <iostream>

class Transaction {
public:
    explicit Transaction(const std::string& logInfo) {
        logTransaction(logInfo);  // 非仮想関数を呼ぶ
    }

    void logTransaction(const std::string& logInfo) const {  // 非仮想！
        std::cout << "Log: " << logInfo << std::endl;
    }
};

class BuyTransaction : public Transaction {
public:
    BuyTransaction(const std::string& symbol, double price)
        : Transaction(createLogString(symbol, price))  // 情報を渡す
    {
        symbol_ = symbol;
        price_ = price;
    }

private:
    static std::string createLogString(const std::string& symbol, double price) {
        return "Buy " + symbol + " at " + std::to_string(price);
    }

    std::string symbol_;
    double price_;
};

int main() {
    BuyTransaction b("AAPL", 150.0);
    // 出力: Log: Buy AAPL at 150.000000
    return 0;
}
```

ポイント：
1. `logTransaction`を**非仮想関数**にする
2. 派生クラスのコンストラクタで、必要な情報（`symbol`, `price`）から文字列を作る
3. その文字列を基底クラスのコンストラクタに渡す
4. 基底クラスは渡された情報でログを記録する

## メンバ初期化リストの文法

```cpp
BuyTransaction(const std::string& symbol, double price)
    : Transaction(createLogString(symbol, price))  // ← メンバ初期化リスト
{                                                   // ← ここから
    symbol_ = symbol;                               // コンストラクタ本体
    price_ = price;
}
```

**`: Transaction(...)`** の意味：

- **基底クラスのコンストラクタを明示的に呼ぶ**記法
- メンバ初期化リストの一部

もし書かなかったら、`Transaction`のデフォルトコンストラクタが自動的に呼ばれます。

メンバ変数の初期化も同じ場所に書けます：

```cpp
BuyTransaction(const std::string& symbol, double price)
    : Transaction(createLogString(symbol, price)),  // 基底クラス
      symbol_(symbol),                              // メンバ変数
      price_(price)                                 // メンバ変数
{
    // コンストラクタ本体
}
```

## なぜ static なのか

```cpp
class BuyTransaction : public Transaction {
public:
    BuyTransaction(const std::string& symbol, double price)
        : Transaction(createLogString(symbol, price))  // ← この時点
    {
    }

private:
    static std::string createLogString(...);
    std::string symbol_;  // ← まだ初期化されていない
    double price_;        // ← まだ初期化されていない
};
```

**`static`メンバ関数は`this`ポインタを持たない → メンバ変数にアクセスできない**

```cpp
static std::string createLogString(const std::string& symbol, double price) {
    // symbol_ や price_ にはアクセスできない！
    // this->symbol_ もエラー
    return "Buy " + symbol + " at " + std::to_string(price);
    // 引数で渡された symbol と price だけを使う
}
```

メンバ初期化リストで呼ばれる時点では、`symbol_`や`price_`はまだ初期化されていません。

`static`にすることで、**コンパイラが未初期化メンバへのアクセスを防いでくれる**んです。

## explicit とは

`explicit`は**暗黙的な型変換を防ぐ**キーワードです。

### explicit なし（暗黙的な変換が起きる）

```cpp
class Transaction {
public:
    Transaction(const std::string& logInfo) {  // explicit なし
        // ...
    }
};

void processTransaction(Transaction t) {
    // ...
}

int main() {
    std::string s = "hello";
    processTransaction(s);  // OK! 暗黙的に変換される
    // std::string → Transaction
}
```

`s`（`std::string`型）が渡されると：
1. 関数は`Transaction`型を期待している
2. コンパイラが「`Transaction(const std::string&)`というコンストラクタがある！」と見つける
3. 自動的に`Transaction(s)`を呼んで、一時的な`Transaction`オブジェクトを作る
4. その一時オブジェクトが関数に渡される

つまり：
```cpp
processTransaction(s);
// ↓ コンパイラが自動的にこう解釈する
processTransaction(Transaction(s));
```

### explicit あり（暗黙的な変換を禁止）

```cpp
class Transaction {
public:
    explicit Transaction(const std::string& logInfo) {  // explicit
        // ...
    }
};

int main() {
    std::string s = "hello";
    processTransaction(s);  // エラー！暗黙的な変換は禁止

    processTransaction(Transaction(s));  // OK! 明示的に変換
}
```

`explicit`をつけると、明示的にコンストラクタを呼ばないと変換できません。

意図しない変換を防ぐために使います。

## 実行順序の詳細

コンストラクタは2つの部分からできています：

1. **メンバ初期化リスト**：`:` の後ろの部分
2. **コンストラクタ本体**：`{ }` の中

実行順序：

```
BuyTransaction b("AAPL", 150.0);
```

1. **メンバ初期化リストが評価される**
   - `createLogString("AAPL", 150.0)` が呼ばれる
   - 戻り値：`"Buy AAPL at 150.000000"` という文字列

2. **Transactionのコンストラクタが呼ばれる**
   - `logInfo = "Buy AAPL at 150.000000"`
   - `logTransaction(logInfo)` を呼ぶ → ログが出る

3. **BuyTransactionのコンストラクタ本体が実行される**
   - `symbol_ = symbol;`
   - `price_ = price;`

実行順序：
```
メンバ初期化リスト（基底クラス含む）
  ↓
コンストラクタ本体
```

## ユースケースパターン

このパターンが有効なのは：

**共通点：**
- **基底クラスで共通の処理**をしたい（ログ、登録、検証など）
- **派生クラスごとに異なる情報**が必要
- **コンストラクタ実行中に行う必要**がある

### 1. ロギング・監査（書籍の例）

```cpp
class Transaction {
public:
    Transaction(const std::string& logEntry) {
        auditLog(logEntry);  // すべての取引を記録
    }
};

class BuyTransaction : public Transaction {
public:
    BuyTransaction(const std::string& symbol, double price)
        : Transaction(createLog(symbol, price)) {}
};
```

**ユースケース**：金融システム、セキュリティシステム

### 2. リソース管理

```cpp
class ResourceHandle {
public:
    ResourceHandle(const std::string& resourceId) {
        registerResource(resourceId);  // リソースを登録
    }
    ~ResourceHandle() {
        unregisterResource();
    }
};

class FileHandle : public ResourceHandle {
public:
    FileHandle(const std::string& filename)
        : ResourceHandle(createResourceId(filename)) {}
private:
    static std::string createResourceId(const std::string& filename);
};
```

**ユースケース**：ファイル、データベース接続、ネットワークソケットの管理

### 3. オブジェクト登録・追跡

```cpp
class GameObject {
public:
    GameObject(const ObjectInfo& info) {
        GameWorld::instance().registerObject(this, info);
    }
};

class Enemy : public GameObject {
public:
    Enemy(const std::string& type, int level)
        : GameObject(createInfo(type, level)) {}
private:
    static ObjectInfo createInfo(const std::string& type, int level);
};
```

**ユースケース**：ゲームエンジン、GUIフレームワーク

### 4. 統計・メトリクス収集

```cpp
class Request {
public:
    Request(const RequestStats& stats) {
        MetricsCollector::record(stats);
    }
};

class HttpRequest : public Request {
public:
    HttpRequest(const std::string& method, const std::string& url)
        : Request(createStats(method, url)) {}
};
```

**ユースケース**：Webサーバー、パフォーマンス監視

### 5. 初期化の検証

```cpp
class ConfigurableObject {
public:
    ConfigurableObject(const Config& config) {
        validate(config);  // 設定を検証
        apply(config);     // 設定を適用
    }
};

class NetworkDevice : public ConfigurableObject {
public:
    NetworkDevice(const std::string& ip, int port)
        : ConfigurableObject(createConfig(ip, port)) {}
};
```

**ユースケース**：設定管理、入力検証

## 覚えておくこと

> コンストラクタやデストラクタで仮想関数を呼ぶな。そのような呼び出しは、現在実行中のコンストラクタやデストラクタが属するクラスよりも派生したクラスには決して降りていかない。

## アンチパターンとベストプラクティスの抽象化

### アンチパターン

```cpp
class Base {
public:
    Base() {
        virtualFunc();  // ダメ！派生クラスの関数は呼ばれない
    }
    virtual void virtualFunc() = 0;
};

class Derived : public Base {
public:
    virtual void virtualFunc() {
        // これは呼ばれない！
    }
};
```

**問題点**：
- コンストラクタ/デストラクタで仮想関数を呼ぶ
- 派生クラスの関数は呼ばれず、未初期化メンバにアクセスする危険

### ベストプラクティス

```cpp
class Base {
public:
    Base(const Info& info) {  // 情報を受け取る
        nonVirtualFunc(info);  // 非仮想関数を呼ぶ
    }
    void nonVirtualFunc(const Info& info);  // 非仮想
};

class Derived : public Base {
public:
    Derived(params)
        : Base(createInfo(params))  // 情報を渡す
    {
    }
private:
    static Info createInfo(params);  // static関数で情報を作る
};
```

**ポイント**：
- 仮想関数を使わない（非仮想関数にする）
- 派生クラス→基底クラスに情報を渡す（逆方向）
- `static`関数で未初期化メンバへのアクセスを防ぐ

## Q&A サマリ

1. **純粋仮想関数とは？** → `= 0`で宣言する、実装を持たない仮想関数。派生クラスで必ず実装する必要がある。

2. **抽象クラスとは？** → 純粋仮想関数を1つでも持つクラス。インスタンス化できない。

3. **コンストラクタの呼び出し順序は？** → 基底クラスのコンストラクタが先、派生クラスのコンストラクタが後。

4. **コンストラクタで仮想関数を呼ぶと何が起きる？** → 派生クラスの関数は呼ばれず、基底クラスの関数が呼ばれる。

5. **なぜ派生クラスの関数が呼ばれない？** → コンストラクタ実行中、オブジェクトの型は「現在構築中のクラスの型」として扱われる。派生クラスのメンバ変数はまだ初期化されていないため。

6. **デストラクタでも同じ問題？** → はい。デストラクタ実行時は、派生クラス部分はすでに破棄されている。

7. **`init()`を`virtual`にすれば解決する？** → いいえ。コンストラクタ実行中は基底クラスの`init()`が呼ばれる。

8. **解決策は？** → 仮想関数を使わず、派生クラスから基底クラスに情報を渡す。

9. **メンバ初期化リストとは？** → `: 基底クラス(...), メンバ変数(...)` という記法。コンストラクタ本体の前に実行される。

10. **なぜ`createLogString`を`static`にする？** → 未初期化メンバ変数へのアクセスを防ぐため。`static`関数は`this`ポインタを持たない。

11. **`explicit`とは？** → 暗黙的な型変換を防ぐキーワード。意図しないコンストラクタ呼び出しを防ぐ。

12. **このパターンが有効なのは？** → 基底クラスで共通の処理をしたい、派生クラスごとに異なる情報が必要、コンストラクタ実行中に行う必要がある場合。

## 参考文献

- Scott Meyers『Effective C++ 第3版』項目9
