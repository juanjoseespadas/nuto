#pragma once

#include "mechanics/constitutive/staticData/IPConstitutiveLawBase.h"
#include <type_traits>

namespace NuTo
{
namespace Constitutive
{

template <typename TLaw>
class IPConstitutiveLawWithoutData : public IPConstitutiveLawBase
{
public:
    static_assert(std::is_base_of<ConstitutiveBase, TLaw>::value, "TLaw must be derived from NuTo::ConstitutiveBase");

    //! @brief constructor
    //! @param rLaw underlying constitutive law
    IPConstitutiveLawWithoutData(TLaw& rLaw)
        : mLaw(rLaw)
    {
    }

    std::unique_ptr<IPConstitutiveLawBase> Clone() const override
    {
        return std::make_unique<IPConstitutiveLawWithoutData<TLaw>>(*this);
    }


    ConstitutiveBase& GetConstitutiveLaw() const override
    {
        return static_cast<ConstitutiveBase&>(mLaw);
    }


    //! @brief allocates rNum additional static data
    //! @param rNum number of addtional static data
    void AllocateAdditional(int) override
    {
    }

    //! @brief Puts current static data to previous static data, previous to pre-previous, etc.
    void ShiftToPast() override
    {
    }

    //! @brief Puts previous static data to current static data, pre-previous to previous, etc.
    void ShiftToFuture() override
    {
    }

protected:
    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    void Evaluate1D(const ConstitutiveInputMap& rConstitutiveInput,
                    const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        mLaw.template Evaluate<1>(rConstitutiveInput, rConstitutiveOutput);
    }

    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    void Evaluate2D(const ConstitutiveInputMap& rConstitutiveInput,
                    const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        mLaw.template Evaluate<2>(rConstitutiveInput, rConstitutiveOutput);
    }

    //! @brief Evaluate the constitutive relation in 1D
    //! @param rConstitutiveInput Input to the constitutive law
    //! @param rConstitutiveOutput Output of the constitutive law
    void Evaluate3D(const ConstitutiveInputMap& rConstitutiveInput,
                    const ConstitutiveOutputMap& rConstitutiveOutput) override
    {
        mLaw.template Evaluate<3>(rConstitutiveInput, rConstitutiveOutput);
    }

private:
    TLaw& mLaw;
};


} // namespace Constitutive
} // namespace NuTo
