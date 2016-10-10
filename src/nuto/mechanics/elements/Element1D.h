/*
 * Element1D.h
 *
 *  Created on: 8 May 2015
 *      Author: ttitsche
 */

#pragma once

#include "nuto/mechanics/elements/ElementBase.h"

namespace NuTo
{


class StructureBase;
class DeformationGradient1D;
class TemperatureGradient1D;
class LocalEqStrain;
class NonlocalEqStrain;
class ConstitutiveTangentLocal1x1;
class EngineeringStress1D;
class HeatFlux1D;
template <int TNumRows, int TNumColumns> class ConstitutiveTangentLocal;
template <int TNumRows, int TNumColumns> class ConstitutiveTangentNonlocal;

class Element1D: public ElementBase
{

#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif // ENABLE_SERIALIZATION

public:
    Element1D(const NuTo::StructureBase* rStructure,  const std::vector<NuTo::NodeBase* >& rNodes,
            ElementData::eElementDataType rElementDataType,IpData::eIpDataType rIpDataType, InterpolationType* rInterpolationType);

    virtual ~Element1D() {}

    //! @brief calculates output data for the element
    //! @param eOutput ... coefficient matrix 0 1 or 2  (mass, damping and stiffness) and internal force (which includes inertia terms)
    //!                    @param updateStaticData (with DummyOutput), IPData, globalrow/column dofs etc.
    virtual Error::eError Evaluate(boost::ptr_multimap<NuTo::Element::eOutput, NuTo::ElementOutputBase>& rElementOutput) override;

    // VHIRTHAM TODO TEMP BEGIN

    //! @brief calculates output data for the element
    //! @param eOutput ... coefficient matrix 0 1 or 2  (mass, damping and stiffness) and internal force (which includes inertia terms)
    //!                    @param updateStaticData (with DummyOutput), IPData, globalrow/column dofs etc.
    Error::eError Evaluate(std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rElementOutput) override
    {
        throw NuTo::MechanicsException(std::string("[")+__PRETTY_FUNCTION__ + "] --- temp implementation missing");
    }

    // VHIRTHAM TODO TEMP END


    //! @brief returns the enum (type of the element)
    //! @return enum
    NuTo::Element::eElementType GetEnumType()const override;

    //! @brief returns the local dimension of the element
    //! this is required to check, if an element can be used in a 1d, 2D or 3D Structure
    //! @return local dimension
    int GetLocalDimension()const override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    NodeBase* GetNode(int rLocalNodeNumber) override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    const NodeBase* GetNode(int rLocalNodeNumber)const override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @brief rDofType dof type
    //! @return pointer to the node
    NodeBase* GetNode(int rLocalNodeNumber, Node::eDof rDofType) override;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @brief rDofType dof type
    //! @return pointer to the node
    const NodeBase* GetNode(int rLocalNodeNumber, Node::eDof rDofType)const override;

    //! @brief sets the rLocalNodeNumber-th node of the element
    //! @param local node number
    //! @param pointer to the node
    void SetNode(int rLocalNodeNumber, NodeBase* rNode) override;

    //! @brief resizes the node vector
    //! @param rNewNumNodes new number of nodes
    void ResizeNodes(int rNewNumNodes);

    //! brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
    //! this routine is used, if e.g. the data type of a node has changed, but the restraints, elements etc. are still identical
    void ExchangeNodePtr(NodeBase* rOldPtr, NodeBase* rNewPtr) override;

    //! @brief sets the section of an element
    //! @param rSection pointer to section
    void SetSection(const SectionBase* rSection) override;

    //! @brief returns a pointer to the section of an element
    //! @return pointer to section
    const SectionBase* GetSection()const override;

    //! @brief Allocates static data for an integration point of an element
    //! @param rConstitutiveLaw constitutive law, which is called to allocate the static data object
    ConstitutiveStaticDataBase* AllocateStaticData(const ConstitutiveBase* rConstitutiveLaw)const override;

    //! @brief calculates the volume of an integration point (weight * detJac)
    //! @return rVolume  vector for storage of the ip volumes (area in 2D, length in 1D)
    const Eigen::VectorXd GetIntegrationPointVolume() const override;

    const Eigen::MatrixXd ExtractNodeValues(int rTimeDerivative, Node::eDof) const override;

//    void ExtractNodeValues(Eigen::MatrixXd& rNodeValues, int rTimeDerivative, Node::eDof rDofType) const;

    //! @brief Calculates the the inverse of the Jacobian and its determinant
    //! @param rDerivativeShapeFunctions Derivatives of the shape functions
    //! @param rNodeCoordinates Node coordinates
    //! @return determinant of the Jacobian (return value)
    double CalculateJacobian(const Eigen::MatrixXd& rDerivativeShapeFunctions,
                             const Eigen::MatrixXd& rNodeCoordinates)const;

    //! @brief ... extract global dofs from nodes (mapping of local row ordering of the element matrices to the global dof ordering)
    virtual const Eigen::VectorXi CalculateGlobalRowDofs() const;

    //! @brief ... extract global dofs from nodes (mapping of local column ordering of the element matrices to the global dof ordering)
    virtual const Eigen::VectorXi CalculateGlobalColumnDofs() const;

