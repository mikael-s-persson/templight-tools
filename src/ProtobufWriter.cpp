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

#include <templight/ProtobufWriter.h>
#include <templight/PrintableEntries.h>
#include <templight/ThinProtobuf.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <string>
#include <cstdint>

namespace templight {


ProtobufWriter::ProtobufWriter(std::ostream& aOS, int aCompressLevel) : 
  EntryWriter(aOS), compressionMode(aCompressLevel) { }

void ProtobufWriter::initialize(const std::string& aSourceName) {
  
  std::string hdr_contents;
  {
    std::ostringstream OS_inner;
    
    /*
  message TemplightHeader {
    required uint32 version = 1;
    optional string source_file = 2;
  }
    */
    
    thin_protobuf::saveVarInt(OS_inner, 1, 1); // version
    if ( !aSourceName.empty() ) 
      thin_protobuf::saveString(OS_inner, 2, aSourceName); // source_file
    
    hdr_contents = OS_inner.str();
  }
  
  std::ostringstream OS;
  // required TemplightHeader header = 1;
  thin_protobuf::saveString(OS, 1, hdr_contents);
  buffer += OS.str();
  
}

void ProtobufWriter::finalize() {
  // repeated TemplightTrace traces = 1;
  thin_protobuf::saveString(OutputOS, 1, buffer);
}

std::string ProtobufWriter::printEntryLocation(
    const std::string& FileName, int Line, int Column) {
  
  /*
  message SourceLocation {
    optional string file_name = 1;
    required uint32 file_id = 2;
    required uint32 line = 3;
    optional uint32 column = 4;
  }
  */
  
  std::ostringstream OS_inner;
  
  std::unordered_map< std::string, std::size_t >::iterator 
    it = fileNameMap.find(FileName);
  
  if ( it == fileNameMap.end() ) {
    thin_protobuf::saveString(OS_inner, 1, FileName); // file_name
    std::size_t file_id = fileNameMap.size();
    thin_protobuf::saveVarInt(OS_inner, 2, file_id);     // file_id
    fileNameMap[FileName] = file_id;
  } else {
    thin_protobuf::saveVarInt(OS_inner, 2, it->second);     // file_id
  }
  
  thin_protobuf::saveVarInt(OS_inner, 3, Line);     // line
  thin_protobuf::saveVarInt(OS_inner, 4, Column);   // column
  
  return OS_inner.str();
}

static void trimSpaces(std::string::iterator& it, std::string::iterator& it_end) {
  while ( it < it_end ) {
    if ( *it == ' ' )
      ++it;
    else if ( *it_end == ' ' )
      --it_end;
    else
      break;
  }
  ++it_end;
}

std::size_t ProtobufWriter::createDictionaryEntry(const std::string& NameOrig) {
  std::unordered_map< std::string, std::size_t >::iterator 
    it_found = templateNameMap.find(NameOrig);
  if ( it_found != templateNameMap.end() )
    return it_found->second;
  
  // FIXME: Convert this code to being constructive of "Name", instead of destructive (replacing sub-strings with '\0' characters).
  std::string Name = NameOrig;
  std::string::iterator it_open = Name.end();
  std::string::iterator it_colon_lo = Name.begin();
  int srch_state = 0;
  std::vector<std::size_t> markers;
  for(std::string::iterator it = Name.begin(); it != Name.end(); ++it) {
    switch(srch_state) {
      case 0: 
        if ( *it == '<' ) {
          // check for "operator<<", "operator<" and "operator<="
          if ( std::string(it - 8, it + 1) == "operator<" ) {
            it_open = Name.end();
            srch_state = 0;
          } else {
            it_open = it;
            ++srch_state;
          }
        } else if ( (*it == ':') && (it + 1 < Name.end()) && (*(it+1) == ':') ) {
          if ( it_colon_lo < it ) {
            markers.push_back( createDictionaryEntry(std::string(it_colon_lo, it)) );
            std::size_t offset_lo  = it_colon_lo - Name.begin();
            Name.replace(it_colon_lo, it, 1, '\0');
            it = Name.begin() + offset_lo + 2;
          } else {
            it += 1;
          }
          it_colon_lo = it + 1;
          it_open = Name.end();
        }
        break;
      case 1:
        if ( *it == '<' ) {
          // check for "operator<<" and "operator<"
          if ( std::string(it - 10, it + 1) == "operator<<<" ) {
            it_open = it;
            srch_state = 1;
          } else {
            ++srch_state;
          }
        } else if ( ( *it == ',' ) || ( *it == '>' ) ) {
          if ( it_colon_lo < it_open ) {
            std::size_t offset_end = it - it_open;
            std::size_t offset_lo  = it_colon_lo - Name.begin();
            markers.push_back( createDictionaryEntry(std::string(it_colon_lo, it_open)) );
            Name.replace(it_colon_lo, it_open, 1, '\0');
            it_open = Name.begin() + offset_lo + 1;
            it = it_open + offset_end;
            it_colon_lo = Name.end();
          }
          std::string::iterator it_lo = it_open + 1;
          std::string::iterator it_hi = it - 1;
          trimSpaces(it_lo, it_hi);
          // Create or find the marker entry:
          markers.push_back( createDictionaryEntry(std::string(it_lo, it_hi)) );
          std::size_t offset_end = it - it_hi;
          std::size_t offset_lo  = it_lo - Name.begin();
          Name.replace(it_lo, it_hi, 1, '\0');
          it = Name.begin() + offset_lo + 1 + offset_end;
          it_open = it;
          it_colon_lo = Name.end();
          if ( *it == '>' ) {
            it_open = Name.end();
            srch_state = 0;
            it_colon_lo = it + 1;
          }
        }
        break;
      default:
        if ( *it == '<' ) {
          ++srch_state;
        } else if ( *it == '>' ) {
          --srch_state;
        }
        break;
    }
  }
  if ( !markers.empty() && it_colon_lo != Name.end() ) {
    markers.push_back( createDictionaryEntry(std::string(it_colon_lo, Name.end())) );
    Name.replace(it_colon_lo, Name.end(), 1, '\0');
  }
  
  /*
  message DictionaryEntry {
    required string marked_name = 1;
    repeated uint32 marker_ids = 2;
  }
  */
  std::ostringstream OS_dict;
  thin_protobuf::saveString(OS_dict, 1, Name); // marked_name
  for(std::vector<std::size_t>::iterator mk = markers.begin(),
      mk_end = markers.end(); mk != mk_end; ++mk) {
    thin_protobuf::saveVarInt(OS_dict, 2, *mk); // marker_ids
  }
  std::string dict_entry = OS_dict.str();
  
  std::size_t id = templateNameMap.size();
  templateNameMap[NameOrig] = id;
  
  std::ostringstream OS_outer;
  // repeated DictionaryEntry names = 3;
  thin_protobuf::saveString(OS_outer, 3, dict_entry);
  buffer += OS_outer.str();
  
  return id;
}

std::string ProtobufWriter::printTemplateName(const std::string& Name) {
  
  /*
  message TemplateName {
    optional string name = 1;
    optional bytes compressed_name = 2;
  }
  */
  
  std::ostringstream OS_inner;
  
  switch( compressionMode ) {
#if 0
    case 1: { // zlib-compressed name:
      llvm::SmallVector<char, 32> CompressedBuffer;
      if ( llvm::zlib::compress(llvm::StringRef(Name), CompressedBuffer) 
                == llvm::zlib::StatusOK ) {
        // optional bytes compressed_name = 2;
        thin_protobuf::saveString(OS_inner, 2, 
          llvm::StringRef(CompressedBuffer.begin(), CompressedBuffer.size()));
        break;
      } // else, go to case 0:
    }
#endif
    case 0:
      // optional string name = 1;
      thin_protobuf::saveString(OS_inner, 1, Name); // name
      break;
    case 2:
    default:
      // optional uint32 dict_id = 3;
      thin_protobuf::saveVarInt(OS_inner, 3, 
        createDictionaryEntry(Name));
      break;
  }
  
  return OS_inner.str();
}

void ProtobufWriter::printEntry(const PrintableEntryBegin& aEntry) {
  
  std::string entry_contents;
  {
    std::ostringstream OS_inner;
    
    /*
  message Begin {
    required InstantiationKind kind = 1;
    required string name = 2;
    required SourceLocation location = 3;
    optional double time_stamp = 4;
    optional uint64 memory_usage = 5;
    optional SourceLocation template_origin = 6;
  }
    */
    
    thin_protobuf::saveVarInt(OS_inner, 1, aEntry.InstantiationKind);    // kind
    thin_protobuf::saveString(OS_inner, 2, 
                         printTemplateName(aEntry.Name));                 // name
    thin_protobuf::saveString(OS_inner, 3, 
                               printEntryLocation(aEntry.FileName, 
                                                  aEntry.Line, 
                                                  aEntry.Column)); // location
    thin_protobuf::saveDouble(OS_inner, 4, aEntry.TimeStamp);            // time_stamp
    if ( aEntry.MemoryUsage > 0 )
      thin_protobuf::saveVarInt(OS_inner, 5, aEntry.MemoryUsage);        // memory_usage
    if ( !aEntry.TempOri_FileName.empty() )
      thin_protobuf::saveString(OS_inner, 6, 
                                 printEntryLocation(aEntry.TempOri_FileName, 
                                                    aEntry.TempOri_Line, 
                                                    aEntry.TempOri_Column)); // template_origin
    
    entry_contents = OS_inner.str();
  }
  
  std::string oneof_contents;
  {
    std::ostringstream OS_inner;
    
    /*
  oneof begin_or_end {
    Begin begin = 1;
    End end = 2;
  } 
     */
    
    thin_protobuf::saveString(OS_inner, 1, entry_contents); // begin
    
    oneof_contents = OS_inner.str();
  }
  
  std::ostringstream OS;
  // repeated TemplightEntry entries = 2;
  thin_protobuf::saveString(OS, 2, oneof_contents);
  buffer += OS.str();
  
}

void ProtobufWriter::printEntry(const PrintableEntryEnd& aEntry) {
  
  std::string entry_contents;
  {
    std::ostringstream OS_inner;
    
    /*
  message End {
    optional double time_stamp = 1;
    optional uint64 memory_usage = 2;
  }
    */
    
    thin_protobuf::saveDouble(OS_inner, 1, aEntry.TimeStamp);     // time_stamp
    if ( aEntry.MemoryUsage > 0 )
      thin_protobuf::saveVarInt(OS_inner, 2, aEntry.MemoryUsage); // memory_usage
    
    entry_contents = OS_inner.str();
  }
  
  std::string oneof_contents;
  {
    std::ostringstream OS_inner;
    
    /*
  oneof begin_or_end {
    Begin begin = 1;
    End end = 2;
  } 
     */
    
    thin_protobuf::saveString(OS_inner, 2, entry_contents); // end
    
    oneof_contents = OS_inner.str();
  }
  
  std::ostringstream OS;
  // repeated TemplightEntry entries = 2;
  thin_protobuf::saveString(OS, 2, oneof_contents);
  buffer += OS.str();
  
}



} // namespace templight

