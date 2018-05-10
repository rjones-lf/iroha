# Python client library example
***Please read `setup.py` and take a look at the branch that are you going to use.***

## How to install iroha python library
To install latest release:
```bash
pip install iroha
```
To install developer version:
```bash
pip install -i https://testpypi.python.org/pypi iroha
```

## How to build and install iroha locally
```bash
python setup.py install
```

## How to build and publish iroha
Creating MacOS wheel and publishing it:
```bash
python setup.py bdist_wheel
twine upload --repository-url https://pypi.org/legacy/ dist/*
```
Creating Linux source distribution and publishing it:
```bash
python setup.py sdist
twine upload --repository-url https://pypi.org/legacy/ dist/*
```
