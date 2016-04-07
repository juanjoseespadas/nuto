// $Id$

#include "nuto/base/Timer.h"
#include "nuto/math/MathException.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/FullVector.h"
#include "nuto/math/SparseMatrixCSR.h"
#include "nuto/math/SparseDirectSolver.h"
#include "nuto/math/SparseDirectSolverMUMPS.h"

NuTo::SparseDirectSolverMUMPS::SparseDirectSolverMUMPS() : SparseDirectSolver()
{
#ifdef HAVE_MUMPS
    // set default solver parameters
    // this->orderingType = 2;          // set ordering to METIS
#else // HAVE_MUMPS
    throw NuTo::MathException(__PRETTY_FUNCTION__, "MUMPS-solver was not found on your system (check cmake)");
#endif // HAVE_MUMPS
}

#ifdef HAVE_MUMPS
void NuTo::SparseDirectSolverMUMPS::Solve(const NuTo::SparseMatrixCSR<double>& rMatrix, const NuTo::FullVector<double,Eigen::Dynamic>& rRhs, NuTo::FullVector<double,Eigen::Dynamic>& rSolution)
{
    Timer timer(std::string("MUMPS ") + __FUNCTION__, GetShowTime());

    Factorization(rMatrix);
    Solution(rRhs, rSolution);
    CleanUp();
}


//! @brief ... prepare the solver, and perform all the steps up to the factorization of the matrix
//! @param rMatrix ... sparse coefficient matrix, stored in compressed CSR format (input)
void NuTo::SparseDirectSolverMUMPS::Factorization(const NuTo::SparseMatrixCSR<double>& rMatrix)
{
    Timer timer(std::string("MUMPS ") + __FUNCTION__ + " reordering and symbolic factorization", GetShowTime());

	// check rMatrix
	if (rMatrix.HasZeroBasedIndexing())
	{
		throw NuTo::MathException(__PRETTY_FUNCTION__, "one based indexing of sparse matrix is required for this solver.");
	}
	int matrixDimension = rMatrix.GetNumRows();
	if (matrixDimension != rMatrix.GetNumColumns())
	{
		throw NuTo::MathException(__PRETTY_FUNCTION__, "matrix is not square.");
	}
	const std::vector<int>& matrixRowIndex = rMatrix.GetRowIndex();
	// extract rows from rowIndex
	std::vector<int> matrixRows(rMatrix.GetNumEntries());
	int entryCount = 0;
	for (int rowCount = 0; rowCount < matrixDimension; rowCount++)
	{
		for (int indexCount = matrixRowIndex[rowCount]; indexCount < matrixRowIndex[rowCount + 1]; indexCount++)
		{
			assert(entryCount < rMatrix.GetNumEntries());
			matrixRows[entryCount] = rowCount + 1;
			entryCount++;
		}
	}
	const std::vector<int>& matrixColumns = rMatrix.GetColumns();
	const std::vector<double>& matrixValues = rMatrix.GetValues();

	// initialize solver data
	// set MPI communicator (also required in sequential version)
	mSolver.comm_fortran = -987654;
	// host is involved in factorization and rSolution phase
	mSolver.par = 1;
	// set rMatrix type
	if (rMatrix.IsSymmetric())
	{
		if (rMatrix.IsPositiveDefinite())
		{
			mSolver.sym = 1;
		}
		else
		{
			mSolver.sym = 2;
		}
	}
	else
	{
		mSolver.sym = 0;
	}
	// initialize solver
	mSolver.job = -1;
	dmumps_c(&mSolver);

	// define the problem
	mSolver.n   = matrixDimension;                       // dimension
	mSolver.nz  = rMatrix.GetNumEntries();                // number of nonzero entries
	mSolver.irn = &matrixRows[0];                        // rows
	mSolver.jcn = const_cast<int*>(&matrixColumns[0]);   // columns
	mSolver.a   = const_cast<double*>(&matrixValues[0]); // values
	mSolver.rhs = 0;                                     // right hand side vector is set befor solution
	// define mSolver specific parameters
	mSolver.icntl[0] = 0; // output stream for error messages
	mSolver.icntl[1] = 0; // output stream for diagnostic printing, statistics, and warning messages
	mSolver.icntl[2] = 0; // output stream for global information
	mSolver.icntl[3] = 0; // level of printing for error, warning, and diagnostic messages (0..4)
	mSolver.icntl[5] = 7; //  control an option for permuting and/or scaling the rMatrix (7 - automaticchoice)
	mSolver.icntl[6] = 7; // determines the pivot order to be used for the factorization (7 -  automatic choice)
	// set scaling strategy
	if (mSolver.sym == 0)
	{
		mSolver.icntl[7] = 7; // simultaneous row and colum iterative scaling
	}
	else
	{
		mSolver.icntl[7] = 1; // diagonal scaling
	}

	// analysis phase
	mSolver.job = 1;
	dmumps_c(&mSolver);
	if (mSolver.info[0] < 0)
	{
		throw NuTo::MathException(__PRETTY_FUNCTION__, "Analysis and reordering phase: " + this->GetErrorString(mSolver.info[0]) + ".");
	}

	timer.Reset(std::string("MUMPS ") + __FUNCTION__ + " numerical factorization");

	// Numerical factorization.
	mSolver.job = 2;
	dmumps_c(&mSolver);
	if (mSolver.info[0] < 0)
	{
		switch (mSolver.info[0])
		{
		case -10:
			std::cout << "numerically singular matrix."  << std::endl;
			break;
		default:
			break;
		}
		throw NuTo::MathException(__PRETTY_FUNCTION__, "Numerical factorization phase: " + this->GetErrorString(mSolver.info[0]) + ".");
	}

}

