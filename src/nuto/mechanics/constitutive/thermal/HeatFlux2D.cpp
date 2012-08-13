// $Id$

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/mechanics/constitutive/thermal/HeatFlux2D.h"

NuTo::HeatFlux2D::HeatFlux2D(): ConstitutiveOutputBase()
{
	mHeatFlux[0] = 0.0;
	mHeatFlux[1] = 0.0;
}


//! @brief ... get number of strain components
//! @return ... number of strain components
unsigned int NuTo::HeatFlux2D::GetNumberOfComponents() const
{
    return 2;
}

//! @brief ... get Engineering Strain
//! @return ... Engineering Strain (exx)
const double* NuTo::HeatFlux2D::GetData() const
{
    return mHeatFlux;
}

#ifdef ENABLE_SERIALIZATION
//! @brief serializes the class
//! @param ar         archive
//! @param version    version
template void NuTo::HeatFlux2D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::HeatFlux2D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::HeatFlux2D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::HeatFlux2D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::HeatFlux2D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::HeatFlux2D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::HeatFlux2D::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize HeatFlux2D" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstitutiveOutputBase)
       & BOOST_SERIALIZATION_NVP(mHeatFlux);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize HeatFlux2D" << std::endl;
#endif
}
#endif // ENABLE_SERIALIZATION

