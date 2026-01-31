from __future__ import annotations

from setuptools import find_packages, setup

setup(
    name="py-entropy",
    version="0.1.0a0",  # v0.1.0 Alpha
    description="Entropy Variables for Python: data decays over time (kernel-backed or simulated).",
    author="Entropy Architect",
    packages=find_packages(),
    python_requires=">=3.9",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Operating System :: POSIX :: Linux",
        "License :: OSI Approved :: MIT License",
        "Development Status :: 3 - Alpha",
        "Topic :: Security",
        "Topic :: System :: Operating System Kernels :: Linux",
    ],
)
