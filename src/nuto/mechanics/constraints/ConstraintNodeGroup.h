// $Id$

#ifndef CONSTRAINTNODEGROUP_H
#define CONSTRAINTNODEGROUP_H

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#endif  // ENABLE_SERIALIZATION

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/constraints/ConstraintBase.h"

namespace NuTo
{
template <class T>
class Group;

//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... abstract class for all constraints applied to a node group
class ConstraintNodeGroup : public ConstraintBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    ConstraintNodeGroup(const Group<NodeBase>* rGroup);

    //! @brief returns the number of constraint equations
    //! @return number of constraints
    int GetNumConstraintEquations()const;

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION


protected:
    const Group<NodeBase>* mGroup;
};
}//namespace NuTo
#endif //CONSTRAINTNODEGROUP_H

