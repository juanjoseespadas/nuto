// $Id$

#ifndef ZEROMEANUNITVARIANCETRANSFORMATION_H
#define ZEROMEANUNITVARIANCETRANSFORMATION_H

#include "nuto/metamodel/Transformation.h"

namespace NuTo
{

//! @author Stefan Eckardt
//! @date February 2010
//! @brief zero mean, unit variance transformation
class ZeroMeanUnitVarianceTransformation : public Transformation
{
#ifdef ENABLE_SERIALIZATION
	friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    //! @param rCoordinate ... coordinate within the point coordinates
    //! @sa mCoordinate
    ZeroMeanUnitVarianceTransformation(unsigned int rCoordinate);
	
    //! @brief copy constructor
    ZeroMeanUnitVarianceTransformation(const ZeroMeanUnitVarianceTransformation &other)
	{
	    mCoordinate = other.mCoordinate;
	}
    
    //! @brief destructor
	~ZeroMeanUnitVarianceTransformation()
	{}

#ifdef ENABLE_SERIALIZATION
	//! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Transformation)
           & BOOST_SERIALIZATION_NVP(mCoordinate)
           & BOOST_SERIALIZATION_NVP(mMean)
           & BOOST_SERIALIZATION_NVP(mStandardDeviation);
    }
#endif  // ENABLE_SERIALIZATION

    //! @brief build the transformation using the given Points
    //! @param rCoordinates ... point coordinates
    virtual void Build(const FullMatrix<double>& rCoordinates);

    //! @brief transform the given points in forward direction x = f(x) 
    //! @param rCoordinates ... point coordinates
    virtual void TransformForward(FullMatrix<double>& rCoordinates)const;

    //! @brief transform the given points in backward direction x = f^(-1)(x)
    //! @param rCoordinates ... point coordinates
    virtual void TransformBackward(FullMatrix<double>& rCoordinates)const;

protected:
    int  mCoordinate;     //!< coordinate within the point coordinates (0<=entry<dim)
    double  mMean;        //!< mean value of given coordinates
    double  mStandardDeviation; //!< standard deviation of given coordinates
};


} // namespace nuto

#endif // ZEROMEANUNITVARIANCETRANSFORMATION_H
