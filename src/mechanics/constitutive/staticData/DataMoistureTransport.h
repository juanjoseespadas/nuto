#pragma once

#include <Eigen/Core>

namespace NuTo
{
class SerializeStreamIn;
class SerializeStreamOut;
namespace Constitutive
{
namespace StaticData
{
//! @brief Storing moisture transport static data.
class DataMoistureTransport
{
public:
    //! @brief Gets the current sorption coefficients.
    //! @return Vector with the current sorption coefficients.
    Eigen::VectorXd GetCurrentSorptionCoeff() const;

    //! @brief Sets the actual sorption coefficients.
    //! @param rLastSorptionCoeff Vector with the actual sorption coefficients.
    void SetCurrentSorptionCoeff(Eigen::VectorXd rCurrentSorptionCoeff);

    //! @brief Gets the relative humidity value of the last timestep.
    //! @return Relative humidity of the last timestep.
    double GetLastRelHumValue() const;

    //! @brief Sets the relative humidity value of the last timestep.
    //! @param rLastRelHumValue Relative humidity of the last timestep.
    void SetLastRelHumValue(double rLastRelHumValue);

    //! @brief Sets the sorption coefficients of the  last timestep.
    //! @param rLastSorptionCoeff Vector with the sorption coefficients of the last timestep.
    Eigen::VectorXd GetLastSorptionCoeff() const;

    //! @brief Sets the sorption coefficients of the last timestep.
    //! @param rLastSorptionCoeff Vector with the sorption coefficients of the last timestep.
    void SetLastSorptionCoeff(Eigen::VectorXd rLastSorptionCoeff);

    //! @brief Check if the data is from desorption.
    //! @return `true` if the data is from desorption, and `false` if it is from adsorption.
    bool IsDesorption() const;

    //! @brief Set the sorption behaviour of data.
    //! @param desorption `true` = desorption, `false` = adsorption.
    void SetDesorption(bool desorption);

    double GetLastJunctionPoint() const;

    void SetLastJunctionPoint(double newLastJunctionPoint);

    double GetCurrentJunctionPoint() const;

    void SetCurrentJunctionPoint(double newCurrentJunctionPoint);

    friend std::ostream& operator<<(std::ostream& os, const DataMoistureTransport& data);

    bool operator==(const DataMoistureTransport& rhs) const;

    bool operator!=(const DataMoistureTransport& rhs) const;

    //! @brief defines the serialization of this class
    //! @param rStream serialize output stream
    virtual void NuToSerializeSave(SerializeStreamOut& rStream)
    {
        SerializeDataMoistureTransport(rStream);
    }

    //! @brief defines the serialization of this class
    //! @param rStream serialize input stream
    virtual void NuToSerializeLoad(SerializeStreamIn& rStream)
    {
        SerializeDataMoistureTransport(rStream);
    }

protected:
    bool mSorptionHistoryDesorption = true;
    double mLastRelHumValue = 1.0;
    double mLastJunctionPoint = 0.0;
    double mCurrentJunctionPoint = 0.0;

    Eigen::VectorXd mCurrentSorptionCoeff = Eigen::VectorXd::Zero(3);
    Eigen::VectorXd mLastSorptionCoeff = Eigen::VectorXd::Zero(3);

private:
    //! @brief defines the serialization of this class
    //! @param rStream serialize input/output stream
    template <typename TStream>
    void SerializeDataMoistureTransport(TStream& rStream);
};

} // namespace StaticData
} // namespace Constitutive
} // namespace NuTo
