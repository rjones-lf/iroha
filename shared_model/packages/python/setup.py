import os
import re
import sys
import platform
import subprocess

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion
import shutil
import multiprocessing

IROHA_REPO = "https://github.com/hyperledger/iroha"
IROHA_BRANCH = "develop"

IROHA_CMAKE_ARGS = dict(
    SWIG_PYTHON="ON",
)
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

        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + extdir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]


        cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
        for key, value in IROHA_CMAKE_ARGS.iteritems():
            cmake_args.append("-D{}={}".format(key,value))

        build_args += '--target irohapy -- -j{}'.format(multiprocessing.cpu_count()).split(' ')

        env = os.environ.copy()

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)
        shutil.copy(self.build_temp+"/shared_model/bindings/iroha.py", extdir+"/")

if __name__ == "__main__":
    setup(
        name='iroha',
        version='0.0.18',
        author='Soramitsu Co Ltd',
        author_email='savva@soramitsu.co.jp',
        description='Python library for Hyperledger Iroha',
        ext_modules=[CMakeExtension('iroha', 'iroha')],
        cmdclass=dict(build_ext=CMakeBuild),
        zip_safe=False,
    )
