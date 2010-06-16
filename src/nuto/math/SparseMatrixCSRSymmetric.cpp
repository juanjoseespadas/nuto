// $Id$

#include <boost/spirit/include/classic_core.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include "nuto/math/Matrix.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrix.h"
#include "nuto/math/SparseMatrixCSR.h"
#include "nuto/math/SparseMatrixCSRSymmetric.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"
#include "nuto/math/MathException.h"

namespace NuTo
{
//! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
//!            in case of restoring from a file with the wrong object type, the file id is printed
//! @return    class name FullMatrixDouble

template<>
std::string SparseMatrixCSRSymmetric<double>::GetTypeId()const
{
    return std::string("SparseMatrixCSRSymmetricDouble");
}

//! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
//!            in case of restoring from a file with the wrong object type, the file id is printed
//! @return    class name FullMatrixInt
template<>
std::string SparseMatrixCSRSymmetric<int>::GetTypeId()const
{
    return std::string("SparseMatrixCSRSymmetricInt");
}

template<>
void SparseMatrixCSRSymmetric<double>::ImportFromSLangText(const char* rFileName)
{
    using namespace boost::spirit::classic;

    // open file
    std::ifstream file(rFileName, std::ios::in);
    if (file.is_open() == false)
    {
        throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]error opening file.");
    }

    // read header
    unsigned int SLangVersion(0), objectType(SLANG_NOTYPE), objectKind(SLANG_NOKIND), objectNumRows(0), objectNumColumns(0), objectNumEntries(0);
    this->ImportFromSLangTextReadHeader(file, SLangVersion, objectType, objectKind, objectNumRows, objectNumColumns, objectNumEntries);

    // check SLang version
    if (SLangVersion != 511)
    {
        throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]unsupported SLang version.");
    }

    // check object type and kind
    if (objectType != SLANG_REAL)
    {
        throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]object data must be of type DOUBLE.");
    }
    if (objectKind != SLANG_COMPACT_MATRIX)
    {
        throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]object must be a COMPACT_MATRIX.");
    }
    // set number of rows and number of entries
    objectNumEntries = objectNumRows;
    objectNumRows = objectNumColumns;
    this->Resize(objectNumRows);
    this->Reserve(objectNumEntries);

    // switch to one based indexing
//    bool oldOneBasedIndexing = this->oneBasedIndexing;
//    this->setOneBasedIndexing();

    // read nonzero entries which are stored in the lower triangle format
    unsigned int row = 1;
    std::vector<unsigned int> tmpRows;
    std::vector<unsigned int> tmpColumns;
    std::vector<double> tmpValues;
    tmpRows.reserve(objectNumEntries);
    tmpColumns.reserve(objectNumEntries);
    tmpValues.reserve(objectNumEntries);
    std::vector<int> tmpRowIndex(objectNumRows);
    for (unsigned int entryCount = 0; entryCount < objectNumEntries; entryCount++)
    {
        std::string line;
        getline (file, line);
        if (parse(line.c_str(),(uint_p[push_back_a(tmpRows)] >> real_p[push_back_a(tmpColumns)] >> real_p[push_back_a(tmpValues)]),space_p).full == false)
        {
            throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]error reading nonzero matrix entries.");
        }
        if (tmpRows[entryCount] > objectNumRows || tmpColumns[entryCount] > objectNumColumns)
        {
            throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]row or column out of bounds.");
        }
        if (tmpRows[entryCount] < row)
        {
            throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]invalid sorting of compressed matrix.");
        }
        if (tmpColumns[entryCount] > tmpRows[entryCount])
        {
            throw MathException("[SparseMatrixCSRSymmetric::importFromSLang]compressed matrix must be stored in lower triangle format.");
        }
        row = tmpRows[entryCount];
        tmpRowIndex[tmpColumns[entryCount] - 1] += 1;
    }

    // store entries in upper triangle format
    if (this->mOneBasedIndexing)
    {
        this->mRowIndex[0] = 1;
    }
    else
    {
        this->mRowIndex[0] = 0;
        for (unsigned int entryCount = 0; entryCount < objectNumEntries; entryCount++)
        {
            tmpRows[entryCount] -= 1;
        }
    }
    for (unsigned int rowCount = 0; rowCount < objectNumRows; rowCount++)
    {
        this->mRowIndex[rowCount + 1] = this->mRowIndex[rowCount] + tmpRowIndex[rowCount];
    }
    tmpRowIndex[0] = 0;
    for (unsigned int rowCount = 1; rowCount < objectNumRows; rowCount++)
    {
        tmpRowIndex[rowCount] = this->mRowIndex[rowCount] - this->mRowIndex[0];
    }
    this->mColumns.resize(objectNumEntries);
    this->mValues.resize(objectNumEntries);
    for (unsigned int entryCount = 0; entryCount < objectNumEntries; entryCount++)
    {
        int curRow = tmpColumns[entryCount] - 1;
        int pos = tmpRowIndex[curRow];
        this->mColumns[pos] = tmpRows[entryCount];
        this->mValues[pos] = tmpValues[entryCount];
        tmpRowIndex[curRow] += 1;
    }

    // close file
    file.close();
}

