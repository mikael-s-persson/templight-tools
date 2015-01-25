/**
 * \file ProtobufWriter.h
 *
 * This library provides a class for writing protobuf formatted templight trace files.
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

#ifndef TEMPLIGHT_PROTOBUF_WRITER_H
#define TEMPLIGHT_PROTOBUF_WRITER_H

#include <templight/PrintableEntries.h>

#include <ostream>
#include <string>
#include <unordered_map>

namespace templight {


/** \brief A trace-writer for a Google protobuf format.
 * 
 * This class will render the traces into the given output stream in 
 * a Google protobuf format, as a flat sequence of begin and end entries.
 * 
 * The message definition for the protobuf format can be found at:
 * https://github.com/mikael-s-persson/templight/blob/master/templight_messages.proto
 * 
 * And the explanation of the protobuf format compression scheme can be found at:
 * https://github.com/mikael-s-persson/templight/wiki/Protobuf-Template-Name-Compression---Explained
 * 
 * \note This is the class invoked when the 'protobuf' format option is used.
 */
class ProtobufWriter : public EntryWriter {
private:
  
  std::string buffer;
  std::unordered_map< std::string, std::size_t > fileNameMap;
  std::unordered_map< std::string, std::size_t > templateNameMap;
  int compressionMode;
  
  std::size_t createDictionaryEntry(const std::string& Name);
  std::string printEntryLocation(const std::string& FileName, int Line, int Column);
  std::string printTemplateName(const std::string& Name);
  
public:
  
  /** \brief Creates a writer for the given output stream.
   * 
   * Creates an entry-writer for the given output stream.
   */
  ProtobufWriter(std::ostream& aOS, int aCompressLevel = 2);
  
  void initialize(const std::string& aSourceName = "") override;
  void finalize() override;
  
  void printEntry(const PrintableEntryBegin& aEntry) override;
  void printEntry(const PrintableEntryEnd& aEntry) override;
  
};


}

#endif



