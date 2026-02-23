# py-entropy ⏳

[English](README.md) | [日本語](README.ja.md)

![py-entropy Logo](logo.png)

**Python初の「オーガニック・メモリ・アロケータ」**

> *"デジタル世界では、データは不老不死だ。しかし現実世界では、記憶は色褪せる。 `py-entropy` は、Pythonの変数に熱力学の法則を導入する。"*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python](https://img.shields.io/badge/Python-3.8%2B-blue)](https://www.python.org/)
[![Kernel](https://img.shields.io/badge/Kernel-Linux%205.x-red)](https://kernel.org/)

## ⚠️ 開発中 (Alpha版)

**現在、このプロジェクトはAlpha段階です。**
コアロジックは実装済みですが、カーネルモジュールの機能は極めて実験的です。バグ、クラッシュ、または予期せぬ動作が発生する可能性があります。

---

## ⚠️ 免責事項 (必ずお読みください)
**本ソフトウェアは「現状有姿」で提供され、いかなる保証もありません。**

- **実験的な性質:** 本ライブラリには、メモリアドレスを直接操作する **Linuxカーネルモジュール** が含まれています。安全性を考慮して設計されていますが、本質的に危険を伴う操作を含みます。
- **システムの安定性:** 不適切な使用やカーネルモジュールの予期せぬバグにより、システムの不安定化、フリーズ、または **カーネルパニック (OSのクラッシュ)** を引き起こす可能性があります。
- **データの安全性:** 重要な本番システムや、保存されていない重要なデータが含まれるマシンでは絶対に使用しないでください。

**作者（および貢献者）は、本ソフトウェアの使用によって生じたいかなる損害、データ損失、またはハードウェアの障害についても一切の責任を負いません。すべて自己責任でご使用ください。** ☠️

## 🧐 これは何？

**py-entropy** は、**「腐敗する変数 (Decaying Variables)」** を実装するPythonライブラリです。
明示的に削除されるまで残り続ける通常の変数とは異なり、エントロピー変数は時間の経過とともに劣化し、自然な忘却や物理的な風化をシミュレートします。

これは単なる視覚的なエフェクト（見た目だけの演出）ではありません。

* **True Mode (真のモード):** カスタム **Linuxカーネルモジュール** を使用し、RAM上のメモリビットを直接物理的に操作（破壊）します。
* **Sim Mode (シミュレーションモード):** クロスプラットフォーム（Windows / macOS）対応のため、数学的な崩壊アルゴリズムを使用します。

## ✨ 特徴

* **💀 自己破壊するデータ:** 時間の経過とともに自動的にノイズへと変わる情報。
* **🧠 オーガニック・セキュリティ:** デフォルトで「前方秘匿性 (Forward Secrecy)」を実現。古いデータは「すでに存在しない」ため、盗まれる心配がありません。
* **🔌 ハイブリッド・アーキテクチャ:** カーネルモード（ハードウェアレベル）とシミュレーションモード（ソフトウェアレベル）をシームレスに切り替えます。

## 🚀 インストール方法

```bash
git clone [https://github.com/kazuki0114/py-entropy.git](https://github.com/kazuki0114/py-entropy.git)
cd py-entropy
pip install -e .
```

**True Entropy Mode (Linux限定)** を有効にするには、カーネルモジュールをコンパイルして組み込む必要があります：

```bash
cd kernel_module
make
sudo insmod entropy_mem.ko
sudo chmod 666 /dev/entropy_mem
```

## 📖 使い方

### 基本的な使い方

```python
import time
from entropy import DecayString

# 時間とともに腐敗する変数を定義する
# カーネルモジュールがロードされている場合、物理的なRAM操作が使用されます。
secret = DecayString("This is a top secret message.")

print(f"Original: {secret}")

# エントロピーが仕事をするのを待つ...
print("Waiting for decay...")
time.sleep(5)

# データは破損（腐敗）しました
print(f"5s Later: {secret}") 
# 出力例: "Th%s is @ tOp s#crXt mXs?age."
```