//! @brief ... use the factorized matrix for the final solution phase
//! @param rMatrix ... sparse coefficient matrix, stored in compressed CSR format (input)
//! @param rRhs ... matrix storing the right-hand-side vectors (input)
//! @param rSolution ... matrix storing the corresponding solution vectors (output)
void NuTo::SparseDirectSolverMUMPS::Solution(const NuTo::FullVector<double,Eigen::Dynamic>& rRhs, NuTo::FullVector<double,Eigen::Dynamic>& rSolution)
{
    Timer timer(std::string("MUMPS ") + __FUNCTION__, GetShowTime());

	// check right hand side
	if (mSolver.n != rRhs.GetNumRows())
	{
		std::cout << "n " << mSolver.n <<  "dim rhs " << rRhs.GetNumRows() << std::endl;
		throw NuTo::MathException(__PRETTY_FUNCTION__, "invalid dimension of right hand side vector.");
	}
	int rhsNumColumns = rRhs.GetNumColumns();

	// prepare rSolution rMatrix (copy rMatrix of right hand side vectors)
	rSolution = rRhs;

	const double *solutionValues = rSolution.data();
	// solution
	mSolver.job = 3;
	for (int rhsCount = 0; rhsCount < rhsNumColumns; rhsCount++)
	{
		mSolver.rhs = const_cast<double*>(&solutionValues[rhsCount*mSolver.n]);
		dmumps_c(&mSolver);
		if (mSolver.info[0] < 0)
		{
			throw NuTo::MathException(__PRETTY_FUNCTION__, "Solution phase: " + this->GetErrorString(mSolver.info[0]) + ".");
		}
	}
}


//! @brief ... Termination and release of memory
void NuTo::SparseDirectSolverMUMPS::CleanUp()
{
    Timer timer(std::string("MUMPS ") + __FUNCTION__, GetShowTime());
    // Termination and release of memory
    mSolver.job = -2;
    dmumps_c(&mSolver);
    if (mSolver.info[0] < 0)
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__, "Termination phase: " + this->GetErrorString(mSolver.info[0]) + ".");
    }
}

