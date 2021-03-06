#include "mechanics/constitutive/laws/LocalDamageModel.h"

#include "mechanics/constitutive/ConstitutiveEnum.h"
#include "base/Logger.h"
#include "base/Exception.h"
#include "mechanics/elements/ElementBase.h"

#include "mechanics/constitutive/inputoutput/ConstitutiveCalculateStaticData.h"
#include "mechanics/constitutive/inputoutput/ConstitutivePlaneState.h"
#include "mechanics/nodes/NodeEnum.h"
#include "mechanics/structures/StructureBase.h"
#include "mechanics/constitutive/laws/EngineeringStressHelper.h"
#include "mechanics/constitutive/inputoutput/EquivalentStrain.h"
#include "mechanics/constitutive/damageLaws/DamageLaw.h"


using NuTo::Constitutive::eConstitutiveParameter;
using NuTo::Constitutive::eOutput;
using NuTo::Constitutive::eInput;
using NuTo::Constitutive::eConstitutiveType;


NuTo::LocalDamageModel::LocalDamageModel()
    : ConstitutiveBase()
    , mDensity(0.)
    , mYoungsModulus(0.)
    , mPoissonsRatio(0.)
    , mTensileStrength(0.)
    , mCompressiveStrength(0.)
    , mDamageLaw()
{
}


double NuTo::LocalDamageModel::GetParameterDouble(eConstitutiveParameter rIdentifier) const
{
    CheckParameters();

    switch (rIdentifier)
    {
    case eConstitutiveParameter::DENSITY:
        return mDensity;

    case eConstitutiveParameter::YOUNGS_MODULUS:
        return mYoungsModulus;

    case eConstitutiveParameter::POISSONS_RATIO:
        return mPoissonsRatio;

    case eConstitutiveParameter::TENSILE_STRENGTH:
        return mTensileStrength;

    case eConstitutiveParameter::COMPRESSIVE_STRENGTH:
        return mCompressiveStrength;

    default:
        throw Exception(__PRETTY_FUNCTION__, "Constitutive law does not have the requested variable");
    }
}

void NuTo::LocalDamageModel::SetParameterDouble(eConstitutiveParameter rIdentifier, double rValue)
{
    switch (rIdentifier)
    {
    case eConstitutiveParameter::COMPRESSIVE_STRENGTH:
        mCompressiveStrength = rValue;
        break;

    case eConstitutiveParameter::DENSITY:
        mDensity = rValue;
        break;

    case eConstitutiveParameter::YOUNGS_MODULUS:
        mYoungsModulus = rValue;
        break;

    case eConstitutiveParameter::POISSONS_RATIO:
        mPoissonsRatio = rValue;
        break;

    case eConstitutiveParameter::TENSILE_STRENGTH:
        mTensileStrength = rValue;
        break;

    default:
        throw Exception(__PRETTY_FUNCTION__, "Constitutive law does not have the requested variable");
    }
}

//! @brief ... get type of constitutive relationship
eConstitutiveType NuTo::LocalDamageModel::GetType() const
{
    return eConstitutiveType::LOCAL_DAMAGE_MODEL;
}


//! @brief ... print information about the object
//! @param rVerboseLevel ... verbosity of the information
void NuTo::LocalDamageModel::Info(unsigned short, Logger& rLogger) const
{
    rLogger << "Info function not yet implemented"
            << "\n";
}


void NuTo::LocalDamageModel::CheckParameters() const
{
}

