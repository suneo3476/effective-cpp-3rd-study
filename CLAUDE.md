# Effective C++ 第3版 学習プロジェクト

## 概要
Scott Meyers著「Effective C++ 第3版」のコードを試しながら学習するプロジェクト。

## 環境
- macOS + Xcode (clang++)
- C++17
- VS Code または Terminal

## ディレクトリ構成
```
src/
  ch01/ - ch09/   # 各章のコード（itemXX.cpp）
doc/              # 講義ノート（itemXX.md）
build/            # コンパイル出力（.gitignore対象）
```

## 使い方
```bash
make run FILE=src/ch01/item01.cpp   # コンパイル＆実行
make build FILE=src/ch01/item01.cpp # コンパイルのみ
make list                           # ソース一覧
make clean                          # ビルド成果物削除
```

VS Code: `Cmd+Shift+B` で現在開いているファイルをビルド＆実行

## 進捗
- [x] Item 1: C++を言語の連合体と見なす
- [x] Item 2: #define より const, enum, inline を使え
- [x] Item 3: const を可能な限り使え
- [x] Item 4: オブジェクトは使う前に初期化せよ
- [x] Item 5: C++が自動生成する関数を知れ
- [x] Item 6: コンパイラ生成関数を明示的に禁止せよ
- [x] Item 7: ポリモーフィックな基底クラスでは仮想デストラクタを宣言せよ
- [x] Item 8: デストラクタから例外を漏らすな
- [x] Item 9: コンストラクタやデストラクタで仮想関数を呼ぶな
- [x] Item 10: 代入演算子は *this への参照を返せ
- [x] Item 11: operator= で自己代入を処理せよ

## 講義ノートについて
- `doc/itemXX.md` に各項目の講義ノートを作成
- 対話で出た質問・疑問・気づきも盛り込む
- 手書きできる分量を意識
- サンプルコードは省略せず完全な形で記載する（省略しない！）
- ユーザーからの指示は CLAUDE.md に反映する
- 各 Item の最後に「Q&A サマリ」セクションを設け、質問と回答を簡潔にリストアップする

## Git フロー（Git Flow）

### ブランチ構成
- `main`: 本番リリース用ブランチ（安定版）
- `develop`: 開発の主軸ブランチ
- `feature/itemXX`: 各 Item の機能開発用ブランチ

### 作業フロー
1. **新しい Item を始める時**
   ```bash
   git checkout develop
   git pull origin develop
   git checkout -b feature/item11
   ```

2. **作業中**
   - 講義ノート作成
   - CLAUDE.md の進捗更新
   - コミット（適宜）

3. **Item 完了時**
   ```bash
   git add .
   git commit -m "Add lecture notes for Item XX"
   git push -u origin feature/itemXX
   ```
   - PR を作成（feature/itemXX → develop）
   - マージ後、feature ブランチは削除

4. **適宜 develop から main へリリース**
   - 複数の Item が完了したタイミングなどで
   - PR を作成（develop → main）

### ブランチ命名規則
- Feature ブランチ: `feature/item{番号}` (例: `feature/item11`)
- セッションIDサフィックス付き: `feature/item{番号}-{sessionId}` (例: `feature/item11-32ySi`)

## 書籍構成
- Chapter 1 (項目1-4): C++に慣れよう
- Chapter 2 (項目5-12): コンストラクタ、デストラクタ、代入演算子
- Chapter 3 (項目13-17): リソース管理
- Chapter 4 (項目18-25): デザインと宣言
- Chapter 5 (項目26-31): 実装
- Chapter 6 (項目32-40): 継承とオブジェクト指向設計
- Chapter 7 (項目41-48): テンプレートとジェネリックプログラミング
- Chapter 8 (項目49-52): newとdeleteのカスタマイズ
- Chapter 9 (項目53-55): その他
