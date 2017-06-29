#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

#include "mechanics/constitutive/ConstitutiveEnum.h"
#include "mechanics/constitutive/staticData/IPConstitutiveLawWithoutData.h"

#include "mechanics/constitutive/laws/LinearElasticAnisotropic.h"
#include "mechanics/constitutive/laws/EngineeringStressHelper.h"
#include "base/Logger.h"
#include "base/Exception.h"

#include "mechanics/constitutive/inputoutput/ConstitutiveIOBase.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveIOMap.h"
#include "mechanics/constitutive/inputoutput/ConstitutivePlaneState.h"
#include "mechanics/constitutive/inputoutput/EngineeringStrain.h"
#include "mechanics/constitutive/inputoutput/EngineeringStress.h"

#include "mechanics/elements/ElementBase.h"
#include "mechanics/elements/ElementEnum.h"
#include "mechanics/nodes/NodeEnum.h"

#include <boost/current_function.hpp>

NuTo::LinearElasticAnisotropic::LinearElasticAnisotropic() :
        ConstitutiveBase()
{
    mStiffness << 0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 1,
                  0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0, 0;
    mRho = 0.;
    SetParametersValid();
}

//#ifdef ENABLE_SERIALIZATION
////! @brief serializes the class
////! @param ar         archive
////! @param version    version
//template<class Archive>
//void NuTo::LinearElasticAnisotropic::serialize(Archive & ar, const unsigned int version)
//{
//#ifdef DEBUG_SERIALIZATION
//    std::cout << "start serialize LinearElasticAnisotropic" << std::endl;
//#endif
//    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstitutiveBase)
//    & BOOST_SERIALIZATION_NVP(mE)
//    & BOOST_SERIALIZATION_NVP(mNu)
//    & BOOST_SERIALIZATION_NVP(mRho)
//    & BOOST_SERIALIZATION_NVP(mThermalExpansionCoefficient);
//#ifdef DEBUG_SERIALIZATION
//    std::cout << "finish serialize LinearElasticAnisotropic" << std::endl;
//#endif
//}
//BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::LinearElasticAnisotropic)
//#endif // ENABLE_SERIALIZATION

std::unique_ptr<NuTo::Constitutive::IPConstitutiveLawBase> NuTo::LinearElasticAnisotropic::CreateIPLaw()
{
    return std::make_unique<Constitutive::IPConstitutiveLawWithoutData<LinearElasticAnisotropic>>(*this);
}


NuTo::ConstitutiveInputMap NuTo::LinearElasticAnisotropic::GetConstitutiveInputs(const ConstitutiveOutputMap& rConstitutiveOutput, const InterpolationType& rInterpolationType) const
{
    ConstitutiveInputMap constitutiveInputMap;

    for (auto& itOutput : rConstitutiveOutput)
    {
        switch (itOutput.first)
        {
        case NuTo::Constitutive::eOutput::ENGINEERING_STRESS:
        case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_VISUALIZE:
        {
            constitutiveInputMap[Constitutive::eInput::ENGINEERING_STRAIN];
            break;
        }
        case NuTo::Constitutive::eOutput::ENGINEERING_STRAIN_VISUALIZE:
            constitutiveInputMap[Constitutive::eInput::ENGINEERING_STRAIN];
            break;
        // no inputs needed for:
        case NuTo::Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
        case NuTo::Constitutive::eOutput::UPDATE_TMP_STATIC_DATA:
        case NuTo::Constitutive::eOutput::UPDATE_STATIC_DATA:
            break;
        default:
            continue;
//            ProcessUnhandledOutput(__PRETTY_FUNCTION__,itOutput.first);
//            throw Exception(std::string("[")+__PRETTY_FUNCTION__+"] output object " + Constitutive::OutputToString(itOutput.first) + " cannot be calculated by this constitutive law.");
        }
    }

    return constitutiveInputMap;
}

