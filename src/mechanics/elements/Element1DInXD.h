/*
 * Element1DInXD.h
 *
 *  Created on: 06 August 2015
 *      Author: phuschke
 */

#pragma once

#include "mechanics/elements/ContinuumElement.h"


namespace NuTo
{
template <int TDim>
struct EvaluateDataContinuum;

class Element1DInXD : public ContinuumElement<1>
{

public:
    Element1DInXD(const std::vector<NuTo::NodeBase*>& rNodes, const InterpolationType& rInterpolationType,
                  const IntegrationTypeBase& integrationType, const DofStatus& dofStatus, int globalDimension);

    Eigen::VectorXd ExtractNodeValues(int rTimeDerivative, Node::eDof) const override;
    const Eigen::VectorXd ExtractGlobalNodeValues(int rTimeDerivative, Node::eDof rDofType) const;

    Eigen::VectorXd InterpolateDofGlobal(const Eigen::VectorXd& rNaturalCoordinates,
                                         Node::eDof rDofType) const override;
    Eigen::VectorXd InterpolateDofGlobal(int rTimeDerivative, const Eigen::VectorXd& rNaturalCoordinates,
                                         Node::eDof rDofType) const override;

protected:
    //! @brief ... check if the element is properly defined (check node dofs, nodes are reordered if the element
    //! length/area/volum is negative)
    void CheckElement() override;

private:
    //! @brief constructor for serialization
    Element1DInXD() = default;

    //! @brief Calculates the rotation matrix. 2x2 in 2D, 3x3 in 3D
    //! @return mRotationMatrix
    Eigen::MatrixXd CalculateRotationMatrix();


    void CalculateElementOutputInternalGradient(BlockFullVector<double>& rInternalGradient,
                                                EvaluateDataContinuum<1>& rData, int rTheIP,
                                                const ConstitutiveInputMap& constitutiveInputMap,
                                                const ConstitutiveOutputMap& constitutiveOutputMap) const override;
    void CalculateElementOutputHessian0(BlockFullMatrix<double>& rHessian0, EvaluateDataContinuum<1>& rData, int rTheIP,
                                        const ConstitutiveOutputMap& constitutiveOutputMap) const override;

    //! @brief Calculates the transformation matrix. The size of the matrix depends on the number of nodes and the
    //! global dimension of the element.
    //! @param rGlobalDimension: global dimension of the element, i.e. 2 or 3
    //! @param rNumberOfNodes: number of nodes of the element
    //! @return  transformationMatrix
    Eigen::MatrixXd CalculateTransformationMatrix(unsigned int rGlobalDimension, unsigned int rNumberOfNodes) const;

    //! @brief Returns the number of dofs for each node depending on the dof type, i.e. scalar or vector quantity
    //! @return number of dofs per node
    int GetNumDofsPerNode(Node::eDof rDofType) const;

    Eigen::MatrixXd mRotationMatrix;

    const int mGlobalDimension;
};

} /* namespace NuTo */
