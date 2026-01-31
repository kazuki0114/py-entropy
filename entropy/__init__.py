from __future__ import annotations

import os
from dataclasses import dataclass
from typing import Optional

from .simulation import SimulatedDecay

DEVICE_PATH_DEFAULT = "/dev/entropy_mem"


class EntropyDeviceError(RuntimeError):
    """Kernel device interaction failed."""


@dataclass(frozen=True)
class DecayConfig:
    """
    Configuration for DecayString.

    device_path:
        Path to kernel device (True Mode).
    force_simulation:
        If True, always use Sim Mode even if device exists.
    """
    device_path: str = DEVICE_PATH_DEFAULT
    force_simulation: bool = False


class DecayString:
    """
    A string that decays over time.

    - True Mode: if /dev/entropy_mem is available and accessible, uses kernel module.
    - Sim Mode: otherwise falls back to simulation.

    Notes on encoding:
      Kernel mode operates on bytes. If you write non-ASCII UTF-8, decayed bytes may become invalid UTF-8.
      This wrapper decodes reads with errors='replace' to avoid exceptions.
    """

    def __init__(self, content: str, *, config: DecayConfig | None = None) -> None:
        self._config: DecayConfig = config or DecayConfig()
        self._use_kernel: bool = False
        self._sim: Optional[SimulatedDecay] = None
        self._fd: Optional[int] = None

        if not self._config.force_simulation and os.path.exists(self._config.device_path):
            try:
                # O_RDWR: allow both write initial content and later "clear" on close.
                self._fd = os.open(self._config.device_path, os.O_RDWR)
                self._write_kernel(content)
                self._use_kernel = True
                return
            except PermissionError:
                # Permission is a normal operational scenario -> fallback to Sim Mode.
                self._close_fd()
            except OSError:
                # Any OS-level failure -> fallback to Sim Mode.
                self._close_fd()

        # Simulation fallback
        self._sim = SimulatedDecay(content)

    # -------------------------
    # Public API
    # -------------------------
    @property
    def value(self) -> str:
        """Get current (possibly decayed) value."""
        if self._use_kernel:
            return self._read_kernel()
        assert self._sim is not None
        return self._sim.read()

    def is_real(self) -> bool:
        """True if using kernel (True Mode)."""
        return self._use_kernel

    def close(self) -> None:
        """
        Explicitly finalize.

        In kernel mode: overwrite device with empty content (clears storage in our module).
        In sim mode: drop internal content.
        """
        if self._use_kernel and self._fd is not None:
            try:
                self._write_kernel("")  # clear
            finally:
                self._close_fd()
            self._use_kernel = False
            return

        if self._sim is not None:
            self._sim.close()
            self._sim = None

    # Context manager support (便利なので実装)
    def __enter__(self) -> "DecayString":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def __str__(self) -> str:
        return self.value

    def __repr__(self) -> str:
        mode = "kernel" if self._use_kernel else "sim"
        return f"<DecayString mode={mode} value={self.value!r}>"

    def __del__(self) -> None:
        # Best-effort cleanup (do not raise)
        try:
            self.close()
        except Exception:
            pass

    # -------------------------
    # Kernel backend helpers
    # -------------------------
    def _write_kernel(self, content: str) -> None:
        if self._fd is None:
            raise EntropyDeviceError("Kernel device is not open.")

        data = content.encode("utf-8", errors="strict")
        try:
            # Reset file offset for predictable semantics.
            os.lseek(self._fd, 0, os.SEEK_SET)
            os.write(self._fd, data)
        except OSError as e:
            raise EntropyDeviceError(f"Kernel write failed: {e}") from e

    def _read_kernel(self) -> str:
        if self._fd is None:
            raise EntropyDeviceError("Kernel device is not open.")

        try:
            os.lseek(self._fd, 0, os.SEEK_SET)
            raw = os.read(self._fd, 4096)
            return raw.decode("utf-8", errors="replace")
        except OSError as e:
            # If kernel read fails at runtime, we return a safe marker.
            return f"<Kernel Error: {e}>"

    def _close_fd(self) -> None:
        if self._fd is not None:
            try:
                os.close(self._fd)
            except Exception:
                pass
            self._fd = None


__all__ = [
    "DecayString",
    "DecayConfig",
    "SimulatedDecay",
    "EntropyDeviceError",
]
