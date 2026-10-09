from setuptools import setup
setup(
    name="nanocam",
    version="0.1.0",
    description="ESP32-NanoCam Python Library",
    author="Juxi Technology",
    py_modules=["nanocam"],
    install_requires=["pyserial"],
    python_requires=">=3.7",
)
