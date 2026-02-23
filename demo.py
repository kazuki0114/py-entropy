# demo.py
import time

from entropy import DecayString

# 時間とともに腐敗する変数を定義する
secret = DecayString("This is a top secret message.")

print(f"Original: {secret}")

# エントロピーが仕事をするのを待つ...
print("Waiting for decay...")
time.sleep(5)

# データは破損（腐敗）しました
print(f"5s Later: {secret}")

# repr()を使って、見えないゴミデータも暴き出す
print(f"Raw data: {repr(secret.value)}")