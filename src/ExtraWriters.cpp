/*
 *    Copyright 2015 Sven Mikael Persson
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

#include <templight/ExtraWriters.h>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

namespace templight {


static const char* const InstantiationKindStrings[] = { 
  "TemplateInstantiation",
  "DefaultTemplateArgumentInstantiation",
  "DefaultFunctionArgumentInstantiation",
  "ExplicitTemplateArgumentSubstitution",
  "DeducedTemplateArgumentSubstitution", 
  "PriorTemplateArgumentSubstitution",
  "DefaultTemplateArgumentChecking", 
  "ExceptionSpecInstantiation",
  "Memoization" };


static std::string escapeXml(const std::string& Input) {
  std::string Result;
  Result.reserve(64);

  unsigned i, pos = 0;
  for (i = 0; i < Input.length(); ++i) {
    if (Input[i] == '<' || Input[i] == '>' || Input[i] == '"'
      || Input[i] == '\'' || Input[i] == '&') {
      Result.insert(Result.length(), Input, pos, i - pos);
      pos = i + 1;
      switch (Input[i]) {
      case '<':
        Result += "&lt;";
        break;
      case '>':
        Result += "&gt;";
        break;
      case '\'':
        Result += "&apos;";
        break;
      case '"':
        Result += "&quot;";
        break;
      case '&':
        Result += "&amp;";
        break;
      default:
        break;
      }
    }
  }
  Result.insert(Result.length(), Input, pos, i - pos);
  return Result;
}


void YamlWriter::initialize(const std::string& aSourceName) {}

void YamlWriter::finalize() {}

void YamlWriter::printEntry(const PrintableEntryBegin& aEntry) {
  /*
- IsBegin:         true
  Kind:            Memoization
  Name:            'Fibonacci<2>'
  Location:        'fibonacci.cpp|17|61'
  TimeStamp:       4.75621e+08
  MemoryUsage:     0
  TemplateOrigin:  'fibonacci.cpp|16|8'
  */
  
  OutputOS << 
    "- IsBegin:         true\n"
    "  Kind:            " << InstantiationKindStrings[aEntry.InstantiationKind] << "\n"
    "  Name:            '" << aEntry.Name << "'\n"
    "  Location:        '" << aEntry.FileName << "|" 
                           << aEntry.Line << "|" 
                           << aEntry.Column << "'\n";
  OutputOS << 
    "  TimeStamp:       " << std::fixed << std::setprecision(9) << aEntry.TimeStamp << "\n"
    "  MemoryUsage:     " << aEntry.MemoryUsage << "\n";
  if( !aEntry.TempOri_FileName.empty() ) {
    OutputOS << 
      "  TemplateOrigin:  '" << aEntry.TempOri_FileName << "|" 
                             << aEntry.TempOri_Line << "|" 
                             << aEntry.TempOri_Column << "'\n";
  }
}

void YamlWriter::printEntry(const PrintableEntryEnd& aEntry) {
  /*
- IsBegin:         false
  TimeStamp:       4.75621e+08
  MemoryUsage:     0
  */
  
  OutputOS << 
    "- IsBegin:         false\n"
    "  TimeStamp:       " << std::fixed << std::setprecision(9) << aEntry.TimeStamp << "\n"
    "  MemoryUsage:     " << aEntry.MemoryUsage << "\n";
}

YamlWriter::YamlWriter(std::ostream& aOS) : EntryWriter(aOS) {
  OutputOS << "---\n";
}

YamlWriter::~YamlWriter() {
  OutputOS << "...\n";
}


XmlWriter::XmlWriter(std::ostream& aOS) : EntryWriter(aOS) {
  OutputOS << "<?xml version=\"1.0\" standalone=\"yes\"?>\n";
}

XmlWriter::~XmlWriter() {
  
}

void XmlWriter::initialize(const std::string& aSourceName) {
  OutputOS << "<Trace>\n";
}

void XmlWriter::finalize() {
  OutputOS << "</Trace>\n";
}

