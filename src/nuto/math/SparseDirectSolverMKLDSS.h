// $Id$

#ifndef SPARSE_DIRECT_SOLVER_MKLDSS_H
#define SPARSE_DIRECT_SOLVER_MKLDSS_H

#include "nuto/math/SparseDirectSolver.h"

namespace NuTo
{
// forward declarations
template<class T> class SparseMatrixCSR;
template<class T> class FullMatrix;

//! @author Stefan Eckardt, ISM
//! @date October 2009
//! @brief ... interface for the MKL-DSS sparse direct solver
class SparseDirectSolverMKLDSS  : public SparseDirectSolver
{
public:
    //! @brief ... default constructor
    SparseDirectSolverMKLDSS();

    //! @brief ... print information about the class attributes
    virtual void Info()
    {
    }
#ifdef HAVE_MKL_DSS
    //! @brief ... solve the system of equations
    //! @param rMatrix ... sparse coefficient matrix stored in the CSR format
    //! @param rRhs ... matrix of right-hand-side vectors (full storage)
    //! @param rSolution ... matrix of solution vectors (full storage)
    void Solve(const SparseMatrixCSR<double>& rMatrix, const FullMatrix<double>& rRhs, FullMatrix<double>& rSolution);

    //! @brief ... enable refinement in the solution stage
    inline void enableRefinement()
    {
        this->mRefinement = true;
    }

    //! @brief ... disable refinement in the solution stage
    inline void DisableRefinement()
    {
        this->mRefinement = false;
    }
protected:
    //! @brief ... determines wether a refinement is enabled or disabled within the solution strategy
    bool mRefinement;
#endif // HAVE_MKL_DSS
};
}
#endif // SPARSE_DIRECT_SOLVER_MKLDSS_H