template<>
void SparseMatrixCSRSymmetric<int>::ImportFromSLangText(const char* fileName)
{
    throw MathException("[SparseMatrixCSRSymmetric::importFromSLang] not implemented for this data-type.");
}

// adds the product of trans(A) * B * A to the matrix (A is a general matrix, and B is a symmetric matrix)
template<>
void SparseMatrixCSRSymmetric<int>::Add_TransA_Mult_B_Mult_A(const NuTo::SparseMatrixCSRGeneral<int>& rMatrixA, const NuTo::SparseMatrixCSRSymmetric<int>& rMatrixB)
{
    throw MathException("[SparseMatrixCSRSymmetric::Add_TransA_Mult_B_Mult_A] not implemented for this data-type.");
}

// adds the product of trans(A) * B * A to the matrix (A is a general matrix, and B is a symmetric matrix)
template<>
void SparseMatrixCSRSymmetric<double>::Add_TransA_Mult_B_Mult_A(const NuTo::SparseMatrixCSRGeneral<double>& rMatrixA, const NuTo::SparseMatrixCSRSymmetric<double>& rMatrixB)
{
    assert(rMatrixB.GetNumRows() == rMatrixB.GetNumColumns());
    if (rMatrixB.GetNumRows() != rMatrixA.GetNumRows())
    {
        throw MathException("[SparseMatrixCSRSymmetric::Add_TransA_Mult_B_Mult_A] invalid dimension of matrices A and B.");
    }
    assert(this->GetNumRows() == this->GetNumColumns());
    if (rMatrixA.GetNumColumns() != this->GetNumRows())
    {
        throw MathException("[SparseMatrixCSRSymmetric::Add_TransA_Mult_B_Mult_A] invalid number of columns in matrix A.");
    }
    // loop over the columns of trans(A) == rows of A
    const std::vector<int>& matrixARowIndex = rMatrixA.GetRowIndex();
    const std::vector<int>& matrixAColumns = rMatrixA.GetColumns();
    const std::vector<double>& matrixAValues = rMatrixA.GetValues();
    for (int transAColumn = 0; transAColumn < rMatrixA.GetNumRows(); transAColumn++)
    {
        for (int transAPos = matrixARowIndex[transAColumn]; transAPos < matrixARowIndex[transAColumn + 1]; transAPos++)
        {
            int transARow = matrixAColumns[transAPos];
            double transAValue = matrixAValues[transAPos];

            // multiply with B matrix
            int BRow = transAColumn;

            // upper triangle and diagonal of B
            for (int BPos = rMatrixB.mRowIndex[BRow]; BPos < rMatrixB.mRowIndex[BRow + 1]; BPos++)
            {
                int BColumn = rMatrixB.mColumns[BPos];
                double BValue = rMatrixB.mValues[BPos];
                double transAValue_Mult_BValue = transAValue * BValue;
                // multiply A
                int ARow = BColumn;
                for (int APos = matrixARowIndex[ARow]; APos < matrixARowIndex[ARow + 1]; APos++)
                {
                    int AColumn = matrixAColumns[APos];
                    double AValue = matrixAValues[APos];
                    // add to this matrix (upper triangle storage)
                    int thisRow = transARow;
                    int thisColumn = AColumn;
                    if (thisColumn >= thisRow)
                    {
                        this->AddEntry(thisRow, thisColumn, transAValue_Mult_BValue * AValue);
                    }

                }
            }

            // lower triangle of B
            for (int BColumn = 0; BColumn < transAColumn; BColumn++)
            {
                int BPos = rMatrixB.mRowIndex[BColumn];
                for (; BPos < rMatrixB.mRowIndex[BColumn + 1]; BPos++)
                {
                    if (rMatrixB.mColumns[BPos] >= transAColumn)
                    {
                        break;
                    }
                }
                if ((BPos < rMatrixB.mRowIndex[BColumn + 1]) && (rMatrixB.mColumns[BPos] == transAColumn))
                {
                    BRow = rMatrixB.mColumns[BPos];
                    double BValue = rMatrixB.mValues[BPos];

                    double transAValue_Mult_BValue = transAValue * BValue;
                    // multiply A
                    int ARow = BColumn;
                    for (int APos = matrixARowIndex[ARow]; APos < matrixARowIndex[ARow + 1]; APos++)
                    {
                        int AColumn = matrixAColumns[APos];
                        double AValue = matrixAValues[APos];

                        // add to this matrix (upper triangle storage)
                        int thisRow = transARow;
                        int thisColumn = AColumn;
                        if (thisColumn >= thisRow)
                        {
                            this->AddEntry(thisRow, thisColumn, transAValue_Mult_BValue * AValue);
                        }

                    }
                }
            }
        }
    }
}