    //! @brief calculates the deformation gradient in 1D
    //! @param rDerivativeShapeFunctionsLocal derivatives of the shape functions with respect to local coordinates (plane world coordinates)
    //! @param rNodalDisplacements local displacements
    //! @return rDeformationGradient (return value)
    const DeformationGradient1D CalculateDeformationGradient(const Eigen::MatrixXd& rDerivativeShapeFunctionsLocal,
                                      const Eigen::MatrixXd& rNodalDisplacements)const;

    //! @brief adds to a matrix the product B^tCB, where B contains the derivatives of the shape functions and C is the constitutive tangent
    //! eventually include also area/width of an element (that's the thermal solution)
    //! @param rDerivativeShapeFunctions derivatives of the shape functions with respect to global coordinates
    //! @param ConstitutiveTangentBase constitutive tangent matrix
    //! @param rFactor factor including determinant of Jacobian and IP weight
    //! @param rRow row, where to start to add the submatrix
    //! @param rCoefficientMatrix to be added to
    virtual void AddDetJBtCB(const Eigen::MatrixXd& rDerivativeShapeFunctionsGlobal,
                                  const ConstitutiveTangentLocal<1,1>& rConstitutiveTangent, double rFactor,
                                  int rRow, int rCol,
                                  FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic>& rCoefficientMatrix)const;

    //! @brief adds up the internal force vector
    //! @param rDerivativeShapeFunctions derivatives of the shape functions with respect to global coordinates
    //! @param rEngineeringStress stress
    //! @param factor factor including det Jacobian area and integration point weight
    //! @param rRow start row (in case of a multifield problem)
    //! @param rResult resforce vector
    virtual void AddDetJBtSigma(const Eigen::MatrixXd& rDerivativeShapeFunctionsGlobal,
                        const EngineeringStress1D& rEngineeringStress,
                        double factor,
                        int rRow,
                        FullVector<double,Eigen::Dynamic>& rResult)const;

    //! @brief add detJ B.T dSigma/dnonlocalEqStrain N
    //! @param derivativeShapeFunctions of the ip for all shape functions
    //! @param tangentStressNonlocalEqStrain derivative of the stress with respect to the nonlocal eq strain
    //! @param rShapeFunctions of the ip for all shape functions
    //! @param rFactor factor including detJ and area
    //! @param rResult result
    void AddDetJBtdSigmadNonlocalEqStrainN(const Eigen::MatrixXd& rDerivativeShapeFunctionsLocal,
    					ConstitutiveTangentLocal<1, 1>& rTangentStressNonlocalEqStrain,
						Eigen::VectorXd rShapeFunctions,
						double rFactor,
						int rRow,
						int rCol,
						FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rResult) const;

    //! @brief add detJ N_transpose dEqStrain/dEpsilon B
    //! @param rShapeFunctions of the ip for the nonlocal eq strain dofs
    //! @param rTangentLocalEqStrainStrain derivative of the local eq strains with respect to the strain
    //! @param rderivativeShapeFunctions of the ip for the displacement dofs
    //! @param rFactor factor including detJ and area
    //! @param rResult result
    void AddDetJNtdLocalEqStraindEpsilonB(Eigen::VectorXd rShapeFunctions,
    					ConstitutiveTangentLocal<1, 1>& rTangentLocalEqStrainStrain,
						const Eigen::MatrixXd& rDerivativeShapeFunctions,
						double rFactor,
						int rRow,
						int rCol,
						FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rResult) const;

    //! @brief adds up the constitutive Tangent times the Shape Functions
    //! @param rShapeFunctions the shape functions
    //! @param rConstitutiveTangent the result given by the constitutive law
    //! @param factor factor including det Jacobian area and integration point weight
    //! @param rRow start row in case of a multifield problem
    //! @param rResult result vector
    void AddDetJNtX(Eigen::VectorXd& rShapeFunctions,
                        ConstitutiveTangentLocal<1,1>& rConstitutiveTangent,
                        double rFactor,
                        int rRow,
                        FullVector<double,Eigen::Dynamic>& rResult) const;

    //! @brief adds up the constitutive Tangent times the derivative shape functions
    //! @param rDerivativeShapeFunctions the derivative shape functions
    //! @param rConstitutiveTangent the result given by the constitutive law
    //! @param factor factor including det Jacobian area and integration point weight
    //! @param rRow start row in case of a multifield problem
    //! @param rResult result vector
    void AddDetJBtX(const Eigen::MatrixXd& rDerivativeShapeFunctions,
                        ConstitutiveTangentLocal<1,1>& rConstitutiveTangent,
                        double rFactor,
                        int rRow,
                        FullVector<double,Eigen::Dynamic>& rResult) const;

