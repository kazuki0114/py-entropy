from __future__ import annotations

import random
import time
from dataclasses import dataclass
from typing import Optional


def _printable_ascii(rng: random.Random) -> str:
    # Printable ASCII range: 33..126
    return chr(rng.randint(33, 126))


@dataclass
class SimulatedDecay:
    """
    Simulation backend.

    Design goals (v0.1.0a0):
    - No external dependencies
    - Stable behavior: for the same elapsed second, the same corruption pattern is produced
      (helps testing / demos).
    - Cumulative decay: after N seconds, N chars (max len) are corrupted.
    """

    _original: list[str]
    _start: float
    _salt: int
    _closed: bool = False
    _last_applied: int = 0
    _state: Optional[list[str]] = None

    def __init__(self, content: str) -> None:
        self._original = list(content)
        self._start = time.monotonic()
        # Salt based on content + monotonic start to avoid identical patterns across instances.
        self._salt = hash((content, int(self._start * 1_000_000))) & 0xFFFFFFFF
        self._state = self._original[:]

    def read(self) -> str:
        if self._closed:
            return ""

        elapsed_sec = int(time.monotonic() - self._start)
        length = len(self._original)
        if length == 0:
            return ""

        target = min(elapsed_sec, length)
        if self._state is None:
            self._state = self._original[:]

        # Apply only the delta since last read (cumulative decay).
        delta = target - self._last_applied
        if delta > 0:
            # Deterministic RNG based on salt and target value
            rng = random.Random(self._salt ^ target)
            for _ in range(delta):
                idx = rng.randrange(0, length)
                self._state[idx] = _printable_ascii(rng)
            self._last_applied = target

        return "".join(self._state)

    def close(self) -> None:
        self._closed = True
        self._state = None
        self._original = []