namespace NuTo
{

template <>
void NuTo::LocalDamageModel::Evaluate<1>(const ConstitutiveInputMap& rConstitutiveInput,
                                         const ConstitutiveOutputMap& rConstitutiveOutput, Data& rStaticData)
{
    throw Exception(__PRETTY_FUNCTION__, "IMPLEMENT ME!");

    double kappa = GetCurrentStaticData<1>(rStaticData, rConstitutiveInput);

    // get constitutive inputs
    const auto& elasticEngineeringStrain = rConstitutiveInput.at(eInput::ENGINEERING_STRAIN)->AsEngineeringStrain2D();

    const auto& planeState =
            *dynamic_cast<ConstitutivePlaneState*>(rConstitutiveInput.at(Constitutive::eInput::PLANE_STATE).get());

    EquivalentStrainModifiedMises<2> eeq(elasticEngineeringStrain, mCompressiveStrength / mTensileStrength,
                                         mPoissonsRatio, planeState.GetPlaneState());
    double localEqStrain = eeq.Get();

    double omega = mDamageLaw->CalculateDamage(kappa);

    bool performUpdateAtEnd = false;

    // calculate coefficients
    double C11, C12, C33;
    switch (planeState.GetPlaneState())
    {
    case ePlaneState::PLANE_STRAIN:
        std::tie(C11, C12, C33) = EngineeringStressHelper::CalculateCoefficients3D(mYoungsModulus, mPoissonsRatio);
        break;
    case ePlaneState::PLANE_STRESS:
        std::tie(C11, C12, C33) =
                EngineeringStressHelper::CalculateCoefficients2DPlaneStress(mYoungsModulus, mPoissonsRatio);
        break;
    default:
        throw Exception(__PRETTY_FUNCTION__, "Invalid type of 2D section behavior found.");
    }


    /////////////////////////////////////////////////
    //         LOOP OVER OUTPUT REQUESTS           //
    /////////////////////////////////////////////////

    for (const auto& itOutput : rConstitutiveOutput)
    {
        switch (itOutput.first)
        {

        case eOutput::ENGINEERING_STRESS:
        {
            ConstitutiveIOBase& engineeringStress = *itOutput.second;
            engineeringStress.AssertIsVector<3>(itOutput.first, __PRETTY_FUNCTION__);
            engineeringStress[0] =
                    (1 - omega) * (C11 * elasticEngineeringStrain[0] + C12 * elasticEngineeringStrain[1]);
            engineeringStress[1] =
                    (1 - omega) * (C11 * elasticEngineeringStrain[1] + C12 * elasticEngineeringStrain[0]);
            engineeringStress[2] = (1 - omega) * C33 * elasticEngineeringStrain[2];
            break;
        }

        case eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
        {
            ConstitutiveIOBase& tangent = *itOutput.second;
            tangent.AssertIsMatrix<3, 3>(itOutput.first, __PRETTY_FUNCTION__);

            double dDamageDKappa = mDamageLaw->CalculateDerivative(kappa);
            auto dLocalEqStrainDStrain = eeq.GetDerivative();

            auto itCalculateStaticData = rConstitutiveInput.find(Constitutive::eInput::CALCULATE_STATIC_DATA);
            const auto& calculateStaticData =
                    *static_cast<const ConstitutiveCalculateStaticData*>(itCalculateStaticData->second.get());

            if (calculateStaticData.GetCalculateStaticData() == eCalculateStaticData::EULER_FORWARD)
                dDamageDKappa = 0.;

            if (localEqStrain < kappa)
                dDamageDKappa = 0.; // set zero as well, corrresponds to
            // dKappa / dLocalEqStrain


            std::vector<double> effectiveStress(3);
            effectiveStress[0] = (C11 * elasticEngineeringStrain[0] + C12 * elasticEngineeringStrain[1]);
            effectiveStress[1] = (C11 * elasticEngineeringStrain[1] + C12 * elasticEngineeringStrain[0]);
            effectiveStress[2] = C33 * elasticEngineeringStrain[2];

            // right coefficients are calculated above
            tangent(0, 0) = (1 - omega) * C11 - dDamageDKappa * dLocalEqStrainDStrain[0] * effectiveStress[0];
            tangent(1, 0) = (1 - omega) * C12 - dDamageDKappa * dLocalEqStrainDStrain[0] * effectiveStress[1];
            tangent(2, 0) = 0. - dDamageDKappa * dLocalEqStrainDStrain[0] * effectiveStress[2];

            tangent(0, 1) = (1 - omega) * C12 - dDamageDKappa * dLocalEqStrainDStrain[1] * effectiveStress[0];
            tangent(1, 1) = (1 - omega) * C11 - dDamageDKappa * dLocalEqStrainDStrain[1] * effectiveStress[1];
            tangent(2, 1) = 0. - dDamageDKappa * dLocalEqStrainDStrain[1] * effectiveStress[2];

            tangent(0, 2) = 0. - dDamageDKappa * dLocalEqStrainDStrain[2] * effectiveStress[0];
            tangent(1, 2) = 0. - dDamageDKappa * dLocalEqStrainDStrain[2] * effectiveStress[1];
            tangent(2, 2) = (1 - omega) * C33 - dDamageDKappa * dLocalEqStrainDStrain[2] * effectiveStress[2];
            break;
        }

        case eOutput::DAMAGE:
        {
            ConstitutiveIOBase& damage = *itOutput.second;
            damage.AssertIsScalar(itOutput.first, __PRETTY_FUNCTION__);

            damage[0] = omega;
            break;
        }
        case eOutput::EXTRAPOLATION_ERROR:
        {
            ConstitutiveIOBase& error = *itOutput.second;
            error[0] = CalculateStaticDataExtrapolationError(rStaticData, rConstitutiveInput);
            break;
        }
        case eOutput::UPDATE_TMP_STATIC_DATA:
        {
            throw Exception(__PRETTY_FUNCTION__,
                            "tmp_static_data has to be updated without any other outputs, call it separately.");
        }
            continue;
        case eOutput::UPDATE_STATIC_DATA:
        {
            performUpdateAtEnd = true;
        }
            continue;

        default:
            continue;
        }
        itOutput.second->SetIsCalculated(true);
    }

    // update history variables
    if (performUpdateAtEnd)
        rStaticData.GetData() = kappa;
}

template <>
void NuTo::LocalDamageModel::Evaluate<2>(const ConstitutiveInputMap& rConstitutiveInput,
                                         const ConstitutiveOutputMap& rConstitutiveOutput, Data& rStaticData)
{
    double kappa = GetCurrentStaticData<2>(rStaticData, rConstitutiveInput);


    // get constitutive inputs
    const auto& strainEl = rConstitutiveInput.at(eInput::ENGINEERING_STRAIN)->AsEngineeringStrain2D();

    const auto& planeState =
            *dynamic_cast<ConstitutivePlaneState*>(rConstitutiveInput.at(Constitutive::eInput::PLANE_STATE).get());

    EquivalentStrainModifiedMises<2> eeq(strainEl, mCompressiveStrength / mTensileStrength, mPoissonsRatio,
                                         planeState.GetPlaneState());
    double localEqStrain = eeq.Get();

    double omega = mDamageLaw->CalculateDamage(kappa);

    bool performUpdateAtEnd = false;

    // calculate coefficients
    double C11, C12, C33;
    switch (planeState.GetPlaneState())
    {
    case ePlaneState::PLANE_STRAIN:
        std::tie(C11, C12, C33) = EngineeringStressHelper::CalculateCoefficients3D(mYoungsModulus, mPoissonsRatio);
        break;
    case ePlaneState::PLANE_STRESS:
        std::tie(C11, C12, C33) =
                EngineeringStressHelper::CalculateCoefficients2DPlaneStress(mYoungsModulus, mPoissonsRatio);
        break;
    default:
        throw Exception(__PRETTY_FUNCTION__, "Invalid type of 2D section behavior found.");
    }


    /////////////////////////////////////////////////
    //         LOOP OVER OUTPUT REQUESTS           //
    /////////////////////////////////////////////////

    for (const auto& itOutput : rConstitutiveOutput)
    {
        switch (itOutput.first)
        {

        case eOutput::ENGINEERING_STRESS:
        {
            ConstitutiveIOBase& engineeringStress = *itOutput.second;
            engineeringStress.AssertIsVector<3>(itOutput.first, __PRETTY_FUNCTION__);
            engineeringStress[0] = (1 - omega) * (C11 * strainEl[0] + C12 * strainEl[1]);
            engineeringStress[1] = (1 - omega) * (C11 * strainEl[1] + C12 * strainEl[0]);
            engineeringStress[2] = (1 - omega) * C33 * strainEl[2];
            break;
        }

        case eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
        {
            Eigen::Matrix3d& tangent = dynamic_cast<Eigen::Matrix3d&>(*itOutput.second);

            double dDamageDKappa = mDamageLaw->CalculateDerivative(kappa);
            auto dLocalEqStrainDStrain = eeq.GetDerivative();

            auto itCalculateStaticData = rConstitutiveInput.find(Constitutive::eInput::CALCULATE_STATIC_DATA);
            const auto& calculateStaticData =
                    *static_cast<const ConstitutiveCalculateStaticData*>(itCalculateStaticData->second.get());

            if (calculateStaticData.GetCalculateStaticData() == eCalculateStaticData::EULER_FORWARD)
                dDamageDKappa = 0.;

            if (localEqStrain < kappa)
                dDamageDKappa = 0.; // set zero as well, corrresponds to
            // dKappa / dLocalEqStrain

            Eigen::Vector3d effectiveStress;
            effectiveStress[0] = (C11 * strainEl[0] + C12 * strainEl[1]);
            effectiveStress[1] = (C11 * strainEl[1] + C12 * strainEl[0]);
            effectiveStress[2] = C33 * strainEl[2];


            Eigen::Matrix3d tangentElastic = Eigen::Matrix3d::Zero();
            tangentElastic(0, 0) = C11;
            tangentElastic(1, 0) = C12;
            tangentElastic(0, 1) = C12;
            tangentElastic(1, 1) = C11;
            tangentElastic(2, 2) = C33;

            tangent = (1 - omega) * tangentElastic -
                      dDamageDKappa * (effectiveStress * dLocalEqStrainDStrain.transpose());
            break;
        }

        case eOutput::DAMAGE:
        {
            ConstitutiveIOBase& damage = *itOutput.second;
            damage.AssertIsScalar(itOutput.first, __PRETTY_FUNCTION__);

            damage[0] = omega;
            break;
        }
        case eOutput::EXTRAPOLATION_ERROR:
        {
            ConstitutiveIOBase& error = *itOutput.second;
            error[0] = CalculateStaticDataExtrapolationError(rStaticData, rConstitutiveInput);
            break;
        }
        case eOutput::UPDATE_TMP_STATIC_DATA:
        {
            throw Exception(__PRETTY_FUNCTION__,
                            "tmp_static_data has to be updated without any other outputs, call it separately.");
        }
            continue;
        case eOutput::UPDATE_STATIC_DATA:
        {
            performUpdateAtEnd = true;
        }
            continue;

        default:
            continue;
        }
        itOutput.second->SetIsCalculated(true);
    }

    // update history variables
    if (performUpdateAtEnd)
        rStaticData.GetData() = kappa;
}

template <>
void NuTo::LocalDamageModel::Evaluate<3>(const ConstitutiveInputMap& rConstitutiveInput,
                                         const ConstitutiveOutputMap& rConstitutiveOutput, Data& rStaticData)
{
    double kappa = GetCurrentStaticData<3>(rStaticData, rConstitutiveInput);
    double omega = mDamageLaw->CalculateDamage(kappa);

    // get constitutive inputs
    const auto& strainEl = rConstitutiveInput.at(eInput::ENGINEERING_STRAIN)->AsEngineeringStrain3D();

    EquivalentStrainModifiedMises<3> eeq(strainEl, mCompressiveStrength / mTensileStrength, mPoissonsRatio);
    double localEqStrain = eeq.Get();

    bool performUpdateAtEnd = false;

    // calculate coefficients
    double C11 = 0, C12 = 0, C44 = 0;
    std::tie(C11, C12, C44) = EngineeringStressHelper::CalculateCoefficients3D(mYoungsModulus, mPoissonsRatio);

    /////////////////////////////////////////////////
    //         LOOP OVER OUTPUT REQUESTS           //
    /////////////////////////////////////////////////

    for (const auto& itOutput : rConstitutiveOutput)
    {
        switch (itOutput.first)
        {

        case eOutput::ENGINEERING_STRESS_VISUALIZE:
        case eOutput::ENGINEERING_STRESS:
        {
            ConstitutiveIOBase& stress = *itOutput.second;
            stress.AssertIsVector<6>(itOutput.first, __PRETTY_FUNCTION__);
            stress[0] = (1. - omega) * (C11 * strainEl[0] + C12 * (strainEl[1] + strainEl[2]));
            stress[1] = (1. - omega) * (C11 * strainEl[1] + C12 * (strainEl[0] + strainEl[2]));
            stress[2] = (1. - omega) * (C11 * strainEl[2] + C12 * (strainEl[0] + strainEl[1]));
            stress[3] = (1. - omega) * (C44 * strainEl[3]);
            stress[4] = (1. - omega) * (C44 * strainEl[4]);
            stress[5] = (1. - omega) * (C44 * strainEl[5]);
            break;
        }

        case eOutput::D_ENGINEERING_STRESS_D_ENGINEERING_STRAIN:
        {
            Eigen::Matrix<double, 6, 6>& tangent = dynamic_cast<Eigen::Matrix<double, 6, 6>&>(*itOutput.second);

            double dDamageDKappa = mDamageLaw->CalculateDerivative(kappa);
            auto dLocalEqStrainDStrain = eeq.GetDerivative();

            auto itCalculateStaticData = rConstitutiveInput.find(Constitutive::eInput::CALCULATE_STATIC_DATA);
            const auto& calculateStaticData =
                    *static_cast<const ConstitutiveCalculateStaticData*>(itCalculateStaticData->second.get());

            if (calculateStaticData.GetCalculateStaticData() == eCalculateStaticData::EULER_FORWARD)
                dDamageDKappa = 0.;

            if (localEqStrain < kappa)
                dDamageDKappa = 0.; // set zero as well, corrresponds to
            // dKappa / dLocalEqStrain


            Eigen::Matrix<double, 6, 1> effectiveStress;
            effectiveStress[0] = C11 * strainEl[0] + C12 * (strainEl[1] + strainEl[2]);
            effectiveStress[1] = C11 * strainEl[1] + C12 * (strainEl[0] + strainEl[2]);
            effectiveStress[2] = C11 * strainEl[2] + C12 * (strainEl[0] + strainEl[1]);
            effectiveStress[3] = C44 * strainEl[3];
            effectiveStress[4] = C44 * strainEl[4];
            effectiveStress[5] = C44 * strainEl[5];


            Eigen::Matrix<double, 6, 6> tangentElastic = Eigen::Matrix<double, 6, 6>::Zero();

            // C11 diagonal:
            tangentElastic(0, 0) = C11;
            tangentElastic(1, 1) = C11;
            tangentElastic(2, 2) = C11;

            // C12 off diagonals:
            tangentElastic(0, 1) = C12;
            tangentElastic(0, 2) = C12;
            tangentElastic(1, 0) = C12;
            tangentElastic(1, 2) = C12;
            tangentElastic(2, 0) = C12;
            tangentElastic(2, 1) = C12;

            // C44 diagonal:
            tangentElastic(3, 3) = C44;
            tangentElastic(4, 4) = C44;
            tangentElastic(5, 5) = C44;

            tangent =
                    (1 - omega) * tangentElastic - effectiveStress * dDamageDKappa * dLocalEqStrainDStrain.transpose();
            break;
        }

        case eOutput::DAMAGE:
        {
            ConstitutiveIOBase& damage = *itOutput.second;
            damage.AssertIsScalar(itOutput.first, __PRETTY_FUNCTION__);

            damage[0] = omega;
            break;
        }
        case NuTo::Constitutive::eOutput::LOCAL_EQ_STRAIN:
        {
            (*itOutput.second)[0] = localEqStrain;
            break;
        }
        case NuTo::Constitutive::eOutput::ENGINEERING_STRAIN_VISUALIZE:
        {
            itOutput.second->AsEngineeringStrain3D() =
                    rConstitutiveInput.at(Constitutive::eInput::ENGINEERING_STRAIN)->AsEngineeringStrain3D();
            break;
        }
        case eOutput::EXTRAPOLATION_ERROR:
        {
            ConstitutiveIOBase& error = *itOutput.second;
            error[0] = CalculateStaticDataExtrapolationError(rStaticData, rConstitutiveInput);
            break;
        }
        case eOutput::UPDATE_TMP_STATIC_DATA:
        {
            throw Exception(__PRETTY_FUNCTION__,
                            "tmp_static_data has to be updated without any other outputs, call it separately.");
        }
        case eOutput::UPDATE_STATIC_DATA:
        {
            performUpdateAtEnd = true;
            continue;
        }

        default:
            continue;
        }
        itOutput.second->SetIsCalculated(true);
    }

    // update history variables
    if (performUpdateAtEnd)
        rStaticData.GetData() = kappa;
}

} // namespace NuTo