void XmlWriter::printEntry(const PrintableEntryBegin& aEntry) {
  std::string EscapedName = escapeXml(aEntry.Name);
  OutputOS << 
    "<TemplateBegin>\n"
    "    <Kind>" << InstantiationKindStrings[aEntry.InstantiationKind] << "</Kind>\n"
    "    <Context context = \"" << EscapedName << "\"/>\n"
    "    <Location>" << aEntry.FileName << "|" 
                     << aEntry.Line << "|" 
                     << aEntry.Column << "</Location>\n";
  OutputOS << 
    "    <TimeStamp time = \"" << std::fixed << std::setprecision(9) << aEntry.TimeStamp << "\"/>\n"
    "    <MemoryUsage bytes = \"" << aEntry.MemoryUsage << "\"/>\n";
  if( !aEntry.TempOri_FileName.empty() ) {
    OutputOS << 
      "    <TemplateOrigin>" << aEntry.TempOri_FileName << "|" 
                             << aEntry.TempOri_Line << "|" 
                             << aEntry.TempOri_Column << "</TemplateOrigin>\n";
  }
  OutputOS << "</TemplateBegin>\n";
}

void XmlWriter::printEntry(const PrintableEntryEnd& aEntry) {
  OutputOS << 
    "<TemplateEnd>\n"
    "    <TimeStamp time = \"" << std::fixed << std::setprecision(9) << aEntry.TimeStamp << "\"/>\n"
    "    <MemoryUsage bytes = \"" << aEntry.MemoryUsage << "\"/>\n"
    "</TemplateEnd>\n";
}



TextWriter::TextWriter(std::ostream& aOS) : EntryWriter(aOS) {}

TextWriter::~TextWriter() {}

void TextWriter::initialize(const std::string& aSourceName) {
  OutputOS << "  SourceFile = " << aSourceName << "\n";
}

void TextWriter::finalize() {}

void TextWriter::printEntry(const PrintableEntryBegin& aEntry) {
  OutputOS << 
    "TemplateBegin\n"
    "  Kind = " << InstantiationKindStrings[aEntry.InstantiationKind] << "\n"
    "  Name = " << aEntry.Name << "\n"
    "  Location = " << aEntry.FileName << "|" 
                    << aEntry.Line << "|" 
                    << aEntry.Column << "\n";
  OutputOS << 
    "  TimeStamp = " << std::fixed << std::setprecision(9) << aEntry.TimeStamp << "\n"
    "  MemoryUsage = " << aEntry.MemoryUsage << "\n";
  if( !aEntry.TempOri_FileName.empty() ) {
    OutputOS << 
      "  TemplateOrigin = " << aEntry.TempOri_FileName << "|" 
                            << aEntry.TempOri_Line << "|" 
                            << aEntry.TempOri_Column << "\n";
  }
}

void TextWriter::printEntry(const PrintableEntryEnd& aEntry) {
  OutputOS << 
    "TemplateEnd\n"
    "  TimeStamp = " << std::fixed << std::setprecision(9) << aEntry.TimeStamp << "\n"
    "  MemoryUsage = " << aEntry.MemoryUsage << "\n";
}



RecordedDFSEntryTree::RecordedDFSEntryTree() : cur_top(invalid_id) {}

void RecordedDFSEntryTree::beginEntry(const PrintableEntryBegin& aEntry) {
  parent_stack.push_back(EntryTraversalTask(
    aEntry, parent_stack.size(), ( cur_top == invalid_id ? invalid_id : cur_top)));
  cur_top = parent_stack.size() - 1;
}

void RecordedDFSEntryTree::endEntry(const PrintableEntryEnd& aEntry) {
  parent_stack[cur_top].finish = aEntry;
  parent_stack[cur_top].id_end = parent_stack.size();
  if ( parent_stack[cur_top].parent_id == invalid_id ) 
    cur_top = invalid_id;
  else 
    cur_top = parent_stack[cur_top].parent_id;
}



TreeWriter::TreeWriter(std::ostream& aOS) : 
  EntryWriter(aOS), tree() { }

TreeWriter::~TreeWriter() { }

