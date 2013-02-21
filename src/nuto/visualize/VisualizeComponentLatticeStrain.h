// $Id: VisualizeComponentLatticeStrain.h 342 2010-10-18 12:39:08Z arnold2 $
#ifndef VisualizeComponentLatticeStrain_H_
#define VisualizeComponentLatticeStrain_H_

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/visualize/VisualizeComponentBase.h"

namespace NuTo
{
//! @author Joerg F. Unger
//! @date Apr 27, 2010
//! @brief ...this routine is normally not needed, since the Strain tensor is export
//! unfortunately, the extraction of the principal Straines in paraview was not straigthforward
//! that is why this class is implemented
class VisualizeComponentLatticeStrain : public VisualizeComponentBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION
public:
    VisualizeComponentLatticeStrain();

    inline NuTo::VisualizeBase::eVisualizeWhat GetComponentEnum()const
    {
    	return NuTo::VisualizeBase::LATTICE_STRAIN;
    }

    inline std::string GetComponentName()const
    {
    	return std::string("LatticeStrain");
    }

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION
};
}
#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::VisualizeComponentLatticeStrain)
#endif // ENABLE_SERIALIZATION
#endif /* VisualizeComponentLatticeStrain_H_ */