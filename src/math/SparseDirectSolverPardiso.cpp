#include "math/SparseDirectSolverPardiso.h"

#include "base/Timer.h"
#include "base/Exception.h"
#include "math/SparseMatrixCSR.h"
#include "math/SparseDirectSolver.h"


/* PARDISO prototype */
extern "C" void pardisoinit(void*, int*, int*, int*, double*, int*);
extern "C" void pardiso(void*, int*, int*, int*, int*, int*, double*, int*, int*, int*, int*, int*, int*, double*,
                        double*, int*, double*);
extern "C" void pardiso_chkmatrix(int*, int*, double*, int*, int*, int*);
extern "C" void pardiso_chkvec(int*, int*, double*, int*);
extern "C" void pardiso_printstats(int*, int*, double*, int*, int*, int*, double*, int*);

NuTo::SparseDirectSolverPardiso::SparseDirectSolverPardiso(int rNumThreads, int rVerboseLevel)
    : SparseDirectSolver()
    , mVerboseLevel(rVerboseLevel)
{
#ifdef HAVE_PARDISO
    this->mNumThreads = rNumThreads; // set the number of threads
    // set default solver parameters
    this->mOrderingType = 2; // set ordering to METIS
    this->mNumRefinementSteps = 0; // maximum number of iterative refinement steps
    this->mPivotingPerturbation = -1; // set pivoting perturbation to default values
    this->mScaling = -1; // enable nonsymmetric permutation and mScaling MPS only for unsymmetric matrices (MKL default)
    this->mWeightedMatching =
            -1; // enable maximum weighted matching algorithm only for unsymmetric matrices (MKL default)
    this->mSolver = 0; // 0: use direct solver, 1: use iterative solver
#else // HAVE_PARDISO
    throw NuTo::Exception("Pardiso-solver was not found on your system (check cmake)");
#endif // HAVE_PARDISO
}

#ifdef HAVE_PARDISO
void NuTo::SparseDirectSolverPardiso::Solve(const NuTo::SparseMatrixCSR<double>& rMatrix, const Eigen::VectorXd& rRhs,
                                            Eigen::VectorXd& rSolution)
{
    Timer timerTotal(std::string("PARDISO ") + __FUNCTION__ + " TOTAL TIME", GetShowTime());
    Timer timer(std::string("PARDISO ") + __FUNCTION__ + " License checking and and initialization", GetShowTime());

    // check rMatrix
    if (rMatrix.HasZeroBasedIndexing())
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "one based indexing of sparse rMatrix is required for this solver.");
    }
    int matrixDimension = rMatrix.GetNumRows();
    if (matrixDimension != rMatrix.GetNumColumns())
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "matrix must be square.");
    }
    const std::vector<int>& matrixRowIndex = rMatrix.GetRowIndex();
    const std::vector<int>& matrixColumns = rMatrix.GetColumns();
    const std::vector<double>& matrixValues = rMatrix.GetValues();
    int matrixType;
    if (rMatrix.IsSymmetric())
    {
        if (rMatrix.IsPositiveDefinite())
        {
            matrixType = 2;
        }
        else
        {
            matrixType = -2;
        }
    }
    else
    {
        matrixType = 11;
    }

    // check right hand side
    if (matrixDimension != rRhs.rows())
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "invalid dimension of right hand side vector.");
    }
    int rhsNumColumns = 1;
    const double* rhsValues = rRhs.data();

    // prepare solution matrix
    rSolution.resize(matrixDimension);
    const double* solutionValues = rSolution.data();

    void* pt[64];
    for (unsigned int count = 0; count < 64; count++)
    {
        pt[count] = 0;
    }
    int maxfct(1); // Maximum number of numerical factorizations.
    int mnum(1); // Which factorization to use.
    int msglvl(this->mVerboseLevel); // Print statistical information in file
    int error(0); // Initialize error flag
    double ddum(0); // Double dummy
    int idum(0); // Integer dummy

    int parameters[64];
    double dparameters[64];

#ifdef _OPENMP
    parameters[2] = mNumThreads;
#else
    parameters[2] = 1;
