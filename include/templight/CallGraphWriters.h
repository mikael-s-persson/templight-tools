/**
 * \file CallGraphWriters.h
 *
 * This library provides a number of classes for creating writers that can print 
 * templight traces in the form of meta-call-graphs into various formats (graphml-cg, 
 * graphviz-cg, callgrind, etc.).
 *
 * \author S. Mikael Persson <mikael.s.persson@gmail.com>
 * \date January 2015
 */

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

#ifndef TEMPLIGHT_CALL_GRAPH_WRITERS_H
#define TEMPLIGHT_CALL_GRAPH_WRITERS_H

#include <templight/PrintableEntries.h>
#include <templight/ExtraWriters.h>

#include <boost/graph/adjacency_list.hpp>

#include <ostream>
#include <memory>
#include <string>
#include <unordered_map>

namespace templight {


/** \brief Represents a node of the meta-call-graph.
 * 
 * This struct represents a node of the meta-call-graph which includes the 
 * name of the template instantiation, its origin (template's definition) and 
 * the time and memory cost (exclusive) of compiling it.
 */
struct MetaCGVertex {
  int InstantiationKind;        ///< The kind of instantiation that this node represents.
  std::string Name;             ///< The name of the instantiation.
  std::string CalleeFileName;   ///< The filename where the template's definition is found.
  int CalleeLine;               ///< The line where the template's definition is found.
  int CalleeColumn;             ///< The column where the template's definition is found.
  std::uint64_t TimeExclCost;   ///< The exclusive cost in compilation time.
  std::uint64_t MemoryExclCost; ///< The exclusive cost in compilation memory usage.
};

/** \brief Represents an edge of the meta-call-graph.
 * 
 * This struct represents an edge of the meta-call-graph which includes the 
 * origin of the instantiation and the time and memory cost (inclusive) of compiling it.
 */
struct MetaCGEdge {
  std::string CallerFileName;   ///< The filename where the point-of-instantiation is found.
  int CallerLine;               ///< The line where the point-of-instantiation is found.
  int CallerColumn;             ///< The column where the point-of-instantiation is found.
  std::uint64_t TimeInclCost;   ///< The inclusive cost in compilation time.
  std::uint64_t MemoryInclCost; ///< The inclusive cost in compilation memory usage.
};

/** \brief A trace-writer that constructs a meta-call-graph, as a BGL graph.
 * 
 * This class can be used as a trace-writer and has the effect of creating a meta-call-graph,
 * presumably to be written to a file by a derived class.
 * This class takes care of merging memoizations with the instantiations they refer to,
 * and to compute the exclusive and inclusive costs of the compilation.
 */
class CallGraphWriter : public TreeWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  CallGraphWriter(
    std::ostream& aOS, double time_threshold = 0, uint64_t memory_threshold = 0);
  ~CallGraphWriter();
  
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, MetaCGVertex, MetaCGEdge> graph_t;
  typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
  typedef boost::graph_traits<graph_t>::edge_descriptor edge_t;
  
protected:
  void openPrintedTreeNode(const EntryTraversalTask& aNode) override;
  void closePrintedTreeNode(const EntryTraversalTask& aNode) override;
  
  void initializeTree(const std::string& aSourceName = "") override;
  void finalizeTree() override;
  
  graph_t g;
  vertex_t g_root;
  std::unordered_map< std::string, vertex_t > inst_map;
  std::unordered_map< std::size_t, vertex_t > tree_to_graph;
  
  /**
   * This virtual function is where derived classes are given the opportunity to 
   * print the meta-call-graph to the output stream.
   * \pre The meta-call-graph has been completely constructed, with consistent cost values.
   * \post The meta-call-graph has been saved or copied out in some manner that makes 
   *       this object ready to disappear (finished its work).
   */
  virtual void writeGraph() = 0;
  
};

/** \brief A trace-writer for the meta-call-graph in the GraphML format.
 * 
 * This class will render the meta-call-graph into the given output stream in 
 * the GraphML format, which is an XML-based format for graphs.
 * \note This is the class invoked when the 'graphml-cg' format option is used.
 */
class GraphMLCGWriter : public CallGraphWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  GraphMLCGWriter(std::ostream& aOS);
  ~GraphMLCGWriter();
  
protected:
  void writeGraph() override;
};


/** \brief A trace-writer for the meta-call-graph in the GraphViz format.
 * 
 * This class will render the meta-call-graph into the given output stream in 
 * the GraphViz format, which is a text format for graph visualizations with the GraphViz tools.
 * \note This is the class invoked when the 'graphviz-cg' format option is used.
 */
class GraphVizCGWriter : public CallGraphWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  GraphVizCGWriter(std::ostream& aOS, double time_threshold, uint64_t memory_threshold);
  ~GraphVizCGWriter();
  
protected:
  void writeGraph() override;
};


/** \brief A trace-writer for the meta-call-graph in the CallGrind format.
 * 
 * This class will render the meta-call-graph into the given output stream in 
 * the CallGrind format, which is a widely used format for run-time profiling data, 
 * and it is usable by many applications, most notably KCacheGrind.
 * \note This is the class invoked when the 'callgrind' format option is used.
 */
class CallGrindWriter : public CallGraphWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  CallGrindWriter(std::ostream& aOS);
  ~CallGrindWriter();
  
protected:
  void writeGraph() override;
};


}

#endif