void NuTo::SparseDirectSolverMUMPS::SchurComplement(const NuTo::SparseMatrixCSR<double>& rMatrix, NuTo::FullVector<int, Eigen::Dynamic> rSchurIndices, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rSchurComplement)
{
    Timer timer(std::string("MUMPS ") + __FUNCTION__ + " reordering and symbolic factorization", GetShowTime());

    // check rMatrix
    if (rMatrix.HasZeroBasedIndexing())
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__, "one based indexing of sparse matrix is required for this solver.");
    }
    int matrixDimension = rMatrix.GetNumRows();
    if (matrixDimension != rMatrix.GetNumColumns())
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__, "matrix is not square.");
    }
    const std::vector<int>& matrixRowIndex = rMatrix.GetRowIndex();
    // extract rows from rowIndex
    std::vector<int> matrixRows(rMatrix.GetNumEntries());
    int entryCount = 0;
    for (int rowCount = 0; rowCount < matrixDimension; rowCount++)
    {
        for (int indexCount = matrixRowIndex[rowCount]; indexCount < matrixRowIndex[rowCount + 1]; indexCount++)
        {
            assert(entryCount < rMatrix.GetNumEntries());
            matrixRows[entryCount] = rowCount + 1;
            entryCount++;
        }
    }
    const std::vector<int>& matrixColumns = rMatrix.GetColumns();
    const std::vector<double>& matrixValues = rMatrix.GetValues();

    // check Schur Indices
    if (matrixDimension < rSchurIndices.GetNumRows() || rSchurIndices.GetNumColumns()!=1)
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__, "invalid dimension of schur indices vector.");
    }

    // add indices by one, since the external interface always counts from zero and mumps requires one based indexing
    rSchurIndices+=1;

    // initialize solver data
    // set MPI communicator (also required in sequential version)
    mSolver.comm_fortran = -987654;
    // host is involved in factorization and rSolution phase
    mSolver.par = 1;
    // set rMatrix type
    if (rMatrix.IsSymmetric())
    {
        if (rMatrix.IsPositiveDefinite())
        {
            mSolver.sym = 1;
        }
        else
        {
            mSolver.sym = 2;
        }
    }
    else
    {
        mSolver.sym = 0;
    }
    // initialize mSolver
    mSolver.job = -1;
    dmumps_c(&mSolver);

    //resize result matrix
    NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic> rSchurComplementTranspose(rSchurIndices.GetNumRows(),rSchurIndices.GetNumRows());

    // define the problem
    mSolver.n   = matrixDimension;                       // dimension
    mSolver.nz  = rMatrix.GetNumEntries();               // number of nonzero entries
    mSolver.irn = &matrixRows[0];                        // rows
    mSolver.jcn = const_cast<int*>(&matrixColumns[0]);   // columns
    mSolver.a   = const_cast<double*>(&matrixValues[0]); // values
    mSolver.rhs = 0;
    mSolver.size_schur = rSchurIndices.GetNumRows();
    mSolver.listvar_schur = const_cast<int*>(rSchurIndices.data()); //variables of the schur matrix (the 1..size_schur)
    mSolver.schur = rSchurComplementTranspose.data();

    // define mSolver specific parameters
    mSolver.icntl[0] = 0; // output stream for error messages
    mSolver.icntl[1] = 0; // output stream for diagnostic printing, statistics, and warning messages
    mSolver.icntl[2] = 0; // output stream for global information
    mSolver.icntl[3] = 0; // level of printing for error, warning, and diagnostic messages (0..4)
    mSolver.icntl[5] = 0; // control an option for permuting and/or scaling the rMatrix (0 - no scaling due to Schur)
    mSolver.icntl[6] = 7; // determines the pivot order to be used for the factorization (7 -  automatic choice)
    mSolver.icntl[7] = 77; // set scaling strategy to automatic
    //mSolver.icntl[13] = 1000; // additional fill in (1000%, standard is 20%) this might indicate that you have not removed the almost zero entries of your matrix
    mSolver.icntl[18] = 1; // centralized Schur complement

    // analysis phase
    mSolver.job = 1;
    dmumps_c(&mSolver);
    if (mSolver.info[0] < 0)
    {
        throw NuTo::MathException(__PRETTY_FUNCTION__, "Analysis and reordering phase: " + this->GetErrorString(mSolver.info[0]) + ".");
    }

    timer.Reset(std::string("MUMPS ") + __FUNCTION__ + " numerical factorization");

    // Numerical factorization.
    mSolver.job = 2;
    dmumps_c(&mSolver);
    if (mSolver.info[0] < 0)
    {
        std::cout << "mSolver info " << mSolver.info[0] << "\n";
    	throw NuTo::MathException(__PRETTY_FUNCTION__, "Numerical factorization phase: " + this->GetErrorString(mSolver.info[0]) + ".");
    }


    CleanUp();

    timer.Reset(std::string("MUMPS ") + __FUNCTION__ + " solution");

    rSchurComplement = rSchurComplementTranspose.transpose();

}


std::string NuTo::SparseDirectSolverMUMPS::GetErrorString(int error) const
{
    assert(error < 0);

    switch (error)
    {
    case -9:
        return "main work array too small";
        break;
    case -10:
        return "numerically singular matrix";
        break;
    default:
        return "unknown error code";
    }
}
#else // HAVE_MUMPS
void NuTo::SparseDirectSolverMUMPS::Solve(const NuTo::SparseMatrixCSR<double>& rMatrix, const NuTo::FullVector<double, Eigen::Dynamic>& rRhs, NuTo::FullVector<double, Eigen::Dynamic>& rSolution)
{
	throw NuTo::MathException(__PRETTY_FUNCTION__, "MUMPS-solver was not found on your system (check cmake)");
}

void NuTo::SparseDirectSolverMUMPS::SchurComplement(const NuTo::SparseMatrixCSR<double>& rMatrix, const NuTo::FullVector<int, Eigen::Dynamic> rSchurIndices, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rSchurComplement)
{
    throw NuTo::MathException(__PRETTY_FUNCTION__, "MUMPS-solver was not found on your system (check cmake)");
}
#endif // HAVE_MUMPS
