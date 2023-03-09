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
      return "DeclaringSpecialMember";
    case 10:
      return "DefiningSynthesizedFunction";
    case 11:
    default:
      return "Memoization";
  }
  return "Memoization";
}

}

