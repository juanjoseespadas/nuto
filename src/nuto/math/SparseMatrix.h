// $Id$

#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/math/Matrix.h"

namespace NuTo
{
// forward devlarations
template<class T> class FullMatrix;

//! @author Stefan Eckardt, ISM
//! @date July 2009
//! @brief ... abstract base class for sparse matrices
template <class T>
class SparseMatrix : public Matrix<T>
{
#ifdef ENABLE_SERIALIZATION
	friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION

public:
    //! @brief ... constructor
    SparseMatrix() : Matrix<T>()
    {
        this->mOneBasedIndexing=false;
        this->mPositiveDefinite=false;
    }

    //! @brief ... get number of non-zero entries
    //! @return number of non-zero matrix entries
    virtual int GetNumEntries() const = 0;

    //! @brief ... get number of rows
    //! @return number of rows
    virtual int GetNumRows() const = 0;

    //! @brief ... get number of columns
    //! @return number of columns
    virtual int GetNumColumns() const = 0;

	//! @brief ... add nonzero entry to matrix
    //! @param row ... row of the nonzero entry (zero based indexing!!!)
    //! @param column ... column of the nonzero entry (zero based indexing!!!)
    //! @param value ... value of the nonzero entry
    virtual void AddEntry(int row, int column, T value) = 0;

    //! @brief ... switch to one based indexing
    virtual void SetOneBasedIndexing() = 0;

    //! @brief ... switch to zero based indexing
    virtual void SetZeroBasedIndexing() = 0;

    //! @brief ... get type of indexing
    //! @return true if one based indexing / false if zero based indexing
    inline bool HasOneBasedIndexing() const
    {
        return this->mOneBasedIndexing;
    }

    //! @brief ... get type of indexing
    //! @return false if one based indexing / true if zero based indexing
    inline bool HasZeroBasedIndexing() const
    {
        return ! this->mOneBasedIndexing;
    }

    //! @brief ... get definiteness of matrix
    //! @return true if matrix is positive definite / false if matrix is indefinite
    inline bool IsPositiveDefinite() const
    {
        return this->mPositiveDefinite;
    }

    //! @brief ... set definiteness of matrix to positive definite
    inline void SetPositiveDefinite()
    {
        this->mPositiveDefinite = true;
    }

    //! @brief ... set definiteness of matrix to indefinite
    inline void SetIndefinite()
    {
        this->mPositiveDefinite = false;
    }

    //! @brief ... symmetry of the matrix
    //! @return ... true if the matrix is symmetric, false otherwise
    virtual bool IsSymmetric() const = 0;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        std::cout << "start serialize of SparseMatrix" << std::endl;
        ar & boost::serialization::make_nvp("Matrix",boost::serialization::base_object< Matrix<T> >(*this));
        ar & BOOST_SERIALIZATION_NVP(mOneBasedIndexing)
        & BOOST_SERIALIZATION_NVP(mPositiveDefinite);
        std::cout << "finish serialize of SparseMatrix" << std::endl;
    }
#endif // ENABLE_SERIALIZATION

    virtual void WriteEntriesToFullMatrix(FullMatrix<T>& FullMatrix) const = 0;

    //! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
    //!            in case of restoring from a file with the wrong object type, the file id is printed
    //! @return    class name
    virtual std::string GetTypeId() const;

    virtual void Max(T& result_output)=0;
    virtual void Max(int& row_output, int& column_output, T& result_output)=0;
    virtual void Min(T& result_output)=0;
    virtual void Min(int& row_output, int& column_output, T& result_output)=0;

protected:
    //! @brief ... internal indexing of the matrix (true if one based indexing / false if zero based indexing)
    bool mOneBasedIndexing;
    //! @brief ... definiteness of the matrix (true if positive definite / false if indefinite)
    bool mPositiveDefinite;
};
}
#endif // SPARSE_MATRIX_H