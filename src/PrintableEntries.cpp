/*
 *    Copyright 2023 Sven Mikael Persson
 *
 *    THIS SOFTWARE IS DISTRIBUTED UNDER THE TERMS OF THE GNU GENERAL PUBLIC LICENSE v3 (GPLv3).
 *
 *    This file is part of templight-tools.
 *
 *    Templight-tools is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Templight-tools is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with templight-tools (as LICENSE in the root folder).
 *    If not, see <http://www.gnu.org/licenses/>.
 */

#include <templight/PrintableEntries.h>

namespace templight {

const char* GetInstantiationKindString(int inst_kind) {
  switch (inst_kind) {
    case 0:
      return "TemplateInstantiation";
    case 1:
      return "DefaultTemplateArgumentInstantiation";
    case 2:
      return "DefaultFunctionArgumentInstantiation";
    case 3:
      return "ExplicitTemplateArgumentSubstitution";
    case 4:
      return "DeducedTemplateArgumentSubstitution";
    case 5:
      return "PriorTemplateArgumentSubstitution";
    case 6:
      return "DefaultTemplateArgumentChecking";
    case 7:
      return "ExceptionSpecEvaluation";
    case 8:
      return "ExceptionSpecInstantiation";
    case 9:
      return "RequirementInstantiation";
    case 10:
      return "NestedRequirementConstraintsCheck";
    case 11:
      return "DeclaringSpecialMember";
    case 12:
      return "DeclaringImplicitEqualityComparison";
    case 13:
      return "DefiningSynthesizedFunction";
    case 14:
      return "ConstraintsCheck";
    case 15:
      return "ConstraintSubstitution";
    case 16:
      return "ConstraintNormalization";
    case 17:
      return "RequirementParameterInstantiation";
    case 18:
      return "ParameterMappingSubstitution";
    case 19:
      return "RewritingOperatorAsSpaceship";
    case 20:
      return "InitializingStructuredBinding";
    case 21:
      return "MarkingClassDllexported";
    case 22:
      return "BuildingBuiltinDumpStructCall";
    case 23:
      return "Memoization";
    default:
      return "UnknownInstantiationKind";
  }
  return "UnknownInstantiationKind";
}

}

