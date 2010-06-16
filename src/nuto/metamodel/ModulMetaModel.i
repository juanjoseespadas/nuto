%module(package="nuto") ModulMetaModel
%feature("autodoc","1");
%{
//Put headers and other declarations here to be added in the wrapper files
#include "nuto/metamodel/NeuralNetwork.h"
%}

// convert python string to std::string
%include "std_string.i"
// convert python tuple to std::vector
%include "std_vector.i"
%ignore Exception;
%include "nuto/base/ModulNuToBase.i"

%include "nuto/metamodel/Metamodel.h"
%include "nuto/metamodel/NeuralNetwork.h"