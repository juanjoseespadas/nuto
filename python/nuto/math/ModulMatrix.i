%module ModulMatrix
%feature("autodoc","1");
//remove the warning for not exposing the base class features of eigen to python via swig
#pragma SWIG nowarn=401
%{
//Put headers and other declarations here to be added in the wrapper files
#include "math/Operator.h"
#include "math/Matrix.h"
%}

//declare inputs of the functions to be used as output on python level
%include "typemaps.i"

// rename the full version including rows and columns
%rename(MaxFull)Max(int& OUTPUT, int& OUTPUT,T& OUTPUT);
%rename(MinFull)Min(int& OUTPUT, int& OUTPUT,T& OUTPUT);

//attention - first rename, and then apply
%apply double *OUTPUT { double& rResultOutput};
%apply int *OUTPUT { int& rRowOutput, int& rColumnOutput, int& rResultOutput};


// convert python string to std::string
%include "std_string.i"
// convert python tuple to std::vector
%include "std_vector.i"
namespace std {
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
}
// use exceptions, but build no interface for NUTO::Exception
%ignore Exception;
%include "base/ModulNuToBase.i"

%import "math/NuToMath.i"

%include "math/Matrix.h"
%template(DoubleMatrix) NuTo::Matrix<double>;
%template(IntMatrix) NuTo::Matrix<int>;
