#pragma once

#include <vector>
#include <cassert>
#include <iostream>

#include <boost/foreach.hpp>

#include "math/SparseMatrix.h"
#include "base/Exception.h"

namespace NuTo
{
template <class T>
class SparseMatrixCSRVector2General;

//! @author Stefan Eckardt, ISM
//! @date July 2009
//! @brief ... abstract base class for sparse matrices which are stored in CSR format
template <class T>
class SparseMatrixCSR : public SparseMatrix<T>
{
    friend class SparseMatrixCSRVector2General<T>;

public:
    //! @brief ... constructor
    //! @param rNumRows_ ... number of rows
    //! @param rNumReserveEntries_ ... number of entries for which memory is reserved (optional)
    SparseMatrixCSR(int rNumRows_, unsigned int rNumReserveEntries_ = 0)
        : SparseMatrix<T>()
        , mValues()
        , mColumns()
        , mRowIndex(rNumRows_ + 1)
    {
        // check for overflow
        assert(rNumRows_ < INT_MAX);
        assert(rNumRows_ >= 0);

        this->mValues.reserve(rNumReserveEntries_);
        this->mColumns.reserve(rNumReserveEntries_);
    }
    virtual ~SparseMatrixCSR() = default;

    SparseMatrixCSR(const SparseMatrixCSR<T>& rOther) = default;

#ifndef SWIG
    SparseMatrixCSR(SparseMatrixCSR<T>&& rOther) = default;

    SparseMatrixCSR<T>& operator=(const SparseMatrixCSR<T>& rOther) = default;
    SparseMatrixCSR<T>& operator=(SparseMatrixCSR<T>&& rOther) = default;

#endif // SWIG

    //! @brief ... resize matrix
    //! @param rNumRows_ ... number of rows
    void Resize(int rNumRows_)
    {
        // check for overflow
        assert(rNumRows_ < INT_MAX);
        assert(rNumRows_ >= 0);

        // delete everything
        this->mRowIndex.resize(0);
        this->mValues.resize(0);
        this->mColumns.resize(0);

        // set number of rows and mColumns
        this->mRowIndex.resize(rNumRows_ + 1);
        if (this->mOneBasedIndexing)
        {
            for (int rowCount = 0; rowCount <= rNumRows_; rowCount++)
            {
                this->mRowIndex[rowCount] = 1;
            }
        }
    }

    //! @brief ... resize matrix
    //! @param rNumRows_ ... number of rows
    //! @param rNumColumns_ ... number of columns
    virtual void Resize(int, int) override
    {
        throw NuTo::Exception("[NuTo::SparseSparseMatrixCSR::Resize] Not implemented for two input values!");
    }

    //! @brief ... reserve memory for non-zero matrix entries
    //! @param rNumReserveEntries_ ... number of entries for which memory is reserved
    void Reserve(unsigned int rNumReserveEntries_)
    {
        this->mValues.reserve(rNumReserveEntries_);
        this->mColumns.reserve(rNumReserveEntries_);
    }

    //! @brief ... returns the row index vector
    //! @return const reference to an integer vector storing the row index
    const std::vector<int>& GetRowIndex() const
    {
        return this->mRowIndex;
    }

    //! @brief ... returns the vector of columns for the non-zero matrix entries
    //! @return const reference to an integer vector storing the columns of nonzero matrix entries
    const std::vector<int>& GetColumns() const
    {
        return this->mColumns;
    }

    //! @brief ... returns the vector of values of the non-zero matrix values
    //! @return const reference to an vector of type T storing thevalues of the non-zero matrix entries
    const std::vector<T>& GetValues() const
    {
        return this->mValues;
    }

    //! @brief ... returns the number of non-zero matrix entries
    //! @return number of non-zero matrix entries
    int GetNumEntries() const override
    {
        return this->mValues.size();
    }

    //! @brief ... returns the number of columns
    //! @return number of columns
    virtual int GetNumColumns() const override = 0;

    //! @brief ... returns the number of rows
    //! @return number of rows
    int GetNumRows() const override
    {
        return this->mRowIndex.size() - 1;
    }

    //! @brief ... add nonzero entry to matrix
    //! @param rRow ... row of the nonzero entry (zero based indexing!!!)
    //! @param rColumn ... column of the nonzero entry (zero based indexing!!!)
    //! @param rValue ... value of the nonzero entry
    virtual void AddValue(int rRow, int rColumn, const T& rValue) override = 0;

    //! @brief Print info about the object
    void Info() const override
    {
        std::cout << "number of rows: " << this->mRowIndex.size() - 1 << std::endl;
        std::cout << "number of nonzero entries: " << this->mValues.size() << "(" << this->mColumns.size() << ")"
                  << std::endl;
        std::cout << "capacity: " << this->mValues.capacity() << "(" << this->mColumns.capacity() << ")" << std::endl;
        std::cout << "mValues: ";
        for (unsigned int entry_count = 0; entry_count < this->mValues.size(); entry_count++)
        {
            std::cout << this->mValues[entry_count] << " ";
        }
        std::cout << std::endl;
        std::cout << "mColumns: ";
        for (unsigned int entry_count = 0; entry_count < this->mColumns.size(); entry_count++)
        {
            std::cout << this->mColumns[entry_count] << " ";
        }
        std::cout << std::endl;
        std::cout << "mRowIndex: ";
        for (unsigned int row_count = 0; row_count < this->mRowIndex.size(); row_count++)
        {
            std::cout << this->mRowIndex[row_count] << " ";
        }
        std::cout << std::endl;
    }

