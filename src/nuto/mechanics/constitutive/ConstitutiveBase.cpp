// $Id$
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/math/FullMatrix.h"
#include <iostream>

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/mechanics/MechanicsException.h"

// constructor
NuTo::ConstitutiveBase::ConstitutiveBase()
{
    this->mParametersValid = false;
}

// set Young's modulus
void NuTo::ConstitutiveBase::SetYoungsModulus(double rE)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetYoungsModulus] The constitutive relationship does not have a parameter Young's modulus.");
}

// get Young's modulus
double NuTo::ConstitutiveBase::GetYoungsModulus() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetYoungsModulus] The constitutive relationship does not have a parameter Young's modulus.");
}

//! @brief ... get factor to modify Young's modulus (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorYoungsModulus(const ElementBase* rElement,int rIp) const
{
    return 1;
}

// set Poisson's ratio
void NuTo::ConstitutiveBase::SetPoissonsRatio(double rNu)
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetPoissonsRatio] The constitutive relationship does not have a parameter Poisson's ratio.");
}

// get Poisson's ratio
double NuTo::ConstitutiveBase::GetPoissonsRatio() const
{
    throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetPoissonsRatio] The constitutive relationship does not have a parameter Poisson's ratio.");
}

//! @brief ... get factor to modify Poisson's ratio (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorPoissonsRatio(const ElementBase* rElement,int rIp) const
{
    return 1;
}

//! @brief ... get initial yield strength
//! @return ... yield strength
double NuTo::ConstitutiveBase::GetInitialYieldStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetInitialYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... set initial yield strength
//! @param rSigma ...  yield strength
void NuTo::ConstitutiveBase::SetInitialYieldStrength(double rSigma)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetInitialYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... get yield strength for multilinear response
//! @return ... first column: equivalent plastic strain
//! @return ... second column: corresponding yield strength
NuTo::FullMatrix<double> NuTo::ConstitutiveBase::GetYieldStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... add yield strength
//! @param rEpsilon ...  equivalent plastic strain
//! @param rSigma ...  yield strength
void NuTo::ConstitutiveBase::AddYieldStrength(double rEpsilon, double rSigma)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AddYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... get factor to modify yield strength (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorYieldStrength(const ElementBase* rElement,int rIp) const
{
    return 1;
}

//! @brief ... get initial hardening modulus
//! @return ... hardening modulus
double NuTo::ConstitutiveBase::GetInitialHardeningModulus() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetHardeningModulus] The constitutive relationship does not have a parameter hardening modulus.");
}

//! @brief ... set initial hardening modulus
//! @param rH ...  hardening modulus
void NuTo::ConstitutiveBase::SetInitialHardeningModulus(double rH)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetHardeningModulus] The constitutive relationship does not have a parameter hardening modulus.");
}

//! @brief ... get hardening modulus for multilinear response
//! @return ... first column: equivalent plastic strain
//! @return ... second column: corresponding hardening modulus
NuTo::FullMatrix<double> NuTo::ConstitutiveBase::GetHardeningModulus() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... add hardening modulus
//! @param rEpsilon ...  equivalent plastic strain
//! @param rSigma ...  hardening modulus
void NuTo::ConstitutiveBase::AddHardeningModulus(double rEpsilon, double rH)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::AddYieldStrength] The constitutive relationship does not have a parameter yield strength.");
}

//! @brief ... get factor to modify hardening modulus (using random fields)
//! @param rElement ...  element
//! @param rIp ...  integration point
double NuTo::ConstitutiveBase::GetRanfieldFactorHardeningModulus(const ElementBase* rElement,int rIp) const
{
    return 1;
}

//! @brief ... get nonlocal radius
//! @return ... nonlocal radius
double NuTo::ConstitutiveBase::GetNonlocalRadius() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetNonlocalRadius] The constitutive relationship does not have a nonlocal radius.");
}

//! @brief ... set nonlocal radius
//! @param rRadius ...  nonlocal radius
void NuTo::ConstitutiveBase::SetNonlocalRadius(double rH)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetNonlocalRadius] The constitutive relationship does not have a nonlocal radius.");
}
//! @brief ... get tensile strength
//! @return ... tensile strength
double NuTo::ConstitutiveBase::GetTensileStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetTensileStrength] The constitutive relationship does not have a tensile strength.");
}

//! @brief ... set tensile strength
//! @param rTensileStrength...  tensile strength
void NuTo::ConstitutiveBase::SetTensileStrength(double rTensileStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetTensileStrength] The constitutive relationship does not have a tensile strength.");
}

//! @brief ... get compressive strength
//! @return ... compressive strength
double NuTo::ConstitutiveBase::GetCompressiveStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetCompressiveStrength] The constitutive relationship does not have a compressive strength.");
}

//! @brief ... set compressive strength
//! @param rCompressiveStrength...  compressive strength
void NuTo::ConstitutiveBase::SetCompressiveStrength(double rCompressiveStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetCompressiveStrength] The constitutive relationship does not have a compressive strength.");
}

//! @brief ... get biaxial compressive strength
//! @return ... biaxial compressive strength
double NuTo::ConstitutiveBase::GetBiaxialCompressiveStrength() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetBiaxialCompressiveStrength] The constitutive relationship does not have a biaxial compressive strength.");
}

//! @brief ... set biaxial compressive strength
//! @param rBiaxialCompressiveStrength...  biaxial compressive strength
void NuTo::ConstitutiveBase::SetBiaxialCompressiveStrength(double rBiaxialCompressiveStrength)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetBiaxialCompressiveStrength] The constitutive relationship does not have a biaxial compressive strength.");
}

//! @brief ... get fracture energy
//! @return ... fracture energy
double NuTo::ConstitutiveBase::GetFractureEnergy() const
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::GetFractureEnergy] The constitutive relationship does not have a fracture energy.");
}

//! @brief ... set fracture energy
//! @param rFractureEnergy... fracture energy
void NuTo::ConstitutiveBase::SetFractureEnergy(double rFractureEnergy)
{
	throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetFractureEnergy] The constitutive relationship does not have a fracture energy.");
}

// modify parameter validity flag
void NuTo::ConstitutiveBase::SetParametersValid()
{
    try
    {
        this->CheckParameters();
    }
    catch (NuTo::MechanicsException)
    {
        this->mParametersValid = false;
        return;
    }
    catch (...)
    {
        throw NuTo::MechanicsException("[NuTo::ConstitutiveBase::SetParametersValid] Unhandled exception");
    }
    this->mParametersValid = true;
}

// info routine
void NuTo::ConstitutiveBase::Info(unsigned short rVerboseLevel) const
{
    std::cout << "    object pointer: " << this << std::endl;
    std::cout << "    parameter validity flag: " << this->mParametersValid << std::endl;
}

#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::ConstitutiveBase::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstitutiveBase::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstitutiveBase::serialize(Archive & ar, const unsigned int version)
{
    std::cout << "start serialize constitutive Base" << std::endl;
    ar & BOOST_SERIALIZATION_NVP(mParametersValid);
    std::cout << "finish serialize Constitutive Base" << std::endl;
}
#endif // ENABLE_SERIALIZATION
