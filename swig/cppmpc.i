%module cppmpc

%include "stl.i"
%include "std_string.i"
%include "std_pair.i"

%{
#include "Python.h"
#include "symengine/basic.h"
#include "stdio.h"


#include "SymbolicObjective.h"
#include "SymEngineUtilities.h"
#include "GetSymbolsVisitor.h"
#include "SymbolicEquality.h"
#include "OrderedSet.h"

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::Symbol;

using cppmpc::OrderedSet;

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

// Macro to construct struct names
%define STRUCT_NAME_(prefix,postfix)
prefix##postfix
%enddef
%define STRUCT_NAME(type)
STRUCT_NAME_(CRCP,type)
%enddef

// Macro to define the  type conversions for RCP<const T> and RCP<const T>&
%define CONVERSION_HELPER(type,name)
%{

// Note that, as mentioned [here](http://www.swig.org/Doc2.0/SWIGPlus.html#SWIGPlus_nn18)
// SWIG converts references to pointers, so the types of $1 below is actually a
// pointer.

// Matching the struct in symengine.py
typedef struct STRUCT_NAME(name) {
    type m;
    
    STRUCT_NAME(name)() = default;
    STRUCT_NAME(name)(type& other): m(other) {}
} STRUCT_NAME(name);
%}
// Python --> C
%typemap(in) type& {
    //TODO(iruh): This leaks memory. Not sure when I can delete it though, as
    // it needs to still be valid when the pointer to the RCP gets dereferenced.
    STRUCT_NAME(name)* rcp_basic = new STRUCT_NAME(name);
    PyObject* capsule = PyCapsule_New(rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(2, capsule, $input);

    PyObject* call_result = PyObject_CallObject(assign_to_capsule_func, args);

    $1 = &rcp_basic->m;
}

// C --> Python
%typemap(out) type& {
    // Check how this impacts memory management, same as above.
    STRUCT_NAME(name)* rcp_basic;
    rcp_basic = (STRUCT_NAME(name)*)$1;
    PyObject* capsule = PyCapsule_New(rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(1, capsule);

    PyObject* basic = PyObject_CallObject(capsule_to_basic_func, args);

    $result = basic;
}

// Python --> C
%typemap(in) type {
    //TODO(iruh): This leaks memory. Not sure when I can delete it though, as
    // it needs to still be valid when the pointer to the RCP gets dereferenced.
    STRUCT_NAME(name) rcp_basic;
    PyObject* capsule = PyCapsule_New(&rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(2, capsule, $input);

    PyObject* call_result = PyObject_CallObject(assign_to_capsule_func, args);

    $1 = rcp_basic.m;
}

// C --> Python
%typemap(out) type {
    // Check how this impacts memory management, same as above.
    STRUCT_NAME(name) rcp_basic = (STRUCT_NAME(name))($1);
    PyObject* capsule = PyCapsule_New(&rcp_basic, NULL, NULL);
    PyObject* args = PyTuple_Pack(1, capsule);

    PyObject* basic = PyObject_CallObject(capsule_to_basic_func, args);

    $result = basic;
}
%enddef

//==========================================================================
// This block needs to go before the the %includes below.

// Create all the types we want to convert
CONVERSION_HELPER(RCP<const Basic>,Basic);
CONVERSION_HELPER(SymEngine::RCP<const SymEngine::Basic>,SymEngineBasic);

CONVERSION_HELPER(RCP<const Symbol>,Symbol);
CONVERSION_HELPER(SymEngine::RCP<const SymEngine::Symbol>,SymEngineSymbol);

//%typemap(in) cppmpc::OrderedSet = OrderedSet;
//%typemap(in) cppmpc::OrderedSet & = OrderedSet;
//%typemap(in) OrderedSet const & = OrderedSet;

//==========================================================================


%include "SymbolicObjective.h"
%include "SymEngineUtilities.h"
%include "GetSymbolsVisitor.h"
%include "OrderedSet.h"

//==========================================================================

%ignore SymbolicEqualityConstraints::convertToLinearSystem(const OrderedSet&) const;

%include "SymbolicEquality.h"

%extend SymbolicEqualityConstraints
{    
    std::pair<SymEngine::DenseMatrix, SymEngine::DenseMatrix>
            SymbolicEqualityConstraints::convertToLinearSystem(
            OrderedSet variableOrdering) const {
        return this->convertToLinearSystem(variableOrdering);
    }
}

//==========================================================================
