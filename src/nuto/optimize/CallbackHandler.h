#ifndef CALLBACKHANDLER_H
#define CALLBACKHANDLER_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/base/NuToObject.h"
#include "nuto/optimize/OptimizeException.h"
#include "nuto/math/FullMatrix.h"

namespace NuTo
{

//! @author Joerg F. Unger, ISM
//! @date September 2009
//! @brief ... abstract class to handle callback routines
class CallbackHandler : public virtual NuToObject
{
#ifdef ENABLE_SERIALIZATION
	friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION

public:
    CallbackHandler() : NuToObject()
    {}
	
#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NuToObject);
    }
#endif // ENABLE_SERIALIZATION

	virtual void SetParameters(const NuTo::FullMatrix<double>& rParameters)
	{
	    //printf("[CallbackHandler::SetParameters]\n");
	    throw OptimizeException("[CallbackHandler::SetParameters] SetParameters function not implemented in CallbackHandler object.");
    }

	virtual double Objective()const
	{
	    //printf("[CallbackHandler::Objective]\n");
		throw OptimizeException("[CallbackHandler::Objective] Objective function not implemented in CallbackHandler object.");
		return 0;
	}

	virtual void Gradient (NuTo::FullMatrix<double>& rGradient)const
	{
	    //printf("[CallbackHandler::Gradient]\n");
	    throw OptimizeException("[CallbackHandler::Gradient] Gradient function not implemented in CallbackHandler object.");
	}

	virtual void Hessian(NuTo::FullMatrix<double>& rHessian)const
	{
	    //printf("[CallbackHandler::Hessian]\n");
	    throw OptimizeException("[CallbackHandler::Hessian] Hessian function not implemented in CallbackHandler object.");
	}

	virtual void Info()const=0;
#ifdef ENABLE_SERIALIZATION
	virtual void Save (const std::string &filename, std::string rType )const{}
    virtual void Restore (const std::string &filename, std::string rType ){}
#endif // ENABLE_SERIALIZATION

    virtual std::string GetTypeId()const
    {
    	return std::string("CallbackHandler");
    }

};
} //namespace NuTo
#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
#include <boost/serialization/assume_abstract.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::CallbackHandler)
#endif // SWIG
#endif // ENABLE_SERIALIZATION
#endif // CALLBACKHANDLER_H