#endif

    /** checks the current license in the file pardiso.lic and initializes the internal
    timer and the address pointer pt. It sets the solver default values according to the matrix type. **/

    pardisoinit(pt, &matrixType, &mSolver, parameters, dparameters, &error);

    // parallel METIS reordering
    parameters[27] = 1;


    if (error != 0)
    {
        if (error == -10)
            throw NuTo::Exception(__PRETTY_FUNCTION__, "No license file found.");
        if (error == -11)
            throw NuTo::Exception(__PRETTY_FUNCTION__, "License is expired.");
        if (error == -12)
            throw NuTo::Exception(__PRETTY_FUNCTION__, "Wrong username or hostname.");
    }

    timer.Reset(std::string("PARDISO ") + __FUNCTION__ + " reordering and symbolic factorization");

    // Reordering and Symbolic Factorization.
    // This step also allocates all memory that is necessary for the factorization.
    int phase = 11;
    // int a = omp_get_num_threads();
    pardiso(pt, &maxfct, &mnum, &matrixType, &phase, &matrixDimension, const_cast<double*>(&matrixValues[0]),
            const_cast<int*>(&matrixRowIndex[0]), const_cast<int*>(&matrixColumns[0]), &idum, &rhsNumColumns,
            parameters, &msglvl, &ddum, &ddum, &error, dparameters);

    if (error != 0)
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__,
                              "Analysis and reordering phase: " + this->GetErrorString(error) + ".");
    }

    timer.Reset(std::string("PARDISO ") + __FUNCTION__ + " numerical factorization");

    // Numerical factorization.
    phase = 22;
    pardiso(pt, &maxfct, &mnum, &matrixType, &phase, &matrixDimension, const_cast<double*>(&matrixValues[0]),
            const_cast<int*>(&matrixRowIndex[0]), const_cast<int*>(&matrixColumns[0]), &idum, &rhsNumColumns,
            parameters, &msglvl, &ddum, &ddum, &error, dparameters);
    if (error != 0)
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__,
                                  "Numerical factorization phase: " + this->GetErrorString(error) + ".");
    }

    timer.Reset(std::string("PARDISO ") + __FUNCTION__ + " back substitution and iterative refinement");

    // Back substitution and iterative refinement.
    phase = 33;
    pardiso(pt, &maxfct, &mnum, &matrixType, &phase, &matrixDimension, const_cast<double*>(&matrixValues[0]),
            const_cast<int*>(&matrixRowIndex[0]), const_cast<int*>(&matrixColumns[0]), &idum, &rhsNumColumns,
            parameters, &msglvl, const_cast<double*>(rhsValues), const_cast<double*>(solutionValues), &error,
            dparameters);
    if (error != 0)
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__, "Back substitution and iterative refinement phase: " +
                                                               this->GetErrorString(error) + ".");
    }

    timer.Reset(std::string("PARDISO ") + __FUNCTION__ + " termination");


    if (this->mVerboseLevel > 1)
    {
        std::cout << "[SparseDirectSolverPardiso::solve] Peak memory symbolic factorization: " << parameters[14]
                  << " KBytes" << std::endl;
        std::cout << "[SparseDirectSolverPardiso::solve] Permanent memory symbolic factorization: " << parameters[15]
                  << " KBytes" << std::endl;
        std::cout << "[SparseDirectSolverPardiso::solve] Memory numerical factorization and solution: "
                  << parameters[16] << " KBytes" << std::endl;
        if (this->mVerboseLevel > 2)
        {
            std::cout << "[SparseDirectSolverPardiso::solve] Number of floating point operations required for "
                         "factorization: "
                      << parameters[18] << " MFLOS" << std::endl;
            if (matrixType == -2)
            {
                std::cout << "[SparseDirectSolverPardiso::solve] Inertia: number of positive eigenvalues: "
                          << parameters[21] << std::endl;
                std::cout << "[SparseDirectSolverPardiso::solve] Inertia: number of negative eigenvalues: "
                          << parameters[22] << std::endl;
                std::cout << "[SparseDirectSolverPardiso::solve] Inertia: number of zero eigenvalues: "
                          << matrixDimension - parameters[21] - parameters[22] << std::endl;
            }
            std::cout << "[SparseDirectSolverPardiso::solve] Number of nonzeros in factors: " << parameters[17]
                      << std::endl;
            std::cout << "[SparseDirectSolverPardiso::solve] Number of performed iterative refinement steps: "
                      << parameters[6] << std::endl;
            if (matrixType != 2)
            {
                std::cout << "[SparseDirectSolverPardiso::solve] Number of perturbed pivots: " << parameters[13]
                          << std::endl;
            }
        }
    }

    // Termination and release of memory
    phase = -1;
    pardiso(pt, &maxfct, &mnum, &matrixType, &phase, &matrixDimension, &ddum, const_cast<int*>(&matrixRowIndex[0]),
            const_cast<int*>(&matrixColumns[0]), &idum, &rhsNumColumns, parameters, &msglvl, &ddum, &ddum, &error,
            dparameters);
    if (error != 0)
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "Termination phase: " + this->GetErrorString(error) + ".");
    }
}

std::string NuTo::SparseDirectSolverPardiso::GetErrorString(int error) const
{
    assert(error != 0);

    switch (error)
    {
    case -1:
        return "input inconsistent";
    case -2:
        return "not enough memory";
    case -3:
        return "reordering problem";
    case -4:
        return "zero pivot, numerical factorization or iterative refinement problem";
    case -5:
        return "unclassified (internal) error";
    case -6:
        return "preordering failed";
    case -7:
        return "diagonal rMatrix problem";
    case -8:
        return "32-bit integer overflow problem";
    case -9:
        return "not enough memory for OOC";
    case -10:
        return "problems with opening OOC temporary files";
    case -11:
        return "read/write problems with the OOC data file";
    default:
        return "unknown error code";
    }
}
#endif // HAVE_PARDISO
