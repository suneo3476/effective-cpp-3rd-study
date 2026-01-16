# Item 11: operator= で自己代入を処理せよ

## 自己代入とは

自分自身に代入すること：

```cpp
class Widget { ... };

Widget w;
w = w;  // 自己代入
```

一見無意味に見えますが、これは**合法なC++コード**です。

## 現実的な自己代入のケース

`w = w` は明らかですが、こういうケースは意外と起こります：

```cpp
a[i] = a[j];  // i と j が同じ値なら自己代入
```

または：

```cpp
*px = *py;  // px と py が同じものを指していたら自己代入
```

これを**エイリアシング（aliasing）**と言います：1つのオブジェクトに複数の方法でアクセスできる状態。

## 問題のある実装例

ビットマップを持つクラス：

```cpp
class Bitmap { ... };

class Widget {
public:
    Widget& operator=(const Widget& rhs) {  // 危険な実装！
        delete pb;                  // 現在のビットマップを削除
        pb = new Bitmap(*rhs.pb);   // rhsのビットマップをコピー
        return *this;
    }
private:
    Bitmap* pb;  // ヒープ上のビットマップへのポインタ
};
```

### 自己代入で何が起きるか

```cpp
Widget w;
w = w;  // *this と rhs は同じオブジェクト
```

実行の流れ：

```cpp
Widget& operator=(const Widget& rhs) {
    // w = w の場合：
    // this == &w
    // &rhs == &w
    // だから this->pb と rhs.pb は同じポインタ（同じオブジェクトのメンバ変数）

    delete pb;  // pb を削除
                // ← rhs.pb も同じポインタだから、rhs.pb も削除される！

    pb = new Bitmap(*rhs.pb);  // 削除済みポインタにアクセス！未定義動作
    return *this;
}
```

詳細：

1. `delete pb` を実行
   - `this->pb` が指すメモリを解放
   - でも `this` と `rhs` は同じオブジェクトなので、`rhs.pb` も同じメモリを指している
   - つまり `rhs.pb` も削除されたことになる

2. `new Bitmap(*rhs.pb)` を実行しようとする
   - `rhs.pb` は削除済みのメモリを指している
   - **削除済みポインタ（dangling pointer）へのアクセス** → 未定義動作！

### ヌルポインタとの違い

- **ヌルポインタ**: `pb == nullptr`（何も指していない）
- **削除済みポインタ**: `pb == 0x1234`（アドレスは持っているが、そのメモリは解放済み）

削除済みポインタの方が危険です。nullチェックでは検出できません。

## 解決策1：同一性テスト（Identity Test）

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    if (this == &rhs) return *this;  // 自己代入チェック

    delete pb;
    pb = new Bitmap(*rhs.pb);
    return *this;
}
```

- `this` = 左辺オブジェクトのアドレス（`Widget*` 型）
- `&rhs` = 右辺オブジェクトのアドレス（`Widget*` 型）
- 両方が同じアドレスなら、自己代入

自己代入は防げますが、この実装には**別の問題**があります。

## 例外安全性の問題

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    if (this == &rhs) return *this;

    delete pb;                    // 1. pbを削除（メモリ解放）
    pb = new Bitmap(*rhs.pb);     // 2. ← ここで例外が投げられたら？
    return *this;
}
```

もし `new Bitmap(*rhs.pb)` が例外を投げたら（メモリ不足、Bitmapのコピーコンストラクタが例外を投げるなど）：

```
1. delete pb が実行される → pbが指していたメモリは解放される
2. new Bitmap(*rhs.pb) が例外を投げる
3. pb = ... の代入は実行されない（例外が投げられたから）
4. pbは削除済みのアドレスを指したまま（dangling pointer）
```

結果：
- Widgetオブジェクトは**壊れた状態**になる
- `pb` は削除済みポインタを指している
- このポインタを削除することも、読むことも危険

## 解決策2：文の順序を工夫する

**削除する前にコピーを作る**：

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    Bitmap* pOrig = pb;           // 元のpbを保存
    pb = new Bitmap(*rhs.pb);     // 新しいビットマップを作る
    delete pOrig;                 // 古いものを削除
    return *this;
}
```

順序を変えることで：
- `new` が例外を投げても、`pb` は変更されていない（安全）
- 自己代入も正しく動く（元のpbを保存してから削除するから）

### 自己代入の場合（w = w）

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    // w = w の場合：this->pb と rhs.pb は同じポインタ（例: 0x1234）

    Bitmap* pOrig = pb;           // pOrig = 0x1234（元のアドレスを保存）
    pb = new Bitmap(*rhs.pb);     // rhs.pb = 0x1234 の内容をコピー
                                  // 新しいBitmapが作られる（例: 0x5678）
                                  // pb = 0x5678
    delete pOrig;                 // 0x1234 を削除
    return *this;
}
```

流れ：
1. 元のビットマップ（0x1234）を保存
2. `rhs.pb`（0x1234）の**内容をコピー**して新しいビットマップ（0x5678）を作る
3. 古いビットマップ（0x1234）を削除

コピーを作ってから削除するので、自己代入でも正しく動きます。

### 効率性の検討

同一性テストを追加することもできます：