    //! @brief ... switch to one based indexing (only internal indexing, interface still uses zero based indexing)
    void SetOneBasedIndexing() override
    {
        if (!this->mOneBasedIndexing)
        {
            for (unsigned int entry_count = 0; entry_count < this->mColumns.size(); entry_count++)
            {
                this->mColumns[entry_count] += 1;
            }
            for (unsigned int row_count = 0; row_count < this->mRowIndex.size(); row_count++)
            {
                this->mRowIndex[row_count] += 1;
            }
            this->mOneBasedIndexing = true;
        }
    }

    //! @brief ... switch to zero based indexing (only internal indexing, interface still uses zero based indexing)
    void SetZeroBasedIndexing() override
    {
        if (this->mOneBasedIndexing)
        {
            for (unsigned int entry_count = 0; entry_count < this->mColumns.size(); entry_count++)
            {
                this->mColumns[entry_count] -= 1;
            }
            for (unsigned int row_count = 0; row_count < this->mRowIndex.size(); row_count++)
            {
                this->mRowIndex[row_count] -= 1;
            }
            this->mOneBasedIndexing = false;
        }
    }

    //! @brief ... multiplies the matrix with an scalar value
    //! @param rOther ... scalar value
    //! @return ... the multiplied matrix (sparse csr storage)
    SparseMatrixCSR<T>& operator*=(const T& rOther)
    {
        BOOST_FOREACH (T& val, this->mValues)
            val *= rOther;
        return *this;
    }

    //! @brief Calculate the maximum matrix entry
    //! @param rRowOutput ... row index of the maximum matrix entry
    //! @param rColumnOutput ... column index of the maximum matrix entry
    //! @param rResultOutput ... maximum matrix entry
    virtual void MaxEntry(int& rRowOutput, int& rColumnOutput, T& rResultOutput) const override
    {
        if (mValues.size() == 0)
            throw Exception("[NuTo::SparseMatrixCSR::Max] Maximum for matrix with zero entries cannot be calculated.");

        rResultOutput = mValues[0];

        unsigned int curRow(0);
        rRowOutput = curRow;
        rColumnOutput = mColumns[0];
        for (unsigned int count = 1; count < mValues.size(); count++)
        {
            while (mRowIndex[curRow] < (int)count)
                curRow++;
            if (mValues[count] > rResultOutput)
            {
                rResultOutput = mValues[count];
                rColumnOutput = (int)mColumns[count];
                rRowOutput = (int)curRow;
            }
        }
    }

    //! @brief Calculate the minimum matrix entry
    //! @param rRowOutput ... row index of the minimum matrix entry
    //! @param rColumnOutput ... column index of the minimum matrix entry
    //! @param rResultOutput ... minimum matrix entry
    virtual void MinEntry(int& rRowOutput, int& rColumnOutput, T& rResultOutput) const override
    {
        if (mValues.size() == 0)
            throw Exception("[NuTo::SparseMatrixCSR::Min] Minimum for matrix with zero entries cannot be calculated.");

        rResultOutput = mValues[0];

        unsigned int curRow(0);
        rRowOutput = curRow;
        rColumnOutput = mColumns[0];
        for (unsigned int count = 1; count < mValues.size(); count++)
        {
            while (mRowIndex[curRow] < (int)count)
                curRow++;
            if (mValues[count] < rResultOutput)
            {
                rResultOutput = mValues[count];
                rColumnOutput = (int)mColumns[count];
                rRowOutput = (int)curRow;
            }
        }
    }

    //! @brief ... remove zero entries from matrix (all entries with an absolute value which is smaller than a
    //! prescribed tolerance)
    //! @param rAbsoluteTolerance ... absolute tolerance
    //! @param rRelativeTolerance ... relative tolerance (this value is multiplied with the largest matrix entry
    //! (absolute values))
    int RemoveZeroEntries(double rAbsoluteTolerance = 0, double rRelativeTolerance = 0) override;


    //! @brief ... sets all the values to zero while keeping the structure of the matrix constant, this is interesting
    //! for stiffness matrices to use the same matrix structure
    void SetZeroEntries() override
    {
        for (unsigned int count = 0; count < mValues.size(); count++)
        {
            mValues[count] = 0;
        }
    }

protected:
    //! @brief value of nonzero matrix entries
    std::vector<T> mValues;
    //! @brief mColumns of nonzero matrix entries
    std::vector<int> mColumns;
    //! @brief row index of nonzero matrix entries (cummulative number of nonzero entries in each row)
    std::vector<int> mRowIndex;
};
}
