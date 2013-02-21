// $Id$

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

#include "nuto/base/Logger.h"
#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveInputBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveOutputBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveStaticDataBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveTangentLocal.h"
#include "nuto/mechanics/constitutive/mechanics/Damage.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient1D.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient2D.h"
#include "nuto/mechanics/constitutive/mechanics/DeformationGradient3D.h"
#include "nuto/mechanics/constitutive/mechanics/LinearElasticEngineeringStress.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress1D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress2D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress3D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain1D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain2D.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStrain3D.h"
#include "nuto/mechanics/elements/ElementBase.h"
#include "nuto/mechanics/sections/SectionBase.h"
#include "nuto/mechanics/sections/SectionEnum.h"

NuTo::LinearElasticEngineeringStress::LinearElasticEngineeringStress() : ConstitutiveBase()
{
	mE = 0.;
	mNu = 0.;
	mRho = 0.;
	SetParametersValid();
}

#ifdef ENABLE_SERIALIZATION
//! @brief serializes the class
//! @param ar         archive
//! @param version    version
template<class Archive>
void NuTo::LinearElasticEngineeringStress::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
   std::cout << "start serialize LinearElasticEngineeringStress" << std::endl;
#endif
   ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstitutiveBase)
      & BOOST_SERIALIZATION_NVP(mE)
      & BOOST_SERIALIZATION_NVP(mNu)
      & BOOST_SERIALIZATION_NVP(mRho)
      & BOOST_SERIALIZATION_NVP(mThermalExpansionCoefficient);
#ifdef DEBUG_SERIALIZATION
   std::cout << "finish serialize LinearElasticEngineeringStress" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::LinearElasticEngineeringStress)
#endif // ENABLE_SERIALIZATION

