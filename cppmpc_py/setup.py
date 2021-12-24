from distutils.core import setup

setup (name = 'cppmpc',
       version = '0.0.0',
       author = "Ian Ruh",
       description = """Install precompiled extension""",
       py_modules = ["cppmpc"],
       packages=[''],
       package_data={'': ['_cppmpc_swig.so']},
       )