NuTo::ConstitutiveInputMap NuTo::LocalDamageModel::GetConstitutiveInputs(const ConstitutiveOutputMap&) const
{
    ConstitutiveInputMap constitutiveInputMap;
    constitutiveInputMap[eInput::ENGINEERING_STRAIN];
    return constitutiveInputMap;
}

template <int TDim>
double NuTo::LocalDamageModel::GetCurrentStaticData(Data& rStaticData,
                                                    const ConstitutiveInputMap& rConstitutiveInput) const
{
    auto itCalculateStaticData = rConstitutiveInput.find(Constitutive::eInput::CALCULATE_STATIC_DATA);
    if (itCalculateStaticData == rConstitutiveInput.end())
        throw Exception(__PRETTY_FUNCTION__,
                        "You need to specify the way the static data should be calculated (input list).");

    const auto& calculateStaticData =
            *static_cast<const ConstitutiveCalculateStaticData*>(itCalculateStaticData->second.get());

    switch (calculateStaticData.GetCalculateStaticData())
    {
    case eCalculateStaticData::USE_PREVIOUS:
    {
        int index = calculateStaticData.GetIndexOfPreviousStaticData();
        return rStaticData.GetData(index);
    }
    case eCalculateStaticData::EULER_BACKWARD:
    {
        double localEqStrain = 0;
        const auto& elasticEngineeringStrain =
                rConstitutiveInput.at(eInput::ENGINEERING_STRAIN)->AsEngineeringStrain<TDim>();

        if (TDim == 2)
        {
            const auto& planeState = *dynamic_cast<ConstitutivePlaneState*>(
                    rConstitutiveInput.at(Constitutive::eInput::PLANE_STATE).get());
            localEqStrain = EquivalentStrainModifiedMises<TDim>(elasticEngineeringStrain,
                                                                mCompressiveStrength / mTensileStrength, mPoissonsRatio,
                                                                planeState.GetPlaneState())
                                    .Get();
        }
        else
        {
            localEqStrain = EquivalentStrainModifiedMises<TDim>(elasticEngineeringStrain,
                                                                mCompressiveStrength / mTensileStrength, mPoissonsRatio)
                                    .Get();
        }


        int index = calculateStaticData.GetIndexOfPreviousStaticData();
        double oldKappa = rStaticData.GetData(index);
        return std::max(localEqStrain, oldKappa);
    }

    case eCalculateStaticData::EULER_FORWARD:
    {
        auto itTimeStep = rConstitutiveInput.find(Constitutive::eInput::TIME_STEP);
        if (itTimeStep == rConstitutiveInput.end())
            throw Exception(__PRETTY_FUNCTION__, "TimeStep input needed for EULER_FORWARD.");
        const auto& timeStep = *(itTimeStep->second);

        assert(rStaticData.GetNumData() >= 2);

        return ConstitutiveCalculateStaticData::EulerForward(rStaticData.GetData(1), rStaticData.GetData(2), timeStep);
    }

    default:
        throw Exception(__PRETTY_FUNCTION__, "Cannot calculate the static data in the requested way.");
    }
}


