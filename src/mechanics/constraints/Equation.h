#pragma once
#include <vector>
#include "mechanics/constraints/Term.h"

#include <functional>

namespace NuTo
{
namespace Constraint
{
typedef std::function<double(double)> RhsFunction;

//! @brief stores a constraint equation
class Equation
{
public:
    //! @brief ctor with constant rhs, defaults to 0
    //! @param dependentNode node reference
    //! @param dependentComponent component in the dof vector of the node
    //! @param rhs value for the constant rhs
    Equation(const NodeBase& dependentNode, int dependentComponent, double rhs = 0)
        : mDependentNode(dependentNode)
        , mDependentComponent(dependentComponent)
        , mRhs([=](double) { return rhs; })
    {
    }

    //! @brief ctor with terms and constant rhs
    //! @param dependentNode node reference
    //! @param dependentComponent component in the dof vector of the node
    //! @param terms equation terms
    //! @param rhs value for the constant rhs
    Equation(const NodeBase& dependentNode, int dependentComponent, std::vector<Term> terms, double rhs = 0)
        : mDependentNode(dependentNode)
        , mDependentComponent(dependentComponent)
        , mRhs([=](double) { return rhs; })
        , mTerms(terms)
    {
    }


public:
    //! @brief ctor with constant rhs, defaults to 0
    //! @param dependentNode node reference
    //! @param dependentComponent component in the dof vector of the node
    //! @param rhs rhs function
    Equation(const NodeBase& dependentNode, int dependentComponent, RhsFunction rhs)
        : mDependentNode(dependentNode)
        , mDependentComponent(dependentComponent)
        , mRhs(rhs)
    {
    }

    //! @brief ctor with terms and constant rhs
    //! @param dependentNode node reference
    //! @param dependentComponent component in the dof vector of the node
    //! @param terms equation terms
    //! @param rhs rhs function
    Equation(const NodeBase& dependentNode, int dependentComponent, std::vector<Term> terms, RhsFunction rhs)
        : mDependentNode(dependentNode)
        , mDependentComponent(dependentComponent)
        , mRhs(rhs)
        , mTerms(terms)
    {
    }

    //    //! @brief ctor with a rhs function
    //    //! @param rhs rhs function
    //    Equation(RhsFunction rhs)
    //        : mRhs(rhs)
    //    {
    //    }

    //    //! @brief ctor with a rhs function and terms
    //    //! @param terms equation terms
    //    //! @param rhs rhs function
    //    Equation(std::vector<Term> terms, RhsFunction rhs)
    //        : mRhs(rhs)
    //        , mTerms(terms)
    //    {
    //    }

    //! @brief adds a term to the equation
    //! @param term equation term
    void AddTerm(Term term)
    {
        mTerms.push_back(term);
    }


    int GetDependentDof(Node::eDof dof) const
    {
        return mDependentNode.get().GetDof(dof, mDependentComponent);
    }

    //! @brief evaluates the rhs function at a given time
    //! @param time global time
    //! @return rhs(time)
    double GetRhs(double time) const
    {
        return mRhs(time);
    }

    //! @brief getter for mTerms
    const std::vector<Term>& GetTerms() const
    {
        return mTerms;
    }

    //! @brief nonconst getter for mTerms
    //! @remark IMO only used for ExchangeNodePtr which is very questionable...
    std::vector<Term>& GetTerms()
    {
        return mTerms;
    }

private:
    //! @brief dependent node
    //! ctor and assignment
    std::reference_wrapper<const NodeBase> mDependentNode;

    //! @brief component component in the dof vector of the node
    int mDependentComponent;

    //! @brief rhs function
    RhsFunction mRhs;

    //! @brief terms
    std::vector<Term> mTerms;
};

} /* Constaint */
} /* NuTo */
