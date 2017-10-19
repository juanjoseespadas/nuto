#pragma once

#include <vector>
#include <map>
#include <eigen3/Eigen/Dense>
#include "base/Exception.h"
#include "mechanics/elements/IPData.h"
#include <memory>


namespace NuTo
{

class ConstitutiveBase;
class IntegrationTypeBase;
class InterpolationType;
class NodeBase;
class Section;
class ElementOutputBase;
enum class eVisualizeWhat;
template <typename IOEnum>
class ConstitutiveIOMap;

namespace Visualize
{
class UnstructuredGrid;
}
enum class eCellTypes;

namespace Constitutive
{
enum class eInput;
enum class eOutput;
}

using ConstitutiveInputMap = ConstitutiveIOMap<Constitutive::eInput>;
using ConstitutiveOutputMap = ConstitutiveIOMap<Constitutive::eOutput>;

namespace Element
{
enum class eOutput;
} // namespace Element

namespace Node
{
enum class eDof : unsigned char;
} // namespace Node

//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard abstract class for all elements
class ElementBase
{

public:
    //! @brief constructor
    //! @param interpolationType ... interpolation type
    //! @param integrationType ... integration type
    ElementBase(const InterpolationType& interpolationType, const IntegrationTypeBase& integrationType);

    ElementBase(const ElementBase&) = default;
    ElementBase(ElementBase&&) = default;

    ElementBase& operator=(const ElementBase&) = default;
    ElementBase& operator=(ElementBase&&) = default;

    virtual ~ElementBase() = default;

    //! @brief Print information about the element
    friend std::ostream& operator<<(std::ostream& out, const ElementBase& element);

    //! @brief returns the local dimension of the element
    //! this is required to check, if an element can be used in a 1d, 2D or 3D Structure
    //! @return local dimension
    virtual int GetLocalDimension() const = 0;

    //! @brief returns the number of nodes in this element
    //! @return number of nodes
    virtual int GetNumNodes() const;

    //! @brief returns the number of nodes in this element that are influenced by it
    //! @remark overridden by boundary elements
    //! @return number of nodes
    virtual int GetNumInfluenceNodes() const
    {
        return GetNumNodes();
    }

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    virtual NodeBase* GetNode(int rLocalNodeNumber) = 0;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @return pointer to the node
    virtual const NodeBase* GetNode(int rLocalNodeNumber) const = 0;

    //! @brief returns a pointer to the i-th node of the element
    //! @remark overridden by boundary elements
    //! @param local node number
    //! @return pointer to the node
    virtual const NodeBase* GetInfluenceNode(int rLocalNodeNumber) const
    {
        return GetNode(rLocalNodeNumber);
    }

    //! @brief returns the number of nodes intimei this element of a specific dof
    //! @brief rDofType dof type
    //! @return number of nodes
    virtual int GetNumNodes(Node::eDof rDofType) const;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @brief rDofType dof type
    //! @return pointer to the node
    virtual NodeBase* GetNode(int rLocalNodeNumber, Node::eDof rDofType) = 0;

    //! @brief returns a pointer to the i-th node of the element
    //! @param local node number
    //! @brief rDofType dof type
    //! @return pointer to the node
    virtual const NodeBase* GetNode(int rLocalNodeNumber, Node::eDof rDofType) const = 0;

    //! @brief sets the rLocalNodeNumber-th node of the element
    //! @param local node number
    //! @param pointer to the node
    virtual void SetNode(int rLocalNodeNumber, NodeBase* rNode) = 0;

    //! @brief resizes the node vector
    //! @param rNewNumNodes new number of nodes
    virtual void ResizeNodes(int rNewNumNodes) = 0;

    //! brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
    //! this routine is used, if e.g. the data type of a node has changed, but the restraints, elements etc. are still
    //! identical
    virtual void ExchangeNodePtr(NodeBase* rOldPtr, NodeBase* rNewPtr) = 0;

    //! @brief sets the constitutive law for an element
    //! @param rConstitutiveLaw reference to constitutive law entry
    void SetConstitutiveLaw(ConstitutiveBase& rConstitutiveLaw);

    //! @brief returns a reference to the constitutive law for an integration point
    //! @param rIP integration point number (counting from zero)
    //! @return reference to constitutive law
    const ConstitutiveBase& GetConstitutiveLaw(unsigned int rIP) const;