//! @brief ... evaluate the constitutive relation in 1D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::LinearElasticEngineeringStress::Evaluate1D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::eOutput, ConstitutiveOutputBase*>& rConstitutiveOutput)
{
	// get section information determining which input on the constitutive level should be used
	const SectionBase* section(rElement->GetSection());

	// check if parameters are valid
    if (this->mParametersValid == false)
    {
   		//throw an exception giving information related to the wrong parameter
    	CheckParameters();
    }

	EngineeringStrain1D engineeringStrain;
	// calculate engineering strain
	if(rConstitutiveInput.find(NuTo::Constitutive::eInput::DEFORMATION_GRADIENT_1D)==rConstitutiveInput.end())
		throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate] deformation gradient 1d needed to evaluate engineering strain2d.");
	const DeformationGradient1D& deformationGradient(rConstitutiveInput.find(NuTo::Constitutive::eInput::DEFORMATION_GRADIENT_1D)->second->GetDeformationGradient1D());
	deformationGradient.GetEngineeringStrain(engineeringStrain);

    for (std::map<NuTo::Constitutive::eOutput, ConstitutiveOutputBase*>::iterator itOutput = rConstitutiveOutput.begin();
    		itOutput != rConstitutiveOutput.end(); itOutput++)
    {
    	switch(itOutput->first)
    	{
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_1D:
    	{
    		EngineeringStrain1D elasticEngineeringStrain(engineeringStrain);
    		// if temperature is an input, subtract thermal strains to get elastic strains
    		if (section->GetInputConstitutiveIsTemperature())
    		{
    			std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::eInput::TEMPERATURE));
    			if (itInput==rConstitutiveInput.end())
    				throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate1D] temperature needed to evaluate thermal engineering strain1d.");
    			double temperature(itInput->second->GetTemperature());
    			double deltaStrain(mThermalExpansionCoefficient * temperature);
    			EngineeringStrain1D elasticEngineeringStrain;
    			elasticEngineeringStrain.mEngineeringStrain -= deltaStrain;
    		}
			EngineeringStress1D& engineeringStress(itOutput->second->GetEngineeringStress1D());
			// calculate Engineering stress
			engineeringStress.mEngineeringStress = mE * elasticEngineeringStrain.mEngineeringStrain;

		    break;
    	}
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_3D:
    	{
    		//this is for the visualize routines
    		EngineeringStrain1D elasticEngineeringStrain(engineeringStrain);
    		// if temperature is an input, subtract thermal strains to get elastic strains
    		if (section->GetInputConstitutiveIsTemperature())
    		{
    			std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::eInput::TEMPERATURE));
    			if (itInput==rConstitutiveInput.end())
    				throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate2D] temperature needed to evaluate thermal engineering strain2d.");
    			double temperature(itInput->second->GetTemperature());
    			double deltaStrain(mThermalExpansionCoefficient * temperature);
    			EngineeringStrain1D elasticEngineeringStrain;
    			elasticEngineeringStrain.mEngineeringStrain -= deltaStrain;
    		}
			EngineeringStress3D& engineeringStress(itOutput->second->GetEngineeringStress3D());

			// calculate Engineering stress
			engineeringStress.mEngineeringStress[0] = mE * elasticEngineeringStrain.mEngineeringStrain;
			engineeringStress.mEngineeringStress[1] = 0.;
			engineeringStress.mEngineeringStress[2] = 0.;
			engineeringStress.mEngineeringStress[3] = 0.;
			engineeringStress.mEngineeringStress[4] = 0.;
			engineeringStress.mEngineeringStress[5] = 0.;

		    break;
    	}
    	case NuTo::Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_1D:
    	{
			ConstitutiveTangentLocal<1,1>& tangent(itOutput->second->AsConstitutiveTangentLocal_1x1());
 		    tangent.mTangent[0]=mE;
		    tangent.SetSymmetry(true);
    		break;
    	}
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRAIN_3D:
    	{
    		EngineeringStrain3D& engineeringStrain3D(itOutput->second->GetEngineeringStrain3D());
			engineeringStrain3D.mEngineeringStrain[0] = engineeringStrain.mEngineeringStrain;
			engineeringStrain3D.mEngineeringStrain[1] = -mNu*engineeringStrain.mEngineeringStrain;
			engineeringStrain3D.mEngineeringStrain[2] = engineeringStrain3D.mEngineeringStrain[1];
			engineeringStrain3D.mEngineeringStrain[3] = 0.;
			engineeringStrain3D.mEngineeringStrain[4] = 0.;
			engineeringStrain3D.mEngineeringStrain[5] = 0.;
    	}
    	break;
    	case NuTo::Constitutive::eOutput::ENGINEERING_PLASTIC_STRAIN_3D:
    	{
    		EngineeringStrain3D& engineeringPlasticStrain(itOutput->second->GetEngineeringStrain3D());
    		engineeringPlasticStrain.mEngineeringStrain[0] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[1] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[2] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[3] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[4] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[5] = 0.;
    		break;
    	}
    	case NuTo::Constitutive::eOutput::DAMAGE:
    	{
    		itOutput->second->GetDamage().SetDamage(0.);
    		break;
    	}
    	case NuTo::Constitutive::eOutput::UPDATE_TMP_STATIC_DATA:
    	case NuTo::Constitutive::eOutput::UPDATE_STATIC_DATA:
    	{
    	    //nothing to be done for update routine
    		break;
    	}
    	default:
    		throw MechanicsException(std::string("[NuTo::LinearElasticEngineeringStress::Evaluate1D] output object)") +
    				NuTo::Constitutive::OutputToString(itOutput->first) +
    				std::string(" could not be calculated, check the allocated material law and the section behavior."));
    	}
    }

    //update history variables but for linear elastic, there is nothing to do

	return Error::SUCCESSFUL;
}

