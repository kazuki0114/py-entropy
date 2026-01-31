# tests/test_basic.py
import unittest
import time
from entropy import DecayString, DecayConfig  # DecayConfigを追加インポート

class TestEntropy(unittest.TestCase):
    def test_decay_simulation(self):
        # シミュレーションモードを強制する設定を作成
        conf = DecayConfig(force_simulation=True)
        
        # config引数として渡す
        s = DecayString("Hello World", config=conf)
        
        self.assertEqual(str(s), "Hello World")
        time.sleep(2)
        # 2秒後には変わっているはず（シミュレーションの仕様依存だが、通常は変わる）
        self.assertNotEqual(str(s), "Hello World")

if __name__ == '__main__':
    unittest.main()