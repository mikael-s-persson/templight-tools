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

#include <templight/CallGraphWriters.h>

#include <boost/graph/graphviz.hpp>

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/vector_property_map.hpp>

#include <tuple>
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

static const int TemplateInstantiationVal = 0;
static const int MemoizationVal = 8;


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



CallGraphWriter::CallGraphWriter(
    std::ostream& aOS, double time_threshold, uint64_t memory_threshold) :
  TreeWriter(aOS, time_threshold, memory_threshold), g() {}

CallGraphWriter::~CallGraphWriter() { }

void CallGraphWriter::initializeTree(const std::string& aSourceName) {
  g_root = add_vertex(g);
  g[g_root].InstantiationKind = 0;
  g[g_root].Name = "CompleteTranslationUnit";
  g[g_root].CalleeFileName = aSourceName;
  g[g_root].CalleeLine = 1;
  g[g_root].CalleeColumn = 1;
  g[g_root].TimeExclCost = 0;
  g[g_root].MemoryExclCost = 0;
}

void CallGraphWriter::finalizeTree() {
  writeGraph();
}

void CallGraphWriter::openPrintedTreeNode(const EntryTraversalTask& aNode) {
  const PrintableEntryBegin& BegEntry = aNode.start;
  const PrintableEntryEnd&   EndEntry = aNode.finish;
  
  vertex_t v = boost::graph_traits<graph_t>::null_vertex();
  std::uint64_t dT_ns = 0;
  if( EndEntry.TimeStamp > BegEntry.TimeStamp )  // avoid underflow
    dT_ns = std::uint64_t((EndEntry.TimeStamp - BegEntry.TimeStamp) * 1e9);
  std::uint64_t mem_diff = 0;
  if( EndEntry.MemoryUsage > BegEntry.MemoryUsage )  // avoid underflow
    mem_diff = EndEntry.MemoryUsage - BegEntry.MemoryUsage;

  // Filter all the instantiations below the memory threshold
  if (memory_threshold_ > 0 && mem_diff < memory_threshold_) {
    return;
  }

  // Filter all the instantiations below the time threshold
  if (time_threshold_ > 0 && (dT_ns - (time_threshold_ * 1e9)) < 0) {
    return;
  }
  
  if( BegEntry.InstantiationKind == MemoizationVal ) {
    // try to find an existing instantiation:
    auto it = inst_map.find(BegEntry.Name);
    if( it != inst_map.end() ) {
      v = it->second;
    } else {
      // if we have a memoization that is unmatched, then
      //  it's not a template, or just 'noise', or whatever.
      return;
    }
  } else {
    v = add_vertex(g);
    if( BegEntry.InstantiationKind == TemplateInstantiationVal )
      inst_map[BegEntry.Name] = v;
    tree_to_graph[aNode.nd_id] = v;
    g[v].InstantiationKind = BegEntry.InstantiationKind;
    g[v].Name = BegEntry.Name;
    g[v].CalleeFileName = BegEntry.TempOri_FileName; // Template origin is the "callee"
    g[v].CalleeLine = BegEntry.TempOri_Line;
    g[v].CalleeColumn = BegEntry.TempOri_Column;
    g[v].TimeExclCost = dT_ns;
    g[v].MemoryExclCost = mem_diff;
  }
  
  vertex_t u = boost::graph_traits<graph_t>::null_vertex();
  
  if( aNode.parent_id == RecordedDFSEntryTree::invalid_id ) {
    u = g_root;
    // node is at the top-level, its costs must be added at root (root has inclusive cost):
    g[g_root].TimeExclCost   += dT_ns;
    g[g_root].MemoryExclCost += mem_diff;
  } else {
    u = tree_to_graph[aNode.parent_id];
    // node has a parent, its costs must be subtracted from its parent:
    if( dT_ns > g[u].TimeExclCost ) // avoid underflow
      g[u].TimeExclCost = 0;
    else
      g[u].TimeExclCost   -= dT_ns;
    if( mem_diff > g[u].MemoryExclCost ) // avoid underflow
      g[u].MemoryExclCost = 0;
    else
      g[u].MemoryExclCost -= mem_diff;
  }
  
  // and, an edge must be added from the parent node to the new vertex:
  edge_t e; bool e_added;
  
  // ---- Avoid parallel edges in the meta-call-graph ----
  // first, check if edge already exists:
  std::tie(e, e_added) = edge(u, v, g);
  if( e_added ) {
    return;
  }
  // ---- End ----
  
  std::tie(e, e_added) = add_edge(u, v, g);
  
  // FIXME (?) Point of instantiation of template should be in the same file as parent template.
  //assert(BegEntry.FileName == g[u].CalleeFileName);
  if( e_added ) {
    g[e].CallerFileName = BegEntry.FileName; // Point of instantiation is the "caller"
    g[e].CallerLine = BegEntry.Line;
    g[e].CallerColumn = BegEntry.Column;
    g[e].TimeInclCost   = dT_ns;
    g[e].MemoryInclCost = mem_diff;
  }
}

