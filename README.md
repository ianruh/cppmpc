# CPP MPC

**Setup**
`mamba create -n cppmpc -c conda-forge cmake xeus-cling cling=0.8 eigen gmp gcc_linux-64`

**Dev Dependencies**
- fd
- clang-format
- black
- cpplint
- ripgrep
- fzf

**Python Wrappers Setup**

Because of reasons, the [symengine.py]() python wrapper for symengine only
supports the symengine commit hash listed in `symengine.py/symengine_version.txt`.
So, the setup procedure needs to look something like this:

1. Clone symengine, and checkout the hash listed in `symengine.py/symengine_version.txt`
2. Build and install symengine
3. Clone symengine.py
4. Run `python setup.py install` to compile the cython extension and install
   the wrappers.
5. Shed a tear for the two days I spent debugging before making sure the versions
   were matched.
6. Build cppmpc
7. Run `pip install -e cppmpc_py/` if developing, otherwise just
   `pip install cppmpc_py/`
7. Run the python tests to make sure everything is hunky-dory.

**Compilation Options**
- `NO_VALIDATE_OBJECTIVE` Don't check the dimensions of the objective before 
  solving the problem. This can speed up the initialization time for the solver
