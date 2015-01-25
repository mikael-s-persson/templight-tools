/**
 * \file EntryPrinter.h
 *
 * This library provides a class to drive the writing / printing of templight traces. 
 * This class implements all the filtering logic, such as blacklisted regular expressions.
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

#ifndef TEMPLIGHT_ENTRY_PRINTER_H
#define TEMPLIGHT_ENTRY_PRINTER_H

#include <templight/PrintableEntries.h>

#include <memory>
#include <string>
#include <regex>

namespace templight {

/** \brief This class drives the printing of templight trace elements.
 * 
 * This class acts as the driver or supervisor of the printing of templight 
 * trace elements, e.g., when rendering the traces to a file. It takes an 
 * EntryWriter to do the actual writing of the trace elements. Most of the 
 * responsibilities of this class are about filtering the traces against 
 * blacklist regular expressions or specific requests to ignore some elements.
 * \note This class was taken from actual templight profiling code, and so, 
 *       some of its inner-workings were tailored for that context (they 
 *       might seem like overkill in this context).
 */
class EntryPrinter {
public:
  
  void skipEntry();
  bool shouldIgnoreEntry(const PrintableEntryBegin& Entry);
  bool shouldIgnoreEntry(const PrintableEntryEnd& Entry);
  
  /** \brief Print the beginning part of a trace entry.
   * 
   * This function prints the beginning part of a trace entry.
   * \param Entry The beginning part of a trace entry.
   */
  void printEntry(const PrintableEntryBegin& Entry);
  
  /** \brief Print the ending part of a trace entry.
   * 
   * This function prints the ending part of a trace entry.
   * \param Entry The ending part of a trace entry.
   */
  void printEntry(const PrintableEntryEnd& Entry);
  
  /** \brief Initialize the printer with a source filename.
   * 
   * This function initializes the printer with a source filename. 
   * This is often the part where the printer will write the file header
   * information in the output stream.
   * \param SourceName The filename of the source file that produced this trace (translation-unit).
   */
  void initialize(const std::string& SourceName = "");
  
  /** \brief Finalize the printer.
   * 
   * This function finalizes the printer. 
   * This is often the part where the printer will write the tail end of the 
   * file-format in the output stream.
   */
  void finalize();
  
  /** \brief Create a printer for a given output file-name.
   * 
   * This creates a printer for a given output file-name, if the 
   * filename is "-", then the standard output (stdout) is used instead.
   */
  EntryPrinter(const std::string& Output);
  ~EntryPrinter();
  
  /** \brief Check if the printer is in a good state.
   * 
   * This function checks if this printer object is in a good state to 
   * render traces to its output stream.
   * \return True, if the printer is in a good state.
   */
  bool isValid() const;
  
  /** \brief Get a pointer to the output stream currently being used.
   * 
   * This function returns a pointer to the ouput stream currently being used to 
   * print the traces to a file (or other stream).
   * \return A pointer to the output stream currently being used.
   */
  std::ostream* getTraceStream() const;
  
  /** \brief Takes ownership of an entry-writer object to use for the rendering.
   * 
   * This function takes ownership of an entry-writer object to use for the actual 
   * rendering of the trace elements into the output stream.
   * \param aPWriter A pointer to an entry-writer object, ownership is taken over by this printer object.
   */
  void takeWriter(EntryWriter* aPWriter);
  
  /** \brief Reads a given blacklist file to add its regular expressions to the filtering process.
   * 
   * This function takes a given file-name, and reads that file to find regular expressions 
   * to be used to filter out any template instantiation that matches one of them.
   * \param BLFilename The filename of the blacklist file to use.
   * \note Only the last call to this function is meaningful, i.e., it overrides any existing 
   *       regex filters (i.e., it is not additive).
   */
  void readBlacklists(const std::string& BLFilename);
  
private:
  
  std::size_t SkippedEndingsCount;
  std::unique_ptr<std::regex> CoRegex;
  std::unique_ptr<std::regex> IdRegex;
  
  std::ostream* TraceOS;
  
  std::unique_ptr<EntryWriter> p_writer;
  
};

}

#endif