namespace NuTo // template specialization in same namespace as definition
{
template<>
void NuTo::LinearElasticAnisotropic::Evaluate<1>(
    const ConstitutiveInputMap& rConstitutiveInput,
    const ConstitutiveOutputMap& rConstitutiveOutput)
{
    std::cout << "Not implemented" << std::endl;
}


template<>
void NuTo::LinearElasticAnisotropic::Evaluate<2>(
    const ConstitutiveInputMap& rConstitutiveInput,
    const ConstitutiveOutputMap& rConstitutiveOutput)
{
    std::cout << "Not implemented" << std::endl;
}


template<>
void NuTo::LinearElasticAnisotropic::Evaluate<3>(
    const ConstitutiveInputMap& rConstitutiveInput,
    const ConstitutiveOutputMap& rConstitutiveOutput)
{
    for (auto& itOutput : rConstitutiveOutput)
    {
        switch (itOutput.first)
        {
            case NuTo::Constitutive::eOutput::ENGINEERING_STRESS:
            case NuTo::Constitutive::eOutput::ENGINEERING_STRESS_VISUALIZE:
            {
                ConstitutiveIOBase& engineeringStress = *itOutput.second;
                engineeringStress.AssertIsVector<6>(itOutput.first, __PRETTY_FUNCTION__);

                const auto& engineeringStrain =
                    rConstitutiveInput.at(Constitutive::eInput::ENGINEERING_STRAIN)->AsEngineeringStrain3D();

                // transform vectors to eigen vectors
                // let eigen do the matrix multiply
                // and transform back: This can certainly be done better.
                Eigen::VectorXd engStress(6);
                Eigen::VectorXd engStrain(6);
                for (int ii=0; ii<6; ii++) {
                    engStrain(ii) = engineeringStrain[ii];
                }

                engStress = this->mStiffness * engStrain;

                for (int ii=0; ii<6; ii++) {
                    engineeringStress[ii] = engStress(ii);
                }

                break;
            }
            case NuTo::Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
            {
                ConstitutiveIOBase& tangent = *itOutput.second;
                tangent.AssertIsMatrix<6, 6>(itOutput.first, __PRETTY_FUNCTION__);

                for (int ii=0; ii<6; ii++) {
                    for (int jj=0; jj<6; jj++) {
                        tangent(ii,jj) = this->mStiffness(ii,jj);
                    }
                }

                break;
            }
            case NuTo::Constitutive::eOutput::ENGINEERING_STRAIN_VISUALIZE:
            {
                itOutput.second->AsEngineeringStrain3D() =
                    rConstitutiveInput.at(Constitutive::eInput::ENGINEERING_STRAIN)->AsEngineeringStrain3D();
                break;
            }
            case NuTo::Constitutive::eOutput::EXTRAPOLATION_ERROR:
                break;
//            case NuTo::Constitutive::eOutput::UPDATE_TMP_STATIC_DATA:
//            case NuTo::Constitutive::eOutput::UPDATE_STATIC_DATA:
//            {
//                //nothing to be done for update routine
//                continue;
//            }
            default:
                continue;
        }
        itOutput.second->SetIsCalculated(true);
    }
}

} // namespace NuTo


bool NuTo::LinearElasticAnisotropic::CheckDofCombinationComputable(NuTo::Node::eDof rDofRow,
                                                                          NuTo::Node::eDof rDofCol,
                                                                          int rTimeDerivative) const
{
    assert(rTimeDerivative == 0 or rTimeDerivative == 1 or rTimeDerivative == 2);
    switch (rTimeDerivative)
    {
    case 0:
    case 1:
    case 2:
    {
        switch (Node::CombineDofs(rDofRow, rDofCol))
        {
        case Node::CombineDofs(Node::eDof::DISPLACEMENTS, Node::eDof::DISPLACEMENTS):
            return true;
        default:
            return false;
        }

    }
        break;
    default:
        return false;
    }
}


bool NuTo::LinearElasticAnisotropic::CheckHaveParameter(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    switch(rIdentifier)
    {
        case Constitutive::eConstitutiveParameter::DENSITY:
        case Constitutive::eConstitutiveParameter::STIFFNESS:
        {
            return true;
        }
        default:
        {
            return false;
        }
    }
}


double NuTo::LinearElasticAnisotropic::GetParameterDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    switch(rIdentifier)
    {
    case Constitutive::eConstitutiveParameter::DENSITY:
        return this->mRho;
    default:
        throw Exception(__PRETTY_FUNCTION__, "Constitutive law does not have the requested variable");
    }
}


