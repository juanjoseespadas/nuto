#pragma once

#include "nuto/mechanics/constitutive/ConstitutiveBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveEnum.h"
#include "nuto/mechanics/constitutive/staticData/IPConstitutiveLawWithoutData.h"

#include "nuto/mechanics/MechanicsException.h"

namespace NuTo
{
class AdditiveBase : public ConstitutiveBase
{
public:
    //! @brief ctor
    AdditiveBase();

    virtual ~AdditiveBase() = default;
    AdditiveBase(const AdditiveBase&  rOther);
    AdditiveBase(      AdditiveBase&& rOther);

    AdditiveBase& operator =(const AdditiveBase&  rOther);
    AdditiveBase& operator =(      AdditiveBase&& rOther);


    //! @brief Adds a constitutive law to a model that combines multiple constitutive laws (additive, parallel)
    //! @param rConstitutiveLaw Constitutive law to be added.
    //! @param rModiesInput Enum which defines which input is modified by a constitutive law.
    virtual void AddConstitutiveLaw(NuTo::ConstitutiveBase& rConstitutiveLaw,
            Constitutive::eInput rModiesInput = Constitutive::eInput::NONE);

    //! @brief Check compatibility between element type and type of constitutive relationship.
    //! @param rElementType Element type.
    //! @return `true` if the element is compatible with the constitutive relationship, `false` otherwise.
    virtual bool CheckElementCompatibility(Element::eElementType rElementType) const override;

    //! @brief Check parameters of the constitutive relationship.
    virtual void CheckParameters() const override;

    //! @brief Checks whether a material model has tmp static data, which has to be updated before stress or
    //! stiffness are calculated.
    virtual bool HaveTmpStaticData() const override;

    //! @brief returns the sublaw with index rInted
    //! @param rIndex ... index
    //! @return reference to ip law
    Constitutive::IPConstitutiveLawBase& GetSublaw(int rIndex);


    //! @brief Determines the constitutive inputs needed to evaluate the constitutive outputs.
    //! @param rConstitutiveOutput Desired constitutive outputs.
    //! @param rInterpolationType Interpolation type to determine additional inputs.
    //! @return Constitutive inputs needed for the evaluation.
    virtual ConstitutiveInputMap GetConstitutiveInputs(const ConstitutiveOutputMap& rConstitutiveOutput,
                                                       const InterpolationType& rInterpolationType) const override;

    virtual bool CheckDofCombinationComputable(Node::eDof rDofRow, Node::eDof rDofCol,
            int rTimeDerivative) const;

protected:
    //! @brief Adds all calculable DOF combinations of an attached constitutive law to an internal storage.
    //! @param rConstitutiveLaw Constitutive law whose DOF combinations are added to additive law.
    void AddCalculableDofCombinations(NuTo::ConstitutiveBase& rConstitutiveLaw);

    //! @brief Debug variable to avoid that a constitutive law can be attached after allocation of static data.
    mutable bool mStaticDataAllocated = false;

    //! @brief Vector storing the IPConstitutiveBase sublaws.
    std::vector<std::unique_ptr<NuTo::Constitutive::IPConstitutiveLawBase>> mSublaws;

    //! @brief Vector of all the computable DOF combinations.
    std::vector<std::set<std::pair<Node::eDof,Node::eDof>>> mComputableDofCombinations;

};
}
