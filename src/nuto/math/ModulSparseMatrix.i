%module(package="nuto") ModulSparseMatrix
%feature("autodoc","1");
%{
//Put headers and other declarations here to be added in the wrapper files
#include "nuto/math/Operator.h"
#include "nuto/math/SparseMatrix.h"
#include "nuto/math/SparseMatrixCSR.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/math/SparseMatrixCSRSymmetric.h"
%}

// convert python string to std::string
%include "std_string.i"
// convert python tuple to std::vector
%include "std_vector.i"
// use exceptions, but build no interface for NUTO::Exception
%ignore Exception;
%include "nuto/base/ModulNuToBase.i"

%import "nuto/math/NuToMath.i"
%import "nuto/math/ModulMatrix.i"

%include "nuto/math/SparseMatrix.h"
%include "nuto/math/SparseMatrixCSR.h"
%include "nuto/math/SparseMatrixCSRGeneral.h"
%include "nuto/math/SparseMatrixCSRSymmetric.h"
%template(DoubleSparseMatrix) NuTo::SparseMatrix<double>;
%template(IntSparseMatrix) NuTo::SparseMatrix<int>;
%template(DoubleSparseMatrixCSR) NuTo::SparseMatrixCSR<double>;
%template(IntSparseMatrixCSR) NuTo::SparseMatrixCSR<int>;
%template(DoubleSparseMatrixCSRGeneral) NuTo::SparseMatrixCSRGeneral<double>;
%template(IntSparseMatrixCSRGeneral) NuTo::SparseMatrixCSRGeneral<int>;
%template(DoubleSparseMatrixCSRSymmetric) NuTo::SparseMatrixCSRSymmetric<double>;
%template(IntSparseMatrixCSRSymmetric) NuTo::SparseMatrixCSRSymmetric<int>;