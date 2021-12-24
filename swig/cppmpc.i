%module cppmpc

%include "stl.i"
%include "std_string.i"

%{
#include "Python.h"
#include "symengine/basic.h"
#include "stdio.h"

using SymEngine::RCP;
using SymEngine::Basic;

// Matching the struct in symengine.py
typedef struct {
    RCP<const Basic> m;
} CRPCBasic;


PyObject* wrapper_module_string = PyString_FromString(
    (char*)"symengine.lib.symengine_wrapper");
PyObject* wrapper_module = PyImport_Import(wrapper_module_string);
PyObject* assign_to_capsule_func = PyObject_GetAttrString(
    wrapper_module,
    (char*)"assign_to_capsule");
PyObject* capsule_to_basic_func = PyObject_GetAttrString(
    wrapper_module,
    (char*)"capsule_to_basic");
%}

using SymEngine::RCP;
using SymEngine::Basic;

// Python --> C
%typemap(in) RCP<const Basic>& {
    CRPCBasic rcp_basic;
    PyObject* capsule = PyCapsule_New(&rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(2, capsule, $input);

    PyObject* call_result = PyObject_CallObject(assign_to_capsule_func, args);

    $1 = &rcp_basic.m;
}

// C --> Python
%typemap(out) RCP<const Basic>& {
    // Check how this impacts memory management
    CRCPBasic rcp_basic;
    rcp_basic.m = $1;
    PyObject* capsule = PyCapsule_New(&rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(1, capsule);

    PyObject* basic = PyObject_CallObject(capsule_to_basic, args);

    $result = basic;
}

%{
#include "SymbolicObjective.hpp"
%}

%include "SymbolicObjective.hpp"
