
from setuptools import setup, Distribution, find_packages

class BinaryDistribution(Distribution):
      def has_ext_modules(self):
            return True

setup(name='iroha-hl',
      version='{{ PYPI_VERSION }}',
      description='Python library for Hyperledger Iroha',
      url='http://github.com/hyperledger/iroha',
      author='Soramitsu Co Ltd',
      author_email='ci@soramitsu.co.jp',
      license='Apache-2.0',
      packages=find_packages(exclude=['dist','build','*.pyc', '.DS_Store', '.vscode', '__pycache__', '*.bak']),
      package_data={
            'iroha': ['_iroha.exp', '_iroha.lib', '_iroha.pyd', '_iroha.pyd.manifest', 'bindings.lib', 'libprotobuf.dll', '_iroha.so', '*.py']
      },
      install_requires=[
            'protobuf>=3.5.2.post1',
            'grpcio>=1.12.1'
      ],
      dependency_links=[
        "https://pypi.org/simple"
      ],
      classifiers=[
            'License :: OSI Approved :: Apache Software License',
            'Development Status :: 4 - Beta',
            'Programming Language :: Python',
            'Operating System :: OS Independent',
            'Programming Language :: Python :: 2.7',
            'Programming Language :: Python :: 3.5'
      ],
      project_urls={
        "Jenkins": "https://jenkins.soramitsu.co.jp/",
        "Nexus": "https://nexus.soramitsu.co.jp/",
        "Documentation": "https://iroha.readthedocs.io/en/latest/",
        "Doxygen": "https://docs/iroha.tech/",
        "Source Code": "https://github.com/hyperledger/iroha",
      },
      zip_safe=False,
      distclass=BinaryDistribution
)