//! @brief ... evaluate the constitutive relation in 2D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::LinearElasticEngineeringStress::Evaluate2D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::eOutput, ConstitutiveOutputBase*>& rConstitutiveOutput)
{
	// get section information determining which input on the constitutive level should be used
	const SectionBase* section(rElement->GetSection());

	// check if parameters are valid
    if (this->mParametersValid == false)
    {
   		//throw an exception giving information related to the wrong parameter
    	CheckParameters();
    }

	EngineeringStrain2D engineeringStrain;
	// calculate engineering strain
	if(rConstitutiveInput.find(NuTo::Constitutive::eInput::DEFORMATION_GRADIENT_2D)==rConstitutiveInput.end())
		throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate] deformation gradient 2d needed to evaluate engineering strain2d.");
	const DeformationGradient2D& deformationGradient(rConstitutiveInput.find(NuTo::Constitutive::eInput::DEFORMATION_GRADIENT_2D)->second->GetDeformationGradient2D());
	deformationGradient.GetEngineeringStrain(engineeringStrain);

    for (std::map<NuTo::Constitutive::eOutput, ConstitutiveOutputBase*>::iterator itOutput = rConstitutiveOutput.begin();
    		itOutput != rConstitutiveOutput.end(); itOutput++)
    {
    	switch(itOutput->first)
    	{
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_2D:
    	{
    		EngineeringStrain2D elasticEngineeringStrain(engineeringStrain);
    		// if temperature is an input, subtract thermal strains to get elastic strains
    		if (section->GetInputConstitutiveIsTemperature())
    		{
    			std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::eInput::TEMPERATURE));
    			if (itInput==rConstitutiveInput.end())
    				throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate2D] temperature needed to evaluate thermal engineering strain2d.");
    			double temperature(itInput->second->GetTemperature());
    			double deltaStrain(mThermalExpansionCoefficient * temperature);
    			EngineeringStrain2D elasticEngineeringStrain;
    			elasticEngineeringStrain.mEngineeringStrain[0] -= deltaStrain;
    			elasticEngineeringStrain.mEngineeringStrain[1] -= deltaStrain;
    		}
			EngineeringStress2D& engineeringStress(itOutput->second->GetEngineeringStress2D());
		    // calculate Engineering stress

		    switch(rElement->GetSection()->GetType())
		    {
		    case Section::PLANE_STRAIN:{
				// calculate coefficients of the material matrix
				double C11, C12, C33;
				this->CalculateCoefficients3D(C11, C12, C33);

				// calculate Engineering stress
				engineeringStress.mEngineeringStress[0] = C11 * elasticEngineeringStrain.mEngineeringStrain[0] + C12 * elasticEngineeringStrain.mEngineeringStrain[1];
				engineeringStress.mEngineeringStress[1] = C11 * elasticEngineeringStrain.mEngineeringStrain[1] + C12 * elasticEngineeringStrain.mEngineeringStrain[0];
				engineeringStress.mEngineeringStress[2] = C33 * elasticEngineeringStrain.mEngineeringStrain[2] ;
		    	break;}
		    case Section::PLANE_STRESS:{
				// calculate coefficients of the material matrix
				double C11, C12, C33;
				this->CalculateCoefficients2DPlainStress(C11, C12, C33);

				// calculate Engineering stress
				engineeringStress.mEngineeringStress[0] = C11 * elasticEngineeringStrain.mEngineeringStrain[0] + C12 * elasticEngineeringStrain.mEngineeringStrain[1];
				engineeringStress.mEngineeringStress[1] = C11 * elasticEngineeringStrain.mEngineeringStrain[1] + C12 * elasticEngineeringStrain.mEngineeringStrain[0];
				engineeringStress.mEngineeringStress[2] = C33 * elasticEngineeringStrain.mEngineeringStrain[2] ;
		    	break;}
		    default:
		    	throw MechanicsException("[NuTo::LinearElastic::GetEngineeringStressFromEngineeringStrain] Invalid type of 2D section behavoir found!!!");
		    }


		    break;
    	}
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_3D:
    	{
    		//this is for the visualize routines
    		EngineeringStrain2D elasticEngineeringStrain(engineeringStrain);
    		// if temperature is an input, subtract thermal strains to get elastic strains
    		if (section->GetInputConstitutiveIsTemperature())
    		{
    			std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::eInput::TEMPERATURE));
    			if (itInput==rConstitutiveInput.end())
    				throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate2D] temperature needed to evaluate thermal engineering strain2d.");
    			double temperature(itInput->second->GetTemperature());
    			double deltaStrain(mThermalExpansionCoefficient * temperature);
    			EngineeringStrain2D elasticEngineeringStrain;
    			elasticEngineeringStrain.mEngineeringStrain[0] -= deltaStrain;
    			elasticEngineeringStrain.mEngineeringStrain[1] -= deltaStrain;
    		}
			EngineeringStress3D& engineeringStress(itOutput->second->GetEngineeringStress3D());
		    // calculate Engineering stress

		    switch(rElement->GetSection()->GetType())
		    {
		    case Section::PLANE_STRAIN:{
				// calculate coefficients of the material matrix
				double C11, C12, C33;
				this->CalculateCoefficients3D(C11, C12, C33);

				// calculate Engineering stress
				engineeringStress.mEngineeringStress[0] = C11 * elasticEngineeringStrain.mEngineeringStrain[0] + C12 * elasticEngineeringStrain.mEngineeringStrain[1];
				engineeringStress.mEngineeringStress[1] = C11 * elasticEngineeringStrain.mEngineeringStrain[1] + C12 * elasticEngineeringStrain.mEngineeringStrain[0];
				engineeringStress.mEngineeringStress[2] = C12 * (elasticEngineeringStrain.mEngineeringStrain[0]+elasticEngineeringStrain.mEngineeringStrain[1]);
				engineeringStress.mEngineeringStress[3] = C33 * elasticEngineeringStrain.mEngineeringStrain[2] ;
				engineeringStress.mEngineeringStress[4] = 0.;
				engineeringStress.mEngineeringStress[5] = 0.;
		    	break;}
		    case Section::PLANE_STRESS:{
				// calculate coefficients of the material matrix
				double C11, C12, C33;
				this->CalculateCoefficients2DPlainStress(C11, C12, C33);

				// calculate Engineering stress
				engineeringStress.mEngineeringStress[0] = C11 * elasticEngineeringStrain.mEngineeringStrain[0] + C12 * elasticEngineeringStrain.mEngineeringStrain[1];
				engineeringStress.mEngineeringStress[1] = C11 * elasticEngineeringStrain.mEngineeringStrain[1] + C12 * elasticEngineeringStrain.mEngineeringStrain[0];
				engineeringStress.mEngineeringStress[2] = 0.;
				engineeringStress.mEngineeringStress[3] = C33 * elasticEngineeringStrain.mEngineeringStrain[2] ;
				engineeringStress.mEngineeringStress[4] = 0.;
				engineeringStress.mEngineeringStress[5] = 0.;
		    	break;}
		    default:
		    	throw MechanicsException("[NuTo::LinearElastic::GetEngineeringStressFromEngineeringStrain] Invalid type of 2D section behavoir found!!!");
		    }


		    break;
    	}
    	case NuTo::Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_2D:
    	{
			ConstitutiveTangentLocal<3,3>& tangent(itOutput->second->AsConstitutiveTangentLocal_3x3());
 		    double *data(tangent.mTangent);

 			assert(rElement->GetSection()!=0);
 		    switch(rElement->GetSection()->GetType())
 		    {
 		    case Section::PLANE_STRAIN:{
 			    // calculate coefficients of the material matrix
 			    double C11, C12, C33;
 			    this->CalculateCoefficients3D(C11, C12, C33);

 			    // store tangent at the output object
 			    data[ 0] = C11;
 			    data[ 1] = C12;
 			    data[ 2] = 0;

 			    data[ 3] = C12;
 			    data[ 4] = C11;
 			    data[ 5] = 0;

 			    data[ 6] = 0.;
 			    data[ 7] = 0.;
 			    data[ 8] = C33;
 		    	break;}
 		    case Section::PLANE_STRESS:{
 			    // calculate coefficients of the material matrix
 			    double C11, C12, C33;
 				this->CalculateCoefficients2DPlainStress(C11, C12, C33);

 			    // store tangent at the output object
 				data[ 0] = C11;
 				data[ 1] = C12;
 				data[ 2] = 0;

 				data[ 3] = C12;
 				data[ 4] = C11;
 				data[ 5] = 0;

 				data[ 6] = 0.;
 				data[ 7] = 0.;
 				data[ 8] = C33;
 		   	break;}
 		    default:
 		    	throw MechanicsException("[NuTo::LinearElastic::GetTangent_EngineeringStress_EngineeringStrain] Invalid type of 2D section behavoir found!!!");
 		    }

		    tangent.SetSymmetry(true);
    		break;
    	}
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRAIN_3D:
    	{
    		EngineeringStrain3D& engineeringStrain3D(itOutput->second->GetEngineeringStrain3D());
    	    switch(rElement->GetSection()->GetType())
    	    {
    	    case Section::PLANE_STRAIN:
    	    	engineeringStrain3D.mEngineeringStrain[0] = engineeringStrain.mEngineeringStrain[0];
    	    	engineeringStrain3D.mEngineeringStrain[1] = engineeringStrain.mEngineeringStrain[1];
    	    	engineeringStrain3D.mEngineeringStrain[2] = 0;
    	    	engineeringStrain3D.mEngineeringStrain[3] = engineeringStrain.mEngineeringStrain[2];
    	    	engineeringStrain3D.mEngineeringStrain[4] = 0.;
    	    	engineeringStrain3D.mEngineeringStrain[5] = 0.;
    	    	break;
    	    case Section::PLANE_STRESS:
    	    	engineeringStrain3D.mEngineeringStrain[0] = engineeringStrain.mEngineeringStrain[0];
    	    	engineeringStrain3D.mEngineeringStrain[1] = engineeringStrain.mEngineeringStrain[1];
    	    	engineeringStrain3D.mEngineeringStrain[2] = mNu*(mNu+1.)/(mNu-1.)*(engineeringStrain.mEngineeringStrain[0]+engineeringStrain.mEngineeringStrain[1]);
    	    	engineeringStrain3D.mEngineeringStrain[3] = engineeringStrain.mEngineeringStrain[2];
    	    	engineeringStrain3D.mEngineeringStrain[4] = 0.;
    	    	engineeringStrain3D.mEngineeringStrain[5] = 0.;
    	    	break;
    	    default:
    	    	throw MechanicsException("[NuTo::LinearElastic::GetEngineeringStrainFromEngineeringStrain] Invalid type of 2D section behavoir found!!!");
    	    }
    	}
    	break;
    	case NuTo::Constitutive::eOutput::ENGINEERING_PLASTIC_STRAIN_3D:
    	{
    		EngineeringStrain3D& engineeringPlasticStrain(itOutput->second->GetEngineeringStrain3D());
    		engineeringPlasticStrain.mEngineeringStrain[0] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[1] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[2] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[3] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[4] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[5] = 0.;
    		break;
    	}
    	case NuTo::Constitutive::eOutput::DAMAGE:
    	{
    		itOutput->second->GetDamage().SetDamage(0.);
    		break;
    	}
    	case NuTo::Constitutive::eOutput::UPDATE_TMP_STATIC_DATA:
    	case NuTo::Constitutive::eOutput::UPDATE_STATIC_DATA:
    	{
    	    //nothing to be done for update routine
    		break;
    	}
    	default:
    		throw MechanicsException(std::string("[NuTo::LinearElasticEngineeringStressEngineeringStress::Evaluate3D] output object)") +
    				NuTo::Constitutive::OutputToString(itOutput->first) +
    				std::string(" culd not be calculated, check the allocated material law and the section behavior."));
    	}
    }

    //update history variables but for linear elastic, there is nothing to do

	return Error::SUCCESSFUL;
}