// subtract (trans(A) * B + trans(B) * A) from the matrix (A and B are general matrices)
template<>
void SparseMatrixCSRSymmetric<int>::Sub_TransA_Mult_TransB_Plus_B_Mult_A(const NuTo::SparseMatrixCSRGeneral<int>& rMatrixA, const NuTo::SparseMatrixCSRGeneral<int>& rMatrixB)
{
    throw MathException("[SparseMatrixCSRSymmetric::Sub_TransA_Mult_B_Plus_TransB_Mult_A] not implemented for this data-type.");
}

// subtract (trans(A) * trans(B) + B * A) from the matrix (A and B are general matrices)
template<>
void SparseMatrixCSRSymmetric<double>::Sub_TransA_Mult_TransB_Plus_B_Mult_A(const NuTo::SparseMatrixCSRGeneral<double>& rMatrixA, const NuTo::SparseMatrixCSRGeneral<double>& rMatrixB)
{
    if (rMatrixA.GetNumRows() != rMatrixB.GetNumColumns())
    {
        throw MathException("[SparseMatrixCSRSymmetric::Sub_TransA_Mult_TransB_Plus_B_Mult_A] invalid number of rows of matrix A and B.");
    }
    if (rMatrixB.GetNumRows() != this->GetNumRows())
    {
        throw MathException("[SparseMatrixCSRSymmetric::Sub_TransA_Mult_TransB_Plus_B_Mult_A] invalid number of columns of matrix B.");
    }
    if (rMatrixA.GetNumColumns() != this->GetNumColumns())
    {
        throw MathException("[SparseMatrixCSRSymmetric::Sub_TransA_Mult_TransB_Plus_B_Mult_A] invalid number of columns of matrix A.");
    }

    // calculate B * A
    // matrix A
    const std::vector<int>& matrixARowIndex = rMatrixA.GetRowIndex();
    const std::vector<int>& matrixAColumns = rMatrixA.GetColumns();
    const std::vector<double> matrixAValues = rMatrixA.GetValues();

    // matrix B
    const std::vector<int>& matrixBRowIndex = rMatrixB.GetRowIndex();
    const std::vector<int>& matrixBColumns = rMatrixB.GetColumns();
    const std::vector<double> matrixBValues = rMatrixB.GetValues();

    for (int bRow = 0; bRow < rMatrixB.GetNumRows(); bRow++)
    {
        for(int bPos = matrixBRowIndex[bRow]; bPos < matrixBRowIndex[bRow + 1]; bPos++)
        {
            int bColumn = matrixBColumns[bPos];
            double bValue = matrixBValues[bPos];

            int aRow = bColumn;
            for(int aPos = matrixARowIndex[aRow]; aPos < matrixARowIndex[aRow + 1]; aPos++)
            {
                int aColumn = matrixAColumns[aPos];
                double aValue = matrixAValues[aPos];
                double value = -1.0 * aValue * bValue;
                if(aColumn >= bRow)
                {
                    // sub value from upper triangle
                    this->AddEntry(bRow, aColumn, value);
                }
                if (bRow >= aColumn)
                {
                    // sub transpose from upper triangle
                    this->AddEntry(aColumn, bRow, value);
                }
            }
        }
    }
}

