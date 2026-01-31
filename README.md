# py-entropy ⏳

![py-entropy Logo](logo.png)

**The First Organic Memory Allocator for Python.**

> *"In the digital world, data is immortal. In the real world, memory fades. `py-entropy` introduces the laws of thermodynamics to your Python variables."*

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Python](https://img.shields.io/badge/Python-3.8%2B-blue)](https://www.python.org/)
[![Kernel](https://img.shields.io/badge/Kernel-Linux%205.x-red)](https://kernel.org/)

## ⚠️ Work In Progress (Development Phase)

**This project is currently in the Alpha stage.**
The core logic is implemented, but the Kernel Module features are experimental. Expect bugs, crashes, or unexpected behavior.

---

## 🧐 What is this?

**py-entropy** is a Python library that implements **"Decaying Variables"**.
Unlike standard variables that persist until deleted, Entropy Variables degrade over time, mimicking natural forgetting or physical weathering.

This is not just a visual effect.

* **True Mode:** Uses a custom **Linux Kernel Module** to physically manipulate memory bits directly in RAM.
* **Sim Mode:** Uses a mathematical decay algorithm for cross-platform compatibility (Windows/macOS).

## ✨ Features

* **💀 Self-Destructing Data:** Information that automatically turns into noise over time.
* **🧠 Organic Security:** Implements "Forward Secrecy" by default. Old data cannot be stolen because it no longer exists.
* **🔌 Hybrid Architecture:** Seamlessly switches between Kernel Mode (hardware-level) and Simulation Mode (software-level).

## 🚀 Installation

```bash
git clone [https://github.com/YOUR_NAME/py-entropy.git](https://github.com/YOUR_NAME/py-entropy.git)
cd py-entropy
pip install .
```

To enable **True Entropy Mode (Linux only)**, you must compile and insert the kernel module:

```bash
cd kernel_module
make
sudo insmod entropy_mem.ko
sudo chmod 666 /dev/entropy_mem
```

## 📖 Usage

### Basic Usage

```python
from entropy import DecayString

secret = DecayString("This is a top secret message.")

import time
from entropy import DecayString

# Define a variable that rots over time
# If the kernel module is loaded, this uses physical RAM manipulation.
secret = DecayString("This is a top secret message.")

print(f"Original: {secret}")

# Wait for entropy to do its work...
print("Waiting for decay...")
time.sleep(5)

# The data is now corrupted
print(f"5s Later: {secret}") 
# Output example: "Th%s is @ tOp s#crXt mXs?age."
```

## ⚠️ Disclaimer (Must Read)

**THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.**

- **Experimental Nature:** This library includes a **Linux Kernel Module** that directly manipulates memory addresses. While designed with safety in mind, it involves operations that are inherently risky.
- **System Stability:** Improper use or unexpected bugs in the kernel module may cause system instability, freezes, or **Kernel Panics (Blue/Black Screen of Death)**.
- **Data Safety:** Do not use this on critical production systems or machines containing unsaved important data.

**The author (and contributors) takes NO RESPONSIBILITY for any damage, data loss, or hardware failure resulting from the use of this software.** **USE AT YOUR OWN RISK.** ☠️