```cpp
Widget& Widget::operator=(const Widget& rhs) {
    if (this == &rhs) return *this;  // 効率化（自己代入の場合はスキップ）

    Bitmap* pOrig = pb;
    pb = new Bitmap(*rhs.pb);
    delete pOrig;
    return *this;
}
```

ただし：
- テストにはコスト（コードサイズ増、分岐によるパイプライン効率低下）がある
- 自己代入がどれくらいの頻度で起こるか次第

## 解決策3：Copy and Swap

**Copy and Swap** は、例外安全と自己代入安全を両方達成する賢いテクニックです。

### 基本版

```cpp
class Widget {
public:
    void swap(Widget& rhs) {  // データを交換する関数
        std::swap(pb, rhs.pb);
    }

    Widget& operator=(const Widget& rhs) {
        Widget temp(rhs);  // 1. rhsのコピーを作る
        swap(temp);        // 2. *thisとtempの中身を交換
        return *this;
    }  // 3. tempが破棄される（古いデータが入っている）

private:
    Bitmap* pb;
};
```

### ステップバイステップ

```cpp
Widget w1, w2;
w1.pb = 0x1234;  // 古いビットマップ
w2.pb = 0x5678;  // 新しいビットマップ

w1 = w2;  // 代入
```

実行の流れ：

```
1. Widget temp(w2) を作る
   temp.pb = 0x5678 のコピー（例: 0x9ABC）

2. swap(temp) を実行
   交換前: w1.pb = 0x1234, temp.pb = 0x9ABC
   交換後: w1.pb = 0x9ABC, temp.pb = 0x1234

3. } でtempが破棄される
   temp.pb（0x1234）が削除される
```

結果：
- `w1.pb` は新しいビットマップ（0x9ABC）を指す
- 古いビットマップ（0x1234）は自動的に削除される

### なぜ安全か？

**例外安全**：
- `Widget temp(rhs)` が例外を投げても、`*this`は変更されていない
- `swap`は例外を投げない（ポインタの交換だけ）

**自己代入安全**：
- 自己代入でも正しく動く（コピーを作ってから交換するから）

### 値渡しバージョン（より賢い）

```cpp
Widget& Widget::operator=(Widget rhs) {  // 値渡し！
    swap(rhs);  // rhsと交換
    return *this;
}  // rhsが破棄される
```

引数が値渡し（`Widget rhs`）になっています。関数呼び出し時に自動的にコピーが作られます：

```cpp
w1 = w2;  // Widget::operator=(w2) が呼ばれる
          // 引数が値渡しなので、w2のコピー（rhs）が作られる
```

流れ：
```
1. w2のコピー（rhs）が自動的に作られる
2. swap(rhs) で *this と rhs を交換
3. } で rhs が破棄される（古いデータが入っている）
```

基本版と同じことを、よりシンプルに実現しています。

書籍では：
> この手法は、コピー操作を関数本体からパラメータ構築に移すことで、コンパイラがより効率的なコードを生成できることがある。ただし、明瞭さを犠牲にしている面もある。

## 覚えておくこと

> `operator=` は、オブジェクトが自分自身に代入されたときにうまく動作するようにせよ。テクニックとして、ソースオブジェクトとターゲットオブジェクトのアドレスを比較する、文の順序を工夫する、copy-and-swap がある。

> 複数のオブジェクトに対して操作する関数は、それらのオブジェクトが同じである場合にも正しく動作するようにせよ。

## Q&A サマリ

1. **自己代入とは？** → オブジェクトが自分自身に代入されること（`w = w`）。

2. **自己代入は現実的に起きる？** → はい。`a[i] = a[j]` や `*px = *py` など、エイリアシングによって起きる。

3. **エイリアシングとは？** → 1つのオブジェクトに複数の方法でアクセスできる状態。

4. **ナイーブな実装の問題は？** → 自己代入時に、削除済みポインタにアクセスして未定義動作になる。

5. **削除済みポインタとヌルポインタの違いは？** → ヌルポインタは何も指さない（nullptr）。削除済みポインタはアドレスを持つが、そのメモリは解放済み。後者の方が危険。

6. **同一性テストとは？** → `if (this == &rhs) return *this;` で、自己代入をチェックすること。

7. **同一性テストだけでは不十分な理由は？** → 例外安全性がない。`new` が例外を投げると、オブジェクトが壊れた状態になる。

8. **例外安全な実装は？** → 削除する前にコピーを作る。`Bitmap* pOrig = pb; pb = new Bitmap(*rhs.pb); delete pOrig;`

9. **Copy and Swap とは？** → コピーを作ってから、`swap`で中身を交換するテクニック。例外安全と自己代入安全を両方達成。

10. **値渡しバージョンの利点は？** → コピー生成が自動化され、コードがシンプルになる。コンパイラが効率的なコードを生成できることもある。

11. **`swap` 関数は例外を投げる？** → いいえ。ポインタの交換だけなので、例外を投げない。

12. **効率性を考えると？** → 自己代入が稀なら、同一性テストのコスト（分岐、コードサイズ増）の方が高い可能性がある。

## 参考文献

- Scott Meyers『Effective C++ 第3版』項目11