void NuTo::LinearElasticAnisotropic::SetParameterDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier, double rValue)
{
    ConstitutiveBase::CheckParameterDouble(rIdentifier, rValue);
    switch(rIdentifier)
    {
    case Constitutive::eConstitutiveParameter::DENSITY:
        this->mRho = rValue;
        break;
    default:
        throw Exception(__PRETTY_FUNCTION__, "Constitutive law does not have the requested variable");
    }
}

Eigen::VectorXd NuTo::LinearElasticAnisotropic::GetParameterFullVectorDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier) const
{
    switch(rIdentifier)
    {
    case Constitutive::eConstitutiveParameter::STIFFNESS:
    {
        Eigen::VectorXd stiffnessFlattened(36);
        int count = 0;
        for (int ii=0; ii< 6; ii++) {
            for (int jj=0; jj< 6; jj++) {
                stiffnessFlattened(count) = this->mStiffness(ii,jj);
                count++;
            }
        }
        return stiffnessFlattened;
    }
    default:
        throw Exception(__PRETTY_FUNCTION__, "Constitutive law does not have the requested variable");
    }
}

void NuTo::LinearElasticAnisotropic::SetParameterFullVectorDouble(NuTo::Constitutive::eConstitutiveParameter rIdentifier, Eigen::VectorXd rValue)
{
    //ConstitutiveBase::CheckParameterFullVector(rIdentifier, rValue);
    switch(rIdentifier)
    {
    case Constitutive::eConstitutiveParameter::STIFFNESS:
    {
        int count = 0;
        for (int ii=0; ii< 6; ii++) {
            for (int jj=0; jj< 6; jj++) {
                this->mStiffness(ii,jj) = rValue(count);
                count++;
            }
        }
        break;
    }
    default:
        throw Exception(__PRETTY_FUNCTION__, "Constitutive law does not have the requested variable");
    }
}

bool NuTo::LinearElasticAnisotropic::CheckOutputTypeCompatibility(NuTo::Constitutive::eOutput rOutputEnum) const
{
    switch (rOutputEnum)
    {
    case Constitutive::eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
    case Constitutive::eOutput::ENGINEERING_STRAIN_VISUALIZE:
    case Constitutive::eOutput::ENGINEERING_STRESS:
    case Constitutive::eOutput::ENGINEERING_STRESS_VISUALIZE:
//    case Constitutive::eOutput::UPDATE_STATIC_DATA:
//    case Constitutive::eOutput::UPDATE_TMP_STATIC_DATA:
    {
        return true;
    }
    default:
    {
        return false;
    }
    }
}


NuTo::Constitutive::eConstitutiveType NuTo::LinearElasticAnisotropic::GetType() const
{
    return NuTo::Constitutive::eConstitutiveType::LINEAR_ELASTIC_ANISOTROPIC;
}


void NuTo::LinearElasticAnisotropic::Info(unsigned short rVerboseLevel, Logger& rLogger) const
{
    this->ConstitutiveBase::Info(rVerboseLevel, rLogger);
    rLogger << "    Stiffness                     :\n " << this->mStiffness << "\n";
    rLogger << "    Density                       : " << this->mRho << "\n";
}


void NuTo::LinearElasticAnisotropic::CheckParameters() const
{
    ConstitutiveBase::CheckParameterDouble(Constitutive::eConstitutiveParameter::DENSITY, mRho);
    // Check stiffness tensor components
    // Symmetry test
    for (int ii=0; ii<6; ii++) {
        for (int jj=0; jj<ii; jj++) {
            if (mStiffness(ii,jj) != mStiffness(jj,ii)) {
                throw NuTo::Exception(BOOST_CURRENT_FUNCTION , "Stiffness must be symmetric (entry ["
                                               + std::to_string(ii) + "," + std::to_string(jj) + "] = "
                                               + std::to_string(mStiffness(ii,jj)) + "\n"
                                               + "(entry [" + std::to_string(jj) + "," + std::to_string(ii) + "] = "
                                               + std::to_string(mStiffness(jj,ii)) + ".");
            }
        }
    }
}