//! @brief ... evaluate the constitutive relation in 1D
//! @param rElement ... element
//! @param rIp ... integration point
//! @param rConstitutiveInput ... input to the constitutive law (strain, temp gradient etc.)
//! @param rConstitutiveOutput ... output to the constitutive law (stress, stiffness, heat flux etc.)
NuTo::Error::eError NuTo::LinearElasticEngineeringStress::Evaluate3D(ElementBase* rElement, int rIp,
		const std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>& rConstitutiveInput,
		std::map<NuTo::Constitutive::eOutput, ConstitutiveOutputBase*>& rConstitutiveOutput)
{
	// get section information determining which input on the constitutive level should be used
	const SectionBase* section(rElement->GetSection());

	// check if parameters are valid
    if (this->mParametersValid == false)
    {
   		//throw an exception giving information related to the wrong parameter
    	CheckParameters();
    }

    double C11, C12, C44;
	EngineeringStrain3D engineeringStrain;
	// calculate engineering strain
	if(rConstitutiveInput.find(NuTo::Constitutive::eInput::DEFORMATION_GRADIENT_3D)==rConstitutiveInput.end())
		throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate] deformation gradient 3d needed to evaluate engineering strain3d.");
	const DeformationGradient3D& deformationGradient(rConstitutiveInput.find(NuTo::Constitutive::eInput::DEFORMATION_GRADIENT_3D)->second->GetDeformationGradient3D());
	deformationGradient.GetEngineeringStrain(engineeringStrain);

	//check, if an nonlinear iteration has to be performed, in this simple case, just calculate the linear elastic coefficients
    if (rConstitutiveOutput.find(NuTo::Constitutive::eOutput::ENGINEERING_STRESS_3D)!=rConstitutiveOutput.end()
    		|| rConstitutiveOutput.find(NuTo::Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_3D)!=rConstitutiveOutput.end())
    {
    	//in a nonlinear material routine, the Return mapping would be performed here
		// calculate coefficients of the material matrix
		this->CalculateCoefficients3D(C11, C12, C44);
    }

    for (std::map<NuTo::Constitutive::eOutput, ConstitutiveOutputBase*>::iterator itOutput = rConstitutiveOutput.begin();
    		itOutput != rConstitutiveOutput.end(); itOutput++)
    {
    	switch(itOutput->first)
    	{
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_3D:
    	{
    		EngineeringStrain3D elasticEngineeringStrain(engineeringStrain);
    		// if temperature is an input, subtract thermal strains to get elastic strains
    		if (section->GetInputConstitutiveIsTemperature())
    		{
    			std::map<NuTo::Constitutive::eInput, const ConstitutiveInputBase*>::const_iterator itInput(rConstitutiveInput.find(NuTo::Constitutive::eInput::TEMPERATURE));
    			if (itInput==rConstitutiveInput.end())
    				throw MechanicsException("[NuTo::LinearElasticEngineeringStress::Evaluate] temperature needed to evaluate thermal engineering strain3d.");
    			double temperature(itInput->second->GetTemperature());
    			double deltaStrain(mThermalExpansionCoefficient * temperature);
    			EngineeringStrain3D elasticEngineeringStrain;
    			elasticEngineeringStrain.mEngineeringStrain[0] -= deltaStrain;
    			elasticEngineeringStrain.mEngineeringStrain[1] -= deltaStrain;
    			elasticEngineeringStrain.mEngineeringStrain[2] -= deltaStrain;
    		}
			EngineeringStress3D& engineeringStress(itOutput->second->GetEngineeringStress3D());
		    // calculate Engineering stress

		    engineeringStress.mEngineeringStress[0] = C11 * elasticEngineeringStrain.mEngineeringStrain[0] + C12 * (elasticEngineeringStrain.mEngineeringStrain[1]+elasticEngineeringStrain.mEngineeringStrain[2]);
		    engineeringStress.mEngineeringStress[1] = C11 * elasticEngineeringStrain.mEngineeringStrain[1] + C12 * (elasticEngineeringStrain.mEngineeringStrain[0]+elasticEngineeringStrain.mEngineeringStrain[2]);
		    engineeringStress.mEngineeringStress[2] = C11 * elasticEngineeringStrain.mEngineeringStrain[2] + C12 * (elasticEngineeringStrain.mEngineeringStrain[0]+elasticEngineeringStrain.mEngineeringStrain[1]);
		    engineeringStress.mEngineeringStress[3] = C44 * elasticEngineeringStrain.mEngineeringStrain[3] ;
		    engineeringStress.mEngineeringStress[4] = C44 * elasticEngineeringStrain.mEngineeringStrain[4] ;
		    engineeringStress.mEngineeringStress[5] = C44 * elasticEngineeringStrain.mEngineeringStrain[5] ;
   		break;
    	}
    	case NuTo::Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN_3D:
    	{
			ConstitutiveTangentLocal<6,6>& tangent(itOutput->second->AsConstitutiveTangentLocal_6x6());
 		    double *data(tangent.mTangent);

		    // store tangent at the output object
		    data[ 0] = C11;
		    data[ 1] = C12;
		    data[ 2] = C12;
		    data[ 3] = 0;
		    data[ 4] = 0;
		    data[ 5] = 0;

		    data[ 6] = C12;
		    data[ 7] = C11;
		    data[ 8] = C12;
		    data[ 9] = 0;
		    data[10] = 0;
		    data[11] = 0;

		    data[12] = C12;
		    data[13] = C12;
		    data[14] = C11;
		    data[15] = 0;
		    data[16] = 0;
		    data[17] = 0;

		    data[18] = 0;
		    data[19] = 0;
		    data[20] = 0;
		    data[21] = C44;
		    data[22] = 0;
		    data[23] = 0;

		    data[24] = 0;
		    data[25] = 0;
		    data[26] = 0;
		    data[27] = 0;
		    data[28] = C44;
		    data[29] = 0;

		    data[30] = 0;
		    data[31] = 0;
		    data[32] = 0;
		    data[33] = 0;
		    data[34] = 0;
		    data[35] = C44;

		    tangent.SetSymmetry(true);
    		break;
    	}
    	case NuTo::Constitutive::eOutput::ENGINEERING_STRAIN_3D:
    		itOutput->second->GetEngineeringStrain3D() = engineeringStrain;
    		break;
    	case NuTo::Constitutive::eOutput::ENGINEERING_PLASTIC_STRAIN_3D:
    	{
    		EngineeringStrain3D& engineeringPlasticStrain(itOutput->second->GetEngineeringStrain3D());
    		engineeringPlasticStrain.mEngineeringStrain[0] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[1] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[2] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[3] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[4] = 0.;
    		engineeringPlasticStrain.mEngineeringStrain[5] = 0.;
    		break;
    	}
    	case NuTo::Constitutive::eOutput::DAMAGE:
    	{
    		itOutput->second->GetDamage().SetDamage(0.);
    		break;
    	}
    	case NuTo::Constitutive::eOutput::UPDATE_TMP_STATIC_DATA:
    	case NuTo::Constitutive::eOutput::UPDATE_STATIC_DATA:
    	{
    	    //nothing to be done for update routine
    		break;
    	}
    	default:
    		throw MechanicsException(std::string("[NuTo::LinearElasticEngineeringStressEngineeringStress::Evaluate3D] output object)") +
    				NuTo::Constitutive::OutputToString(itOutput->first) +
    				std::string(" culd not be calculated, check the allocated material law and the section behavior."));
    	}
    }

    //update history variables but for linear elastic, there is nothing to do

	return Error::SUCCESSFUL;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::LinearElasticEngineeringStress::AllocateStaticDataEngineeringStress_EngineeringStrain1D(const ElementBase* rElement)const
{
	return 0;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::LinearElasticEngineeringStress::AllocateStaticDataEngineeringStress_EngineeringStrain2D(const ElementBase* rElement)const
{
	return 0;
}

//! @brief ... allocate the correct static data
//! @return ... see brief explanation
NuTo::ConstitutiveStaticDataBase* NuTo::LinearElasticEngineeringStress::AllocateStaticDataEngineeringStress_EngineeringStrain3D(const ElementBase* rElement)const
{
	return 0;
}

// calculate coefficients of the material matrix
void NuTo::LinearElasticEngineeringStress::CalculateCoefficients2DPlainStress(double& C11, double& C12, double& C33) const
{
    double factor = this->mE/(1.0 - (this->mNu * this->mNu));
    C11 = factor;
    C12 = factor * this->mNu;
    C33 = factor * 0.5 * (1.0 - this->mNu);
}

// calculate coefficients of the material matrix
void NuTo::LinearElasticEngineeringStress::CalculateCoefficients3D(double& C11, double& C12, double& C44) const
{
    double factor = this->mE/((1.0 + this->mNu) * (1.0 - 2.0 * this->mNu));
    C11 = factor * (1.0 - this->mNu);
    C12 = factor * this->mNu;
    C44 = this->mE/(2.*(1.0 + this->mNu));
}

///////////////////////////////////////////////////////////////////////////

// parameters /////////////////////////////////////////////////////////////
//! @brief ... get density
//! @return ... density
double NuTo::LinearElasticEngineeringStress::GetDensity() const
{
	return this->mRho;
}

//! @brief ... set density
//! @param rRho ... density
void NuTo::LinearElasticEngineeringStress::SetDensity(double rRho)
{
    this->CheckDensity(rRho);
    this->mRho = rRho;
    this->SetParametersValid();
}

//! @brief ... get Young's modulus
//! @return ... Young's modulus
double NuTo::LinearElasticEngineeringStress::GetYoungsModulus() const
{
	return mE;
}


//! @brief ... set Young's modulus
//! @param rE ... Young's modulus
void NuTo::LinearElasticEngineeringStress::SetYoungsModulus(double rE)
{
    this->CheckYoungsModulus(rE);
    this->mE = rE;
    this->SetParametersValid();
}


//! @brief ... get Poisson's ratio
//! @return ... Poisson's ratio
double NuTo::LinearElasticEngineeringStress::GetPoissonsRatio() const
{
    return mNu;
}


//! @brief ... set Poisson's ratio
//! @param rNu ... Poisson's ratio
void NuTo::LinearElasticEngineeringStress::SetPoissonsRatio(double rNu)
{
    this->CheckPoissonsRatio(rNu);
    this->mNu = rNu;
    this->SetParametersValid();
}

//! @brief ... get thermal expansion coefficient
//! @return ... thermal expansion coefficient
double NuTo::LinearElasticEngineeringStress::GetThermalExpansionCoefficient() const
{
    return mThermalExpansionCoefficient;
}

//! @brief ... set thermal expansion coefficient
//! @param rNu ... thermal expansion coefficient
void NuTo::LinearElasticEngineeringStress::SetThermalExpansionCoefficient(double rAlpha)
{
    this->CheckThermalExpansionCoefficient(rAlpha);
    this->mThermalExpansionCoefficient = rAlpha;
    this->SetParametersValid();
}
///////////////////////////////////////////////////////////////////////////


//! @brief ... get type of constitutive relationship
//! @return ... type of constitutive relationship
//! @sa eConstitutiveType
NuTo::Constitutive::eConstitutiveType NuTo::LinearElasticEngineeringStress::GetType() const
{
    return NuTo::Constitutive::LINEAR_ELASTIC;
}


//! @brief ... check compatibility between element type and type of constitutive relationship
//! @param rElementType ... element type
//! @return ... <B>true</B> if the element is compatible with the constitutive relationship, <B>false</B> otherwise.
bool NuTo::LinearElasticEngineeringStress::CheckElementCompatibility(NuTo::Element::eElementType rElementType) const
{
    switch (rElementType)
    {
    case NuTo::Element::BRICK8N:
        return true;
    case NuTo::Element::PLANE2D3N:
        return true;
    case NuTo::Element::PLANE2D4N:
        return true;
    case NuTo::Element::PLANE2D6N:
        return true;
    case NuTo::Element::TETRAHEDRON4N:
        return true;
    case NuTo::Element::TETRAHEDRON10N:
        return true;
    case NuTo::Element::TRUSS1D2N:
        return true;
    case NuTo::Element::TRUSS1D3N:
        return true;
    default:
        return false;
    }
}

//! @brief ... check if density is positive
//! @param rRho ... density
void NuTo::LinearElasticEngineeringStress::CheckDensity(double rRho) const
{
    if (rRho < 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::LinearElasticEngineeringStress::CheckDensity] The density must be a positive value.");
    }
}