void CallGraphWriter::closePrintedTreeNode(const EntryTraversalTask& aNode) {}



namespace {
  
  struct GraphMLCGDFSVis {
    typedef CallGraphWriter::graph_t Graph;
    typedef CallGraphWriter::vertex_t Vertex;
    typedef CallGraphWriter::edge_t Edge;
    
    std::ostream* p_out;
    
    GraphMLCGDFSVis(std::ostream& aOS) : p_out(&aOS) { }
    
    void initialize_vertex(Vertex u, const Graph& g) {}
    void start_vertex(Vertex u, const Graph& g) {}
    void discover_vertex(Vertex u, const Graph& g) {
      
      (*p_out) << "<node id=\"n" << u << "\">\n";
      
      std::string EscapedName = escapeXml(g[u].Name);
      (*p_out) << 
        "  <data key=\"d0\">" << InstantiationKindStrings[g[u].InstantiationKind] << "</data>\n"
        "  <data key=\"d1\">\"" << EscapedName <<"\"</data>\n"
        "  <data key=\"d2\">\"" << g[u].CalleeFileName << "|" 
                                << g[u].CalleeLine << "|" 
                                << g[u].CalleeColumn << "\"</data>\n";
      (*p_out) << 
        "  <data key=\"d3\">" << std::fixed << std::setprecision(9) << (1e-9 * double(g[u].TimeExclCost)) << "</data>\n"
        "  <data key=\"d4\">" << g[u].MemoryExclCost << "</data>\n";
      
      (*p_out) << "</node>\n";
      
    }
    void examine_edge(Edge u, const Graph& g) {}
    void tree_edge(Edge u, const Graph& g) {}
    void back_edge(Edge u, const Graph& g) {}
    void forward_or_cross_edge(Edge u, const Graph& g) {}
    void finish_edge(Edge u, const Graph& g) {}
    void finish_vertex(Vertex u, const Graph& g) {
      int i = 0;
      for(auto oe_r = out_edges(u,g); oe_r.first != oe_r.second; ++oe_r.first, ++i) {
        (*p_out) << "<edge id=\"e" << u << "_" << i
                 << "\" source=\"n" << u 
                 << "\" target=\"n" << target(*oe_r.first, g) << "\"/>\n";
        
        (*p_out) << 
          "  <data key=\"d5\">\"" << g[*oe_r.first].CallerFileName << "|" 
                                  << g[*oe_r.first].CallerLine << "|" 
                                  << g[*oe_r.first].CallerColumn << "\"</data>\n";
        (*p_out) << 
          "  <data key=\"d6\">" << std::fixed << std::setprecision(9) << (1e-9 * double(g[*oe_r.first].TimeInclCost)) << "</data>\n"
          "  <data key=\"d7\">" << g[*oe_r.first].MemoryInclCost << "</data>\n";
        
        (*p_out) << "</edge>\n";
      }
    }
  };
  
}

GraphMLCGWriter::GraphMLCGWriter(std::ostream& aOS) : 
  CallGraphWriter(aOS) {}

GraphMLCGWriter::~GraphMLCGWriter() {}

void GraphMLCGWriter::writeGraph() {
  
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
    "<key id=\"d5\" for=\"edge\" attr.name=\"FromLocation\" attr.type=\"string\"/>\n"
    "<key id=\"d6\" for=\"edge\" attr.name=\"Time\" attr.type=\"double\">\n"
      "<default>0.0</default>\n"
    "</key>\n"
    "<key id=\"d7\" for=\"edge\" attr.name=\"Memory\" attr.type=\"long\">\n"
      "<default>0</default>\n"
    "</key>\n";
  OutputOS << "<graph>\n";
  
  boost::vector_property_map< boost::default_color_type > ColorMap;
  boost::depth_first_visit(g, g_root, GraphMLCGDFSVis(OutputOS), ColorMap);
  
  OutputOS << "</graph>\n";
  OutputOS << "</graphml>\n";
}