    //! @brief returns a reference to the constitutive law for an integration point
    //! @param rIP integration point number (counting from zero)
    //! @return reference to constitutive law
    ConstitutiveBase& GetConstitutiveLaw(unsigned int rIP);

    //! @brief returns a reference to the IPData object
    //! @return reference to the IPData object
    IPData& GetIPData();

    //! @brief returns true, if the constitutive law has been assigned
    //! @param rIP integration point number (counting from zero)
    bool HasConstitutiveLawAssigned(unsigned int rIP) const;

    //! @brief sets the section of an element
    //! @param rSection reference to section
    virtual void SetSection(std::shared_ptr<const Section> section);

    //! @brief returns a reference to the section of an element
    //! @return pointer to section
    virtual std::shared_ptr<const Section> GetSection() const;

    //! @brief sets the integration type of an element
    //! @param rIntegrationType reference to integration type
    void SetIntegrationType(const IntegrationTypeBase& rIntegrationType);

    //! @brief returns a pointer to the integration type of an element
    //! @return reference to integration type
    const IntegrationTypeBase& GetIntegrationType() const;

    //! @brief sets the interpolation type of an element
    //! @param rInterpolationType interpolation type
    void SetInterpolationType(const InterpolationType& rInterpolationType);

    //! @brief returns a pointer to the interpolation type of an element
    //! @return reference to interpolation type
    const InterpolationType& GetInterpolationType() const;

    //! @brief returns the number of integration points
    //! @return number of integration points
    int GetNumIntegrationPoints() const;

    //! @brief returns the weight of an integration point
    //! @param rIP integration point
    //! @return weight
    double GetIntegrationPointWeight(unsigned int rIP) const;

    //! @brief calculates output data for the element
    //! @param rInput ... constitutive input map for the constitutive law
    //! @param rOutput ...  coefficient matrix 0 1 or 2  (mass, damping and stiffness) and internal force (which
    //! includes inertia terms)
    virtual void Evaluate(const ConstitutiveInputMap& rInput,
                          std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rOutput) = 0;

    //! @brief Evaluate the constitutive law attached to an integration point.
    //! @param rConstitutiveInput Input map of the constitutive law.
    //! @param rConstitutiveOuput Output map of the constitutive law.
    //! @param IP The current integration point.
    template <int TDim>
    void EvaluateConstitutiveLaw(const ConstitutiveInputMap& rConstitutiveInput,
                                 ConstitutiveOutputMap& rConstitutiveOutput, unsigned int IP);


    //! @brief calculates output data for the element with a standard input (EULER_BACKWARD static data)
    //! @param rOutput ...  coefficient matrix 0 1 or 2  (mass, damping and stiffness) and internal force (which
    //! includes inertia terms)
    void Evaluate(std::map<Element::eOutput, std::shared_ptr<ElementOutputBase>>& rOutput);

    //! @brief integrates the stress over the element
    //! @param rStress integrated stress
    void GetIntegratedStress(Eigen::MatrixXd& rStress);

    //! @brief integrates the strain over the element
    //! @param rStrain integrated strain
    void GetIntegratedStrain(Eigen::MatrixXd& rStress);

    Eigen::VectorXd ExtractNodeValues(Node::eDof rDofType) const
    {
        return this->ExtractNodeValues(0, rDofType);
    }

    virtual Eigen::VectorXd ExtractNodeValues(int rTimeDerivative, Node::eDof rDofType) const;

    virtual Eigen::VectorXd InterpolateDofGlobal(const Eigen::VectorXd& rNaturalCoordinates, Node::eDof rDofType) const;

    virtual Eigen::VectorXd InterpolateDofGlobal(int rTimeDerivative, const Eigen::VectorXd& rNaturalCoordinates,
                                                 Node::eDof rDofType) const;

    Eigen::Vector3d InterpolateDof3D(const Eigen::VectorXd& rNaturalCoordinates, Node::eDof rDofType) const;

    //! @brief interpolates a vector dof to 3D, mainly for visualization
    //! @remark is overridden by the boundary element
    //! @param rTimeDerivative ... time derivative (0..2)
    //! @param rNaturalCoordinates ... coordinates of the point in natural element coordinates
    //! @param rDofType ... dof type
    Eigen::Vector3d InterpolateDof3D(int rTimeDerivative, const Eigen::VectorXd& rNaturalCoordinates,
                                     Node::eDof rDofType) const;

    //! @brief calculates the volume of an integration point (weight * detJac)
    //! @return rVolume  vector for storage of the ip volumes (area in 2D, length in 1D)
    virtual const Eigen::VectorXd GetIntegrationPointVolume() const = 0;

