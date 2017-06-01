/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) and the Intelligence Advanced Research Projects Activity
 * (IARPA) by employees of the Federal Government in the course of their
 * official duties. Pursuant to title 17 Section 105 of the United States Code,
 * this software is not subject to copyright protection and is in the public
 * domain. NIST and IARPA assume no responsibility whatsoever for its use by
 * other parties, and makes no guarantees, expressed or implied, about its
 * quality, reliability, or any other characteristic.
 */

#include <be_framework_enumeration.h>

#include <n2nv_makeTemplates.h>

const std::map<N2N::Validation::MakeTemplates::Type, std::string>
N2N_Validation_MakeTemplates_Type_EnumToStringMap = {
    {N2N::Validation::MakeTemplates::Type::Enrollment, "Enrollment"},
    {N2N::Validation::MakeTemplates::Type::SearchLatent, "Search Latent"},
    {N2N::Validation::MakeTemplates::Type::SearchCapture, "Search Capture"}
};
BE_FRAMEWORK_ENUMERATION_DEFINITIONS(
    N2N::Validation::MakeTemplates::Type,
    N2N_Validation_MakeTemplates_Type_EnumToStringMap);