GraphVizCGWriter::GraphVizCGWriter(
    std::ostream& aOS, double time_threshold, uint64_t memory_threshold) : 
  CallGraphWriter(aOS, time_threshold, memory_threshold) { }

GraphVizCGWriter::~GraphVizCGWriter() {}

namespace {
  
  struct GraphVizCGLabelWriter {
    CallGraphWriter::graph_t* p_g;
    GraphVizCGLabelWriter(CallGraphWriter::graph_t* pG) : p_g(pG) { };
    
    void operator()(std::ostream& out, CallGraphWriter::vertex_t v) const {
      std::string EscapedName = (*p_g)[v].Name;
      out << "Time: " << std::fixed << std::setprecision(9)
				<< (1e-9 * double((*p_g)[v].TimeExclCost)) 
        << " seconds | " << EscapedName;
    }
  };
  
}

void GraphVizCGWriter::writeGraph() {
  boost::write_graphviz(OutputOS, g, GraphVizCGLabelWriter(&g));
}


namespace {
  
  struct CallGrindWriterDFSVis {
    typedef CallGraphWriter::graph_t Graph;
    typedef CallGraphWriter::vertex_t Vertex;
    typedef CallGraphWriter::edge_t Edge;
    
    std::ostream* p_out;
    Vertex g_root;
    
    CallGrindWriterDFSVis(std::ostream& aOS, Vertex aGRoot) : p_out(&aOS), g_root(aGRoot) { }
    
    void initialize_vertex(Vertex u, const Graph& g) {}
    void start_vertex(Vertex u, const Graph& g) {}
    void discover_vertex(Vertex u, const Graph& g) {
      if( u == g_root ) {
        
        for(auto oe_r = out_edges(g_root,g); oe_r.first != oe_r.second; ++oe_r.first) {
          Vertex v = target(*oe_r.first, g);
          (*p_out) 
            << "fl=" << g[*oe_r.first].CallerFileName << "\n"
            << "fn=global\n"
            << g[*oe_r.first].CallerLine << " 0 0\n"
            << "cfi=" << g[v].CalleeFileName << "\n"
            << "cfn=" << g[v].Name << "\n"
            << "calls=1 " << g[v].CalleeLine << "\n"
            << g[*oe_r.first].CallerLine << " " << g[*oe_r.first].TimeInclCost 
            << " " << g[*oe_r.first].MemoryInclCost << "\n";
        }
        
        return;
      }
      
      (*p_out) << "\n"
        << "fl=" << g[u].CalleeFileName << "\n"
        << "fn=" << g[u].Name << "\n"
        << g[u].CalleeLine << " " << g[u].TimeExclCost << " " 
        << g[u].MemoryExclCost << "\n";
      
      // FIXME Deal with mismatches in CallerFileName and current 'fl'.
      for(auto oe_r = out_edges(u,g); oe_r.first != oe_r.second; ++oe_r.first) {
        Vertex v = target(*oe_r.first, g);
        (*p_out) 
          << "cfi=" << g[v].CalleeFileName << "\n"
          << "cfn=" << g[v].Name << "\n"
          << "calls=1 " << g[v].CalleeLine << "\n"
          << g[*oe_r.first].CallerLine << " " << g[*oe_r.first].TimeInclCost 
          << " " << g[*oe_r.first].MemoryInclCost << "\n";
      }
      
    }
    void examine_edge(Edge u, const Graph& g) {}
    void tree_edge(Edge u, const Graph& g) {}
    void back_edge(Edge u, const Graph& g) {}
    void forward_or_cross_edge(Edge u, const Graph& g) {}
    void finish_edge(Edge u, const Graph& g) {}
    void finish_vertex(Vertex u, const Graph& g) {}
  };
  
}

CallGrindWriter::CallGrindWriter(std::ostream& aOS) : 
  CallGraphWriter(aOS) { }

CallGrindWriter::~CallGrindWriter() {}

void CallGrindWriter::writeGraph() {
  
  // Write the header information.
  OutputOS 
    << "version: 1\n"
    << "positions: line\n"
    << "event: CTime : Compilation Time (ns)\n"
    << "event: CMem : Compiler Memory Usage (bytes)\n"
    << "events: CTime CMem\n"
    << "summary: " << g[g_root].TimeExclCost << " " << g[g_root].MemoryExclCost << "\n\n";
  // NOTE: root vertex "exclusive" costs are actually inclusive, and thus, the total costs.
  
  boost::vector_property_map< boost::default_color_type > ColorMap;
  boost::depth_first_visit(g, g_root, CallGrindWriterDFSVis(OutputOS, g_root), ColorMap);
  
  
}



}

