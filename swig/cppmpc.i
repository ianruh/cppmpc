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
} CRCPBasic;


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

// Note that, as mentioned [here](http://www.swig.org/Doc2.0/SWIGPlus.html#SWIGPlus_nn18)
// SWIG converts references to pointers, so the types of $1 below is actually a
// pointer.

// Python --> C
%typemap(in) const RCP<const Basic>& {
    //TODO(iruh): This leaks memory. Not sure when I can delete it though, as
    // it needs to still be valid when the pointer to the RCP gets dereferenced.
    CRCPBasic* rcp_basic = new CRCPBasic;
    PyObject* capsule = PyCapsule_New(rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(2, capsule, $input);

    PyObject* call_result = PyObject_CallObject(assign_to_capsule_func, args);

    $1 = &rcp_basic->m;
}

// C --> Python
%typemap(out) const RCP<const Basic>& {
    // Check how this impacts memory management, same as above.
    CRCPBasic* rcp_basic;
    rcp_basic = (CRCPBasic*)$1;
    PyObject* capsule = PyCapsule_New(rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(1, capsule);

    PyObject* basic = PyObject_CallObject(capsule_to_basic_func, args);

    $result = basic;
}

%{
#include "SymbolicObjective.hpp"
#include "SymEngineUtilities.hpp"
%}

%include "SymbolicObjective.hpp"
%include "SymEngineUtilities.hpp"
