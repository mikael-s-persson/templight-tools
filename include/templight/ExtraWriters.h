/**
 * \file ExtraWriters.h
 *
 * This library provides a number of classes for creating writers that can print 
 * templight traces into various formats (xml, text, graphml, graphviz, etc.).
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

#ifndef TEMPLIGHT_EXTRA_WRITERS_H
#define TEMPLIGHT_EXTRA_WRITERS_H

#include <templight/PrintableEntries.h>

#include <ostream>
#include <memory>
#include <string>
#include <vector>

namespace templight {

/** \brief A trace-writer for an YAML format.
 * 
 * This class will render the traces into the given output stream in 
 * the YAML format, as a flat sequence of begin and end entries.
 * \note This is the class invoked when the 'yaml' format option is used.
 */
class YamlWriter : public EntryWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  YamlWriter(std::ostream& aOS);
  ~YamlWriter();
  
  void initialize(const std::string& aSourceName = "") override;
  void finalize() override;
  
  void printEntry(const PrintableEntryBegin& aEntry) override;
  void printEntry(const PrintableEntryEnd& aEntry) override;
  
};


/** \brief A trace-writer for an XML format.
 * 
 * This class will render the traces into the given output stream in 
 * the XML format, as a flat sequence of begin and end entries.
 * \note This is the class invoked when the 'xml' format option is used.
 */
class XmlWriter : public EntryWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  XmlWriter(std::ostream& aOS);
  ~XmlWriter();
  
  void initialize(const std::string& aSourceName = "") override;
  void finalize() override;
  
  void printEntry(const PrintableEntryBegin& aEntry) override;
  void printEntry(const PrintableEntryEnd& aEntry) override;
  
};


/** \brief A trace-writer for a text format.
 * 
 * This class will render the traces into the given output stream in 
 * the text format, as a flat sequence of begin and end entries.
 * \note This is the class invoked when the 'text' format option is used.
 */
class TextWriter : public EntryWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  TextWriter(std::ostream& aOS);
  ~TextWriter();
  
  void initialize(const std::string& aSourceName = "") override;
  void finalize() override;
  
  void printEntry(const PrintableEntryBegin& aEntry) override;
  void printEntry(const PrintableEntryEnd& aEntry) override;
  
};


struct EntryTraversalTask {
  static const std::size_t invalid_id = ~std::size_t(0);
  
  PrintableEntryBegin start;
  PrintableEntryEnd finish;
  std::size_t nd_id, id_end, parent_id;
  EntryTraversalTask(const PrintableEntryBegin& aStart,
                     std::size_t aNdId, std::size_t aParentId) :
                     start(aStart), finish(), 
                     nd_id(aNdId), id_end(invalid_id), 
                     parent_id(aParentId) { };
};


struct RecordedDFSEntryTree {
  static const std::size_t invalid_id = ~std::size_t(0);
  
  std::vector<EntryTraversalTask> parent_stack;
  std::size_t cur_top;
  
  RecordedDFSEntryTree();
  
  void beginEntry(const PrintableEntryBegin& aEntry);
  
  void endEntry(const PrintableEntryEnd& aEntry);
  
};


/** \brief A base-class for writing template instantiation trees.
 * 
 * This class will arrange the traces into a template instantiation 
 * tree (stored as a depth-first traversal stack), which it will then 
 * write to the output stream with the help of the derived-class implementation.
 */
class TreeWriter : public EntryWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  TreeWriter(
      std::ostream& aOS, double time_threshold = 0, uint64_t memory_threshold = 0);
  ~TreeWriter();
  
  void initialize(const std::string& aSourceName = "") override;
  void finalize() override;
  
  void printEntry(const PrintableEntryBegin& aEntry) override;
  void printEntry(const PrintableEntryEnd& aEntry) override;
  
protected:
  
  /** \brief Opens for the printing of a tree node.
   * 
   * This function marks the beginning of the printing of a template 
   * instantiation tree node.
   * \param aNode The template instantiation tree node being opened.
   */
  virtual void openPrintedTreeNode(const EntryTraversalTask& aNode) = 0;
  
  /** \brief Closes the printing of a tree node.
   * 
   * This function marks the end of the printing of a template 
   * instantiation tree node.
   * \param aNode The template instantiation tree node being closed.
   */
  virtual void closePrintedTreeNode(const EntryTraversalTask& aNode) = 0;
  
  /** \brief Initializes the tree-writer with a given source filename.
   * 
   * Initializes the tree-writer with a given source filename. This often 
   * does the writing of the header information for the format implemented 
   * by the derived class.
   * \param aSourceName The filename of the source file that generated the tree being printed.
   */
  virtual void initializeTree(const std::string& aSourceName) = 0;
  
  /** \brief Finalizes the tree-writer.
   * 
   * Finalizes the tree-writer. This often 
   * does the writing of the closing tags or information for the format implemented 
   * by the derived class.
   */
  virtual void finalizeTree() = 0;
  
  RecordedDFSEntryTree tree;
  double time_threshold_;
  uint64_t memory_threshold_;
};



/** \brief A tree-writer for a nested XML format.
 * 
 * This class will render the template instantiation tree into the given output stream in 
 * a nested XML format, as a nested arrangement of entries.
 * \note This is the class invoked when the 'nestedxml' format option is used.
 */
class NestedXMLWriter : public TreeWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  NestedXMLWriter(std::ostream& aOS);
  ~NestedXMLWriter();
  
protected:
  void openPrintedTreeNode(const EntryTraversalTask& aNode) override;
  void closePrintedTreeNode(const EntryTraversalTask& aNode) override;
  
  void initializeTree(const std::string& aSourceName = "") override;
  void finalizeTree() override;
  
};


/** \brief A tree-writer for the GraphML format.
 * 
 * This class will render the template instantiation tree into the given output stream in 
 * the GraphML format, which is an XML-based format for graphs.
 * \note This is the class invoked when the 'graphml' format option is used.
 */
class GraphMLWriter : public TreeWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  GraphMLWriter(std::ostream& aOS);
  ~GraphMLWriter();
  
protected:
  void openPrintedTreeNode(const EntryTraversalTask& aNode) override;
  void closePrintedTreeNode(const EntryTraversalTask& aNode) override;
  
  void initializeTree(const std::string& aSourceName = "") override;
  void finalizeTree() override;
  
private:
  int last_edge_id;
};


/** \brief A tree-writer for the GraphViz format.
 * 
 * This class will render the template instantiation tree into the given output stream in 
 * the GraphViz format, which is a text format for graph visualizations with the GraphViz tools.
 * \note This is the class invoked when the 'graphviz' format option is used.
 */
class GraphVizWriter : public TreeWriter {
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  GraphVizWriter(std::ostream& aOS);
  ~GraphVizWriter();
  
protected:
  void openPrintedTreeNode(const EntryTraversalTask& aNode) override;
  void closePrintedTreeNode(const EntryTraversalTask& aNode) override;
  
  void initializeTree(const std::string& aSourceName = "") override;
  void finalizeTree() override;
  
};



}

#endif


