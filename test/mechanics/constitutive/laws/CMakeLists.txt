add_unit_test(GradientDamageFatigueEngineeringStress
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/EngineeringStrain.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/EquivalentStrain.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOMap.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/laws/EngineeringStressHelper.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/laws/GradientDamageEngineeringStress.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveEnum.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/timeIntegration/ImplExCallback.cpp
    )

add_unit_test(LocalDamageModel
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/EngineeringStrain.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/EquivalentStrain.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOMap.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/laws/EngineeringStressHelper.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveEnum.cpp
    )

add_unit_test(HeatConduction
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/EngineeringStrain.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOMap.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveEnum.cpp
    )

add_unit_test(PhaseField
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/EngineeringStrain.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/laws/EngineeringStressHelper.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/inputoutput/ConstitutiveIOMap.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveBase.cpp
    ${CMAKE_SOURCE_DIR}/src/mechanics/constitutive/ConstitutiveEnum.cpp
    )