    //! @brief adds to a matrix the product N1^t X N2, where N1 and N2 contains the the shape functions and X is the constitutive tangent
    //! @param rShapeFunction1 shape function 1
    //! @param rShapeFunction2 shape function 2
    //! @param ConstitutiveTangentBase constitutive tangent matrix
    //! @param rFactor factor including area, determinant of Jacobian and IP weight
    //! @param rRow row, where to start to add the submatrix
    //! @param rCol col, where to start to add the submatrix
    //! @param rResult result
    void AddDetJNtXN(const Eigen::VectorXd& rShapeFunction1,
                        const Eigen::VectorXd& rShapeFunction2,
                        const ConstitutiveTangentLocal<1,1>& rConstitutiveTangent,
                        double rFactor,
                        int rRow,
                        int rCol,
                        FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rResult)const;

    //! @brief adds to a matrix the product B^t X N, where B is the derivative shape functions, N the shape function and X the constitutive tangent
    //! @param rDerivativeShapeFunction derivative shape function
    //! @param rShapeFunction shape function
    //! @param ConstitutiveTangentBase constitutive tangent matrix
    //! @param rFactor factor including area, determinant of Jacobian and IP weight
    //! @param rRow row, where to start to add the submatrix
    //! @param rCol col, where to start to add the submatrix
    //! @param rResult result
    void AddDetJBtXN(const Eigen::MatrixXd& rDerivativeShapeFunction,
                        const Eigen::VectorXd& rShapeFunction,
                        const ConstitutiveTangentLocal<1,1>& rConstitutiveTangent,
                        double rFactor,
                        int rRow,
                        int rCol,
                        FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rResult)const;

    //! @brief adds to a matrix the product B1^t X B2, where B1 and B2 are the derivative shape functions and X is the constitutive tangent
    //! @param rDerivativeShapeFunction1 derivative shape function 1
    //! @param rDerivativeShapeFunction2 derivative shape function 2
    //! @param ConstitutiveTangentBase constitutive tangent matrix
    //! @param rFactor factor including area, determinant of Jacobian and IP weight
    //! @param rRow row, where to start to add the submatrix
    //! @param rCol col, where to start to add the submatrix
    //! @param rResult result
    void AddDetJBtXB(const Eigen::MatrixXd& rDerivativeShapeFunctions1,
                        const Eigen::MatrixXd& rDerivativeShapeFunctions2,
                        const ConstitutiveTangentLocal<1,1>& rConstitutiveTangent,
                        double rFactor,
                        int rRow,
                        int rCol,
                        FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rResult)const;

    //! @brief calculates the Kee matrix
    //! @param rShapeFunctions of the ip for all shape functions
    //! @param rDerivativeShapeFunctions of the ip for all shape functions
    //! @param nonlocal gradient radius xi
    //! @param rFactor multiplication factor (detJ area..)
    //! @param Kee return matrix with detJ * (Nt N + cBtB)
    void CalculateKee(Eigen::VectorXd rShapeFunctions,
    		const Eigen::MatrixXd& rDerivativeShapeFunctions,
			ConstitutiveTangentLocal<1, 1>& rNonlocalParameterC,
			double rFactor,
            FullMatrix<double,Eigen::Dynamic,Eigen::Dynamic>& rKee) const;

    //! @brief add Kee*nonlocalEqStrain-detJ*N.T*localEqStrain (detJ is already included in Kee)
    //! @param rShapeFunctions of the ip for all shape functions
    //! @param rLocalEqStrain local eq. strain values
    //! @param rKee stiffness matrix Kee
    //! @param rNodeNonlocalEqStrain nodal nonlocal eq strain values
    //! @param rFactor factor including detJ and area
    //! @param rResult result
    void AddDetJRnonlocalEqStrain(const Eigen::VectorXd& rShapeFunctions, 
                                LocalEqStrain& rLocslEqStrain, 
                                FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rKee, 
                                Eigen::MatrixXd& rNodeNonlocalEqStrain, 
                                double rFactor,
    		                    int startIndexNonlocalEqStrain, 
                                FullVector<double, Eigen::Dynamic>& rResult) const;

    //! @brief cast the base pointer to an Element1D, otherwise throws an exception
    const NuTo::Element1D* AsElement1D()const override
    {
        return this;
    }

    //! @brief cast the base pointer to an Element1D, otherwise throws an exception
    NuTo::Element1D* AsElement1D() override
    {
        return this;
    }

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class, this is the load routine
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void load(Archive & ar, const unsigned int version);

    //! @brief serializes the class, this is the save routine
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    //! @brief NodeBase-Pointer are not serialized to avoid cyclic dependencies, but are serialized as Pointer-Address (uintptr_t)
    //! Deserialization of the NodeBase-Pointer is done by searching and casting back the Address in the map
    //! @param mNodeMapCast   std::map containing the old and new Addresses
    virtual void SetNodePtrAfterSerialization(const std::map<std::uintptr_t, std::uintptr_t>& mNodeMapCast) override;

#endif  // ENABLE_SERIALIZATION


protected:

    //! @brief ... check if the element is properly defined (check node dofs, nodes are reordered if the element length/area/volum is negative)
    void CheckElement() override;

    std::vector<NodeBase*> mNodes;

    const SectionBase *mSection;

    //! @brief just for serialization
    Element1D() = default;
private:

};

} /* namespace NuTo */

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::Element1D)
#endif // ENABLE_SERIALIZATION


