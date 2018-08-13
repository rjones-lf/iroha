import multiprocessing
import os
import shutil
import subprocess
import sys
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

IROHA_REPO = "https://github.com/hyperledger/iroha"
IROHA_BRANCH = "develop"

IROHA_CMAKE_ARGS = dict(
    SWIG_PYTHON="ON",
)

if sys.version_info[0] == 2:
    IROHA_CMAKE_ARGS["SUPPORT_PYTHON2"] = "ON"

class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        for ext in self.extensions:
            self.build_extension(ext)

    def clone(self):
        if not os.path.isdir("iroha"):
            subprocess.check_call('git clone {} -b {} --depth 1'.format(IROHA_REPO, IROHA_BRANCH).split())

    def build_extension(self, ext):
        self.clone()
        cfg = 'Debug' if self.debug else 'Release'

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        IROHA_CMAKE_ARGS["CMAKE_LIBRARY_OUTPUT_DIRECTORY"] = extdir
        IROHA_CMAKE_ARGS["PYTHON_EXECUTABLE"] = sys.executable
        IROHA_CMAKE_ARGS["CMAKE_BUILD_TYPE"] = cfg

        cmake_args = []
        for key, value in IROHA_CMAKE_ARGS.items():
            cmake_args.append("-D{}={}".format(key,value))

        build_args = '--config {} --target irohapy -- -j{}'.format(cfg, multiprocessing.cpu_count()).split(' ')

        env = os.environ.copy()

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)
        shutil.copy(self.build_temp+"/shared_model/bindings/iroha.py", extdir+"/")

if __name__ == "__main__":
    setup(
        name='iroha',
        version='0.0.23',
        author='Soramitsu Co Ltd',
        author_email='savva@soramitsu.co.jp',
        description='Python library for Hyperledger Iroha',
        ext_modules=[CMakeExtension('iroha', 'iroha')],
        cmdclass=dict(build_ext=CMakeBuild),
        zip_safe=False,
    )