// multiply sparse matrix with full matrix
template<>
FullMatrix<int> SparseMatrixCSRSymmetric<int>::operator* (const FullMatrix<int>& rMatrix) const
{
    throw MathException("[SparseMatrixCSRSymmetric<int>::operator*] not implemented for this data type.");
}

// multiply sparse matrix with full matrix
template<>
FullMatrix<double> SparseMatrixCSRSymmetric<double>::operator* (const FullMatrix<double>& rMatrix) const
{
    if (this->GetNumColumns() != rMatrix.GetNumRows())
    {
        throw MathException("[SparseMatrixCSRSymmetric<int>::operator*] invalid number of rows in input matrix.");
    }
    FullMatrix<double> result(this->GetNumRows(),rMatrix.GetNumColumns());

    for (int matrixCol = 0; matrixCol < rMatrix.GetNumColumns(); matrixCol++)
    {
        const double* matrixValues = rMatrix.GetEigenMatrix().data() + matrixCol * rMatrix.GetNumRows();
        double* resultValues = result.GetEigenMatrix().data() + matrixCol * rMatrix.GetNumRows();
        for (int thisRow = 0; thisRow < this->GetNumRows(); thisRow++)
        {
            for (int thisPos = this->mRowIndex[thisRow]; thisPos < this->mRowIndex[thisRow + 1]; thisPos++)
            {
                int thisColumn = this->mColumns[thisPos];
                int thisValue = this->mValues[thisPos];
                resultValues[thisRow] += thisValue * matrixValues[thisColumn];
                if (thisRow != thisColumn)
                {
                    resultValues[thisColumn] += thisValue * matrixValues[thisRow];
                }
            }
        }
    }
    return result;
}

// multiply sparse matrix with scalar
template<>
SparseMatrixCSRSymmetric<int> SparseMatrixCSRSymmetric<int>::operator* (const int& rScal) const
{
    throw MathException("[SparseMatrixCSRSymmetric<int>::operator*] not implemented for this data type.");
}

// multiply sparse matrix with scalar
template<>
SparseMatrixCSRSymmetric<double> SparseMatrixCSRSymmetric<double>::operator* (const double& rScal) const
{
	SparseMatrixCSRSymmetric<double> result(*this);
	BOOST_FOREACH( double &val, result.mValues )
		val *= rScal;
	return result;
}


} // namespace NuTo