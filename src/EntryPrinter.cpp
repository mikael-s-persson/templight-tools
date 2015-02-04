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

#include <templight/EntryPrinter.h>

#include <iostream>
#include <fstream>
#include <regex>

namespace templight {


void EntryPrinter::skipEntry() {
  if ( SkippedEndingsCount ) {
    ++SkippedEndingsCount; // note: this is needed because skipped entries are begin entries for which "shouldIgnoreEntry" is never called.
    return; // Already skipping entries.
  }
  SkippedEndingsCount = 1;
}

bool EntryPrinter::shouldIgnoreEntry(const PrintableEntryBegin &Entry) {
  // Check the black-lists:
  // (1) Is currently ignoring entries?
  if ( SkippedEndingsCount ) {
    ++SkippedEndingsCount;
    return true;
  }
  // (2) Regexes:
  if ( ( CoRegex && ( std::regex_match(Entry.Name, *CoRegex) ) ) || 
       ( IdRegex && ( std::regex_match(Entry.Name, *IdRegex) ) ) ) {
    skipEntry();
    return true;
  }
  
  return false;
}

bool EntryPrinter::shouldIgnoreEntry(const PrintableEntryEnd &Entry) {
  // Check the black-lists:
  // (1) Is currently ignoring entries?
  if ( SkippedEndingsCount ) {
    --SkippedEndingsCount;
    return true;
  }
  return false;
}


void EntryPrinter::printEntry(const PrintableEntryBegin &Entry) {
  if ( shouldIgnoreEntry(Entry) )
    return;
  
  if ( p_writer )
    p_writer->printEntry(Entry);
}

void EntryPrinter::printEntry(const PrintableEntryEnd &Entry) {
  if ( shouldIgnoreEntry(Entry) )
    return;
  
  if ( p_writer )
    p_writer->printEntry(Entry);
}

void EntryPrinter::initialize(const std::string& SourceName) {
  if ( p_writer )
    p_writer->initialize(SourceName);
}

void EntryPrinter::finalize() {
  if ( p_writer )
    p_writer->finalize();
}

EntryPrinter::EntryPrinter(const std::string &Output) : SkippedEndingsCount(0), TraceOS(0) {
  if ( Output == "-" ) {
    TraceOS = &std::cout;
  } else {
    TraceOS = new std::ofstream(Output);
    if ( !TraceOS || !(*TraceOS) ) {
      std::cerr <<
        "Error: [Templight-Tools] Can not open file to write trace of template instantiations: "
        << Output << std::endl;
      if( TraceOS )
        delete TraceOS;
      TraceOS = nullptr;
      return;
    }
  }
}

EntryPrinter::~EntryPrinter() {
  p_writer.reset(); // Delete writer before the trace-OS.
  if ( TraceOS ) {
    TraceOS->flush();
    if ( TraceOS != &std::cout )
      delete TraceOS;
  }
}

bool EntryPrinter::isValid() const { return static_cast<bool>(p_writer); }

std::ostream* EntryPrinter::getTraceStream() const { return TraceOS; }

void EntryPrinter::takeWriter(EntryWriter* aPWriter) {
  p_writer.reset(aPWriter);
}

void EntryPrinter::readBlacklists(const std::string& BLFilename) {
  if ( BLFilename.empty() ) {
    CoRegex.reset();
    IdRegex.reset();
    return;
  }
  
  std::ifstream file_in(BLFilename.c_str());
  if( ! file_in ) {
    std::cerr << "Error: [Templight-Tools] Could not open the blacklist file!" << std::endl;
    CoRegex.reset();
    IdRegex.reset();
    return;
  }
  
  std::regex findCo("^context ");
  std::regex findId("^identifier ");
  
  std::string curLine, CoPattern, IdPattern;
  
  while( std::getline(file_in, curLine) ) {
    if( std::regex_match(curLine, findCo) ) {
      if(!CoPattern.empty())
        CoPattern += '|';
      CoPattern += '(';
      CoPattern.append(curLine.begin()+8, curLine.end());
      CoPattern += ')';
    } else if( std::regex_match(curLine, findId) ) {
      if(!IdPattern.empty())
        IdPattern += '|';
      IdPattern += '(';
      IdPattern.append(curLine.begin()+11, curLine.end());
      IdPattern += ')';
    }
  }
  
  CoRegex.reset(new std::regex(CoPattern));
  IdRegex.reset(new std::regex(IdPattern));
  return;
}


}