void TreeWriter::printEntry(const PrintableEntryBegin& aEntry) {
  tree.beginEntry(aEntry);
}

void TreeWriter::printEntry(const PrintableEntryEnd& aEntry) {
  tree.endEntry(aEntry);
}

void TreeWriter::initialize(const std::string& aSourceName) {
  this->initializeTree(aSourceName);
}

void TreeWriter::finalize() {
  std::vector<std::size_t> open_set;
  std::vector<EntryTraversalTask>& t = tree.parent_stack;
  
  for(std::size_t i = 0, i_end = t.size(); i != i_end; ++i ) {
    while ( !open_set.empty() && (i >= t[open_set.back()].id_end) ) {
      closePrintedTreeNode(t[open_set.back()]);
      open_set.pop_back();
    }
    openPrintedTreeNode(t[i]);
    open_set.push_back(i);
  }
  while ( !open_set.empty() ) {
    closePrintedTreeNode(t[open_set.back()]);
    open_set.pop_back();
  }
  
  this->finalizeTree();
  
}



NestedXMLWriter::NestedXMLWriter(std::ostream& aOS) : 
  TreeWriter(aOS) {
  OutputOS << "<?xml version=\"1.0\" standalone=\"yes\"?>\n";
}

NestedXMLWriter::~NestedXMLWriter() { }

void NestedXMLWriter::initializeTree(const std::string& aSourceName) {
  OutputOS << "<Trace>\n";
}

void NestedXMLWriter::finalizeTree() {
  OutputOS << "</Trace>\n";
}

void NestedXMLWriter::openPrintedTreeNode(const EntryTraversalTask& aNode) {
  const PrintableEntryBegin& BegEntry = aNode.start;
  const PrintableEntryEnd&   EndEntry = aNode.finish;
  std::string EscapedName = escapeXml(BegEntry.Name);
  
  OutputOS << 
    "<Entry Kind=\"" << InstantiationKindStrings[BegEntry.InstantiationKind] 
    << "\" Name=\"" << EscapedName << "\" ";
  OutputOS << 
    "Location=\"" << BegEntry.FileName << "|" 
                  << BegEntry.Line << "|" 
                  << BegEntry.Column << "\" ";
  if( !BegEntry.TempOri_FileName.empty() ) {
    OutputOS << 
      "TemplateOrigin=\"" << BegEntry.TempOri_FileName << "|" 
                          << BegEntry.TempOri_Line << "|" 
                          << BegEntry.TempOri_Column << "\" ";
  }
  OutputOS << 
    "Time=\"" << std::fixed << std::setprecision(9) << (EndEntry.TimeStamp - BegEntry.TimeStamp) 
    << "\" Memory=\"" << (EndEntry.MemoryUsage - BegEntry.MemoryUsage) << "\">\n";
  
  // Print only first part (heading).
}

void NestedXMLWriter::closePrintedTreeNode(const EntryTraversalTask& aNode) {
  OutputOS << "</Entry>\n";
}




GraphMLWriter::GraphMLWriter(std::ostream& aOS) : 
  TreeWriter(aOS), last_edge_id(0) {
  OutputOS <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\""
    " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
    " xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns"
    " http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">\n";
  OutputOS <<
    "<key id=\"d0\" for=\"node\" attr.name=\"Kind\" attr.type=\"string\"/>\n"
    "<key id=\"d1\" for=\"node\" attr.name=\"Name\" attr.type=\"string\"/>\n"
    "<key id=\"d2\" for=\"node\" attr.name=\"Location\" attr.type=\"string\"/>\n"
    "<key id=\"d3\" for=\"node\" attr.name=\"Time\" attr.type=\"double\">\n"
      "<default>0.0</default>\n"
    "</key>\n"
    "<key id=\"d4\" for=\"node\" attr.name=\"Memory\" attr.type=\"long\">\n"
      "<default>0</default>\n"
    "</key>\n"
    "<key id=\"d5\" for=\"node\" attr.name=\"TemplateOrigin\" attr.type=\"string\"/>\n";
}