double
NuTo::LocalDamageModel::CalculateStaticDataExtrapolationError(Data& rStaticData,
                                                              const ConstitutiveInputMap& rConstitutiveInput) const
{
    return 0;
    // static data 0 contains the extrapolated values \tilde \kappa_n
    // static data 1 contains the implicit data \kappa_n-1
    // static data 2 contains the implicit data \kappa_n-2

    const auto& planeState =
            *dynamic_cast<ConstitutivePlaneState*>(rConstitutiveInput.at(Constitutive::eInput::PLANE_STATE).get());

    const auto& elasticEngineeringStrain = rConstitutiveInput.at(eInput::ENGINEERING_STRAIN)->AsEngineeringStrain2D();
    EquivalentStrainModifiedMises<2> eeq(elasticEngineeringStrain, mCompressiveStrength / mTensileStrength,
                                         mPoissonsRatio, planeState.GetPlaneState());
    double localEqStrain = eeq.Get();
    //    double k_n_t = rStaticData.GetData(0);
    double k_nn = rStaticData.GetData(1);
    double k_n = std::max(localEqStrain, k_nn); // calculate kappa implicitly

    return (k_n - k_nn) / k_n;
}


bool NuTo::LocalDamageModel::CheckDofCombinationComputable(NuTo::Node::eDof rDofRow, NuTo::Node::eDof rDofCol,
                                                           int rTimeDerivative) const
{
    return rTimeDerivative < 1 && rDofRow == Node::eDof::DISPLACEMENTS && rDofCol == Node::eDof::DISPLACEMENTS;
}
