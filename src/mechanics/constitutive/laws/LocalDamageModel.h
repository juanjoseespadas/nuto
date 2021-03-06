//============================================================================
// Name        : LocalDamageModel.cpp
// Author      : Philip Huschke
// Version     : 01 Sep 2016
// Copyright   :
// Description : Local damage model
//============================================================================

#pragma once

#include "mechanics/constitutive/staticData/IPConstitutiveLaw.h"
#include "mechanics/constitutive/ConstitutiveBase.h"

namespace NuTo
{

class LocalDamageModel : public ConstitutiveBase
{

public:
    LocalDamageModel();

    typedef double StaticDataType;
    using Data = typename Constitutive::StaticData::DataContainer<double>;

    std::unique_ptr<Constitutive::IPConstitutiveLawBase> CreateIPLaw() override
    {
        return std::make_unique<Constitutive::IPConstitutiveLaw<LocalDamageModel>>(*this, 0.);
    }

    ConstitutiveInputMap GetConstitutiveInputs(const ConstitutiveOutputMap& rConstitutiveOutput) const override;

    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.).
    //! @param rConstitutiveOutput Output to the constitutive law (stress, stiffness, heat flux etc.).
    //! @param rStaticData static data.
    template <int TDim>
    void Evaluate(const ConstitutiveInputMap& rConstitutiveInput, const ConstitutiveOutputMap& rConstitutiveOutput,
                  Data& rStaticData);


    //! @brief Calculates the current static data based on the given CALCULATE_STATIC_DATA input.
    //! @param rStaticData History data.
    //! @param rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.).
    //! @return Kappa value calculated from history data.
    template <int TDim>
    double GetCurrentStaticData(Data& rStaticData, const ConstitutiveInputMap& rConstitutiveInput) const;

    //! @brief Calculates the error of the extrapolation.
    //! @param rStaticData History data.
    //! @param rConstitutiveInput Input to the constitutive law (strain, temp gradient etc.).
    //! @return Error of the extrapolation.
    double CalculateStaticDataExtrapolationError(Data& rStaticData,
                                                 const ConstitutiveInputMap& rConstitutiveInput) const;


    //! @brief ... gets a variable of the constitutive law which is selected by an enum
    //! @param rIdentifier ... Enum to identify the requested variable
    //! @return ... value of the requested variable
    virtual double GetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier) const override;

    //! @brief ... sets a variable of the constitutive law which is selected by an enum
    //! @param rIdentifier ... Enum to identify the requested variable
    //! @param rValue ... new value for requested variable
    virtual void SetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier, double rValue) override;

    //! @brief ... get type of constitutive relationship
    //! @return ... type of constitutive relationship
    //! @sa eConstitutiveType
    Constitutive::eConstitutiveType GetType() const override;

    //! @brief ... check parameters of the constitutive relationship
    void CheckParameters() const override;

    //! @brief ... determines which submatrices of a multi-doftype problem can be solved by the constitutive law
    //! @param rDofRow ... row dof
    //! @param rDofCol ... column dof
    //! @param rTimeDerivative ... time derivative
    virtual bool CheckDofCombinationComputable(Node::eDof rDofRow, Node::eDof rDofCol,
                                               int rTimeDerivative) const override;

    //! @brief ... print information about the object
    //! @param rVerboseLevel ... verbosity of the information
    //! @param rLogger stream for the output
    void Info(unsigned short rVerboseLevel, Logger& rLogger) const override;

    //! @brief ... returns true, if a material model has tmp static data (which has to be updated before stress or
    //! stiffness are calculated)
    //! @return ... see brief explanation
    bool HaveTmpStaticData() const override
    {
        return false;
    }

    void SetDamageLaw(std::shared_ptr<Constitutive::DamageLaw> damageLaw) override
    {
        mDamageLaw = damageLaw;
    }

private:
    //! @brief Density \f$ \rho \f$
    double mDensity;

    //! @brief Young's modulus \f$ E \f$
    double mYoungsModulus;

    //! @brief Poisson's ratio \f$ \nu \f$
    double mPoissonsRatio;

    //! @brief Tensile strength \f$ \sigma_t \f$
    double mTensileStrength;

    //! @brief Compressive strength \f$ \sigma_c \f$
    double mCompressiveStrength;

    //! @brief Damage law
    std::shared_ptr<NuTo::Constitutive::DamageLaw> mDamageLaw;
};
}