//! @brief ... check if Young's modulus is positive
//! @param rE ... Young's modulus
void NuTo::LinearElasticEngineeringStress::CheckYoungsModulus(double rE) const
{
    if (rE < 0.0)
    {
        throw NuTo::MechanicsException("[NuTo::LinearElasticEngineeringStress::CheckYoungsModulus] The Young's modulus must be a non-negative value.");
    }
}

//! @brief ... check if Poisson's ratio is valid \f$ (-1.0 < \nu < 0.5) \f$
//! @param rE ... Poisson's ratio
void NuTo::LinearElasticEngineeringStress::CheckPoissonsRatio(double rNu) const
{
    if (rNu <= -1.0)
    {
        throw NuTo::MechanicsException("[NuTo::LinearElasticEngineeringStress::CheckPoissonsRatio] Poisson's ratio must be greater or equal to -1.0.");
    }
    if (rNu >= 0.5)
    {
        throw NuTo::MechanicsException("[NuTo::LinearElasticEngineeringStress::CheckPoissonsRatio] Poisson's ratio must be smaller or equal to 0.5.");
    }
}

//! @brief ... check thermal expansion coefficient
//! @param rAlpha ... thermal expansion coefficient
void NuTo::LinearElasticEngineeringStress::CheckThermalExpansionCoefficient(double rAlpha) const
{
}

//! @brief ... print information about the object
//! @param rVerboseLevel ... verbosity of the information
void NuTo::LinearElasticEngineeringStress::Info(unsigned short rVerboseLevel, Logger& rLogger) const
{
    this->ConstitutiveBase::Info(rVerboseLevel, rLogger);
    rLogger << "    Young's modulus               : " << this->mE << "\n";
    rLogger << "    Poisson's ratio               : " << this->mNu << "\n";
    rLogger << "    Density                       : " << this->mRho << "\n";
    rLogger << "    thermal expansion coefficient : " << this->mThermalExpansionCoefficient << "\n";
}

// check parameters
void NuTo::LinearElasticEngineeringStress::CheckParameters()const
{
    this->CheckYoungsModulus(this->mE);
    this->CheckPoissonsRatio(this->mNu);
    this->CheckDensity(this->mRho);
    this->CheckThermalExpansionCoefficient(this->mThermalExpansionCoefficient);
}