GraphMLWriter::~GraphMLWriter() {
  OutputOS << "</graphml>\n";
}

void GraphMLWriter::initializeTree(const std::string& aSourceName) {
  OutputOS << "<graph>\n";
}

void GraphMLWriter::finalizeTree() { 
  OutputOS << "</graph>\n";
}

void GraphMLWriter::openPrintedTreeNode(const EntryTraversalTask& aNode) {
  const PrintableEntryBegin& BegEntry = aNode.start;
  const PrintableEntryEnd&   EndEntry = aNode.finish;
  
  OutputOS << "<node id=\"n" << aNode.nd_id << "\">\n";
  
  std::string EscapedName = escapeXml(BegEntry.Name);
  OutputOS << 
    "  <data key=\"d0\">" << InstantiationKindStrings[BegEntry.InstantiationKind] << "</data>\n"
    "  <data key=\"d1\">\"" << EscapedName <<"\"</data>\n"
    "  <data key=\"d2\">\"" << BegEntry.FileName << "|" 
                            << BegEntry.Line << "|" 
                            << BegEntry.Column << "\"</data>\n";
  OutputOS << 
    "  <data key=\"d3\">" << std::fixed << std::setprecision(9) << (EndEntry.TimeStamp - BegEntry.TimeStamp) << "</data>\n"
    "  <data key=\"d4\">" << (EndEntry.MemoryUsage - BegEntry.MemoryUsage) << "</data>\n";
  if( !BegEntry.TempOri_FileName.empty() ) {
    OutputOS << 
      "  <data key=\"d5\">\"" << BegEntry.TempOri_FileName << "|" 
                              << BegEntry.TempOri_Line << "|" 
                              << BegEntry.TempOri_Column << "\"</data>\n";
  }
  
  OutputOS << "</node>\n";
  if ( aNode.parent_id == RecordedDFSEntryTree::invalid_id )
    return;
  
  OutputOS << 
    "<edge id=\"e" << (last_edge_id++) 
    << "\" source=\"n" << aNode.parent_id 
    << "\" target=\"n" << aNode.nd_id << "\"/>\n";
}

void GraphMLWriter::closePrintedTreeNode(const EntryTraversalTask& aNode) {}




GraphVizWriter::GraphVizWriter(std::ostream& aOS) : 
  TreeWriter(aOS) { }

GraphVizWriter::~GraphVizWriter() {}

void GraphVizWriter::initializeTree(const std::string& aSourceName) {
  OutputOS << "digraph Trace {\n";
}

void GraphVizWriter::finalizeTree() {
  OutputOS << "}\n";
}

void GraphVizWriter::openPrintedTreeNode(const EntryTraversalTask& aNode) {
  const PrintableEntryBegin& BegEntry = aNode.start;
  const PrintableEntryEnd&   EndEntry = aNode.finish;
  
  std::string EscapedName = escapeXml(BegEntry.Name);
  OutputOS 
    << "n" << aNode.nd_id << " [label = "
    << "\"" << InstantiationKindStrings[BegEntry.InstantiationKind] << "\\n"
    << EscapedName << "\\n"
    << "At " << BegEntry.FileName << " Line " << BegEntry.Line << " Column " << BegEntry.Column << "\\n";
  if( !BegEntry.TempOri_FileName.empty() ) {
    OutputOS << 
      "From " << BegEntry.TempOri_FileName 
      << " Line " << BegEntry.TempOri_Line 
      << " Column " << BegEntry.TempOri_Column << "\\n";
  }
  OutputOS 
    << "Time: " << std::fixed << std::setprecision(9) << (EndEntry.TimeStamp - BegEntry.TimeStamp) 
    << " seconds Memory: " << (EndEntry.MemoryUsage - BegEntry.MemoryUsage) << " bytes\" ];\n";
  
  if ( aNode.parent_id == RecordedDFSEntryTree::invalid_id )
    return;
  
  OutputOS << "n" << aNode.parent_id << " -> n" << aNode.nd_id << ";\n";
}

void GraphVizWriter::closePrintedTreeNode(const EntryTraversalTask& aNode) {}


}

