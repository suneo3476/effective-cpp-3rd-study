# Item 4: オブジェクトは使う前に初期化せよ

## 核心
C++の初期化ルールは複雑。覚えるより「常に初期化する」方が安全。

## 未初期化の危険

```cpp
int x;      // 初期化されるかも、されないかも
Point p;    // メンバが初期化されるかも、されないかも
```

未初期化値を読むと**未定義動作**。クラッシュするか、ゴミが入る。

Item 1 の視点：Cパーツ（配列など）は初期化保証なし。STLパーツ（vectorなど）は初期化される。

## 組み込み型は手動で初期化

```cpp
int x = 0;
const char* text = "Hello";
double d;
std::cin >> d;  // 入力で初期化
```

## コンストラクタ：代入 vs 初期化

### 代入（ダメ）

```cpp
class ABEntry {
private:
    std::string theName;
    std::string theAddress;
    int numTimesConsulted;
public:
    ABEntry(const std::string& name, const std::string& address) {
        theName = name;       // 代入
        theAddress = address; // 代入
        numTimesConsulted = 0;
    }
};
```

実際に起きること：
1. コンストラクタ本体に入る**前に** `theName` のデフォルトコンストラクタが呼ばれる
2. コンストラクタ本体で `name` を代入

**2回仕事してる**（デフォルト初期化 → 上書き）。

### 初期化子リスト（良い）

```cpp
ABEntry::ABEntry(const std::string& name, const std::string& address)
    : theName(name),        // 初期化
      theAddress(address),  // 初期化
      numTimesConsulted(0)  // 初期化
{}
```

`theName` が `name` で**コピーコンストラクト**される。1回で済む。

### コピーコンストラクタとは

オブジェクトを「別のオブジェクトを元に作る」時に呼ばれるコンストラクタ。

```cpp
std::string a = "Hello";
std::string b(a);  // コピーコンストラクタ：a を元に b を作る
```

### 効率の差

| 方法 | 呼ばれるもの |
|------|-------------|
| 代入 | デフォルトコンストラクタ + コピー代入演算子 |
| 初期化子リスト | コピーコンストラクタのみ |

## 初期化子リストの順序

初期化順序は**クラス定義の宣言順**で決まる。リストの記述順ではない。

```cpp
class ABEntry {
private:
    std::string theName;      // 1番目に初期化
    std::string theAddress;   // 2番目に初期化
    int numTimesConsulted;    // 3番目に初期化
};
```

混乱を避けるため、**宣言順と同じ順序で書くべき**。

## 異なるファイル間の初期化順序問題

### 問題

```cpp
// FileSystem.cpp
FileSystem tfs;  // グローバルオブジェクト

// Directory.cpp
class Directory {
public:
    Directory() {
        std::size_t disks = tfs.numDisks();  // tfs を使う
    }
};
Directory tempDir;  // グローバルオブジェクト
```

`tempDir` のコンストラクタで `tfs` を使うが、`tfs` がまだ初期化されてないかも。

**異なる翻訳単位（.cpp ファイル）の非ローカル static オブジェクトの初期化順序は未定義。**

### 解決策：関数内 static に変える

```cpp
FileSystem& tfs() {
    static FileSystem fs;  // ローカル static
    return fs;
}
```

C++ は「ローカル static は関数が初めて呼ばれた時に初期化」と保証。順序が確定する。

これは Singleton パターンと同じ構造。小さなコードでも使ってOK。

### 補足：マルチスレッド

- C++03以前：複数スレッドが同時に呼ぶと二重初期化の危険
- C++11以降：言語が自動で対処（他スレッドは待機）

今は気にしなくてOK。

## 覚えておくこと

> 組み込み型は手動で初期化せよ。C++ は初期化してくれないことがある。

> コンストラクタでは、代入ではなく初期化子リストを使え。宣言順と同じ順序で書け。

> 異なる翻訳単位間の初期化順序問題は、非ローカル static を関数内 static に置き換えて回避せよ。