    //! @brief returns the coordinates of an integration point
    //! @param rIpNum integration point
    //! @return rCoordinates coordinates to be returned
    virtual const Eigen::Vector3d GetGlobalIntegrationPointCoordinates(int rIpNum) const;

    //! @brief computes the natural coordinates of an given point
    //! implemented with an exception for all elements, reimplementation required for those elements
    //! @param rGlobCoords (input) ... pointer to the array of coordinates
    //! @param rLocCoords (output) ... coordinates to be returned
    //! @return True if coordinates are within the element, False otherwise
    virtual bool GetLocalPointCoordinates(const double* rGlobCoords, double* rLocCoords) const;

    //! @brief Gets the control node of an boundary element, if it has one
    //! @return boundary control node
    virtual NodeBase* GetBoundaryControlNode() const;


    void Info() const;

    //! @brief returns the knots of the element
    //! @return reference on the matrix containing the knots
    virtual const Eigen::MatrixXd& GetKnots() const
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "Only implemented in ContinuumElementIGA.");
    }

    //! @brief returns the knotIDs of the element
    //! @return reference on the vector containing the knotIDs
    virtual const Eigen::VectorXi& GetKnotIDs() const
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "Only implemented in ContinuumElementIGA.");
    }

    virtual Eigen::VectorXd InterpolateDofGlobalSurfaceDerivative(int, const Eigen::VectorXd&, int, int) const
    {
        throw NuTo::Exception(__PRETTY_FUNCTION__, "Only implemented in ContinuumElementIGA.");
    }


    //! @brief Computes all data in visualizeComponents for the visualization. Decomposes the element into small cells
    //! for the cisualization.
    //! @param visualizer
    //! @param visualizeComponents: a list of visualization components to be visualized
    virtual void Visualize(Visualize::UnstructuredGrid& visualizer,
                           const std::vector<eVisualizeWhat>& visualizeComponents);

    //! @brief Computes all data in visualizeComponents for the visualization. Extrapolates integration point data to
    //! element nodes
    //! @param visualizer
    //! @param visualizeComponents: a list of visualization components to be visualized
    virtual void VisualizeExtrapolateToNodes(Visualize::UnstructuredGrid& visualizer,
                                             const std::vector<eVisualizeWhat>& visualizeComponents);

    //! @brief Computes all data in visualizeComponents for the visualization. Visualizes integration point data as
    //! vertiex elements
    //! @param visualizer
    //! @param visualizeComponents: a list of visualization components to be visualized
    virtual void VisualizeIntegrationPointData(Visualize::UnstructuredGrid& visualizer,
                                               const std::vector<eVisualizeWhat>& visualizeComponents);

    virtual void GetVisualizationCells(unsigned int& NumVisualizationPoints,
                                       std::vector<double>& VisualizationPointLocalCoordinates,
                                       unsigned int& NumVisualizationCells,
                                       std::vector<NuTo::eCellTypes>& VisualizationCellType,
                                       std::vector<unsigned int>& VisualizationCellsIncidence,
                                       std::vector<unsigned int>& VisualizationCellsIP) const;

    //! @brief ... check if the element is properly defined (check node dofs, nodes are reordered if the element
    //! length/area/volum is negative)
    virtual void CheckElement() = 0;

protected:
    //! @brief Outstream function for "virtual friend idiom"
    virtual void Info(std::ostream& out) const;

    //! @brief ... reorder nodes such that the sign of the length/area/volume of the element changes
    virtual void ReorderNodes();

    void AddPlaneStateToInput(ConstitutiveInputMap& input) const;
    //! @brief ... extract global dofs from nodes (mapping of local row ordering of the element matrices to the global
    //! dof ordering)
    //! @param rGlobalRowDofs ... vector of global row dofs
    // virtual void CalculateGlobalRowDofs(std::vector<int>& rGlobalRowDofs) const = 0;

    //! @brief ... extract global dofs from nodes (mapping of local column ordering of the element matrices to the
    //! global dof ordering)
    //! @param rGlobalColumnDofs ... vector of global column dofs
    // virtual void CalculateGlobalColumnDofs(std::vector<int>& rGlobalColumnDofs) const = 0;

    const InterpolationType* mInterpolationType;

    IPData mIPData;
};

std::ostream& operator<<(std::ostream& out, const ElementBase& element);

} // namespace NuTo
