#pragma once

#include "mechanics/constitutive/ConstitutiveBase.h"
#include "mechanics/constitutive/staticData/IPConstitutiveLawWithoutData.h"


namespace NuTo
{
//! Isotropic material with linear relation between electric field E and dielectric displacement D

//! Relation to the anisotropic case:
//! \f$D_i = \epsilon_{ij} E_j \f$, with \f$\epsilon_{ij} = \epsilon \delta_{ij}\f$
class LinearDielectric : public ConstitutiveBase
{

public:
    LinearDielectric();

    std::unique_ptr<Constitutive::IPConstitutiveLawBase> CreateIPLaw() override
    {
        return std::make_unique<Constitutive::IPConstitutiveLawWithoutData<LinearDielectric>>(*this);
    }


    ConstitutiveInputMap GetConstitutiveInputs(const ConstitutiveOutputMap& rConstitutiveOutput) const override;

    //! @brief Evaluate the constitutive relation.
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output to the constitutive law
    template <int TDim>
    void Evaluate(const ConstitutiveInputMap& rConstitutiveInput, const ConstitutiveOutputMap& rConstitutiveOutput);

    //! @brief ... determines which submatrices of a multi-doftype problem can be solved by the constitutive law
    //! @param rDofRow ... row dof
    //! @param rDofCol ... column dof
    //! @param rTimeDerivative ... time derivative
    virtual bool CheckDofCombinationComputable(Node::eDof rDofRow, Node::eDof rDofCol,
                                               int rTimeDerivative) const override;

    //! @brief Checks if the constitutive law has a specific parameter.
    //! @param rIdentifier Enum to identify the requested parameter
    virtual bool CheckHaveParameter(Constitutive::eConstitutiveParameter rIdentifier) const override;

    //! @brief Gets a parameter of the constitutive law which is selected by an enum.
    //! @param rIdentifier Enum to identify the requested parameter
    //! @return Value of the requested variable
    virtual Eigen::MatrixXd GetParameterMatrixDouble(Constitutive::eConstitutiveParameter rIdentifier) const override;

    //! @brief Sets a parameter of the constitutive law which is selected by an enum.
    //! @param rIdentifier Enum to identify the requested parameter
    //! @param rValue New value for requested variable
    virtual void SetParameterMatrixDouble(Constitutive::eConstitutiveParameter rIdentifier,
                                          Eigen::MatrixXd rValue) override;

    //! @brief Sets a parameter of the constitutive law which is selected by an enum.
    //! @param rIdentifier Enum to identify the requested parameter
    //! @param rValue New value for requested variable
    virtual void SetParameterDouble(Constitutive::eConstitutiveParameter rIdentifier, double rValue) override;

    //! @brief Gets a set of all constitutive output enums that are compatible with the constitutive law.
    //! @return Set of all constitutive output enums that are compatible with the constitutive law
    virtual bool CheckOutputTypeCompatibility(Constitutive::eOutput rOutputEnum) const override;

    //! @brief Get type of constitutive relationship.
    //! @return Type of constitutive relationship
    //! @sa eConstitutiveType
    Constitutive::eConstitutiveType GetType() const override;

    //! @brief Check parameters of the constitutive relationship.
    void CheckParameters() const override;

    //! @brief Print information about the object.
    //! @param rVerboseLevel Verbosity of the information
    //! @param rLogger Stream for the output
    void Info(unsigned short rVerboseLevel, Logger& rLogger) const override;

    //! @brief Returns true, if a material model has tmp static data (which has to be updated before stress or stiffness
    //! are calculated).
    bool HaveTmpStaticData() const override
    {
        return false;
    }


protected:
    Eigen::Matrix3d mPermittivity;

    template <int TDim>
    struct InputData
    {
        Eigen::Matrix<double, TDim, 1> mElectricField;
    };
};
}
