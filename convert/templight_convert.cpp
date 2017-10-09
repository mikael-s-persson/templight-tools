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
#include <templight/ExtraWriters.h>
#include <templight/ProtobufReader.h>
#include <templight/ProtobufWriter.h>
#include <templight/CallGraphWriters.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <set>
#include <system_error>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;


int main(int argc, const char **argv) {
  
  using namespace templight;
  
  po::options_description generic_options("Generic options");
  generic_options.add_options()
    ("help,h", "produce this help message.")
  ;
  
  po::options_description io_options("I/O options");
  io_options.add_options()
    ("output,o", po::value<std::string>()->default_value("-"), "Write Templight profiling traces to <output-file>. Use '-' for output to stdout (default).")
    ("format,f", po::value<std::string>()->default_value("protobuf"), "Specify the format of Templight outputs (protobuf / yaml / xml / text / graphml / graphviz / nestedxml / graphml-cg / graphviz-cg / callgrind, default is protobuf).")
    ("blacklist,b", po::value<std::string>(), "Use regex expressions in <file> to filter out undesirable traces.")
    ("compression,c", po::value<int>()->default_value(0), "Specify the compression level of Templight outputs whenever the format allows.")
    ("input,i", po::value< std::vector<std::string> >(), "Read Templight profiling traces from <input-file>. If not specified, the traces will be read from stdin.")
    ("inst-only", "Only keep template instantiations in the output trace.")
    ("time-threshold,t", po::value<double>()->default_value(0.0), "Filter out all the template instantitation below this time (in seconds) threshold.")
    ("mem-threshold,m", po::value<uint64_t>()->default_value(0), "Filter out all the template instantitation below this memory (in bytes) threshold.")
  ;
  
  po::options_description cmdline_options;
  cmdline_options.add(generic_options).add(io_options);
  
  po::positional_options_description p;
  p.add("input",-1);
  
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);
  
  if(vm.count("help")) {
    std::cout << 
      "Templight/Convert\n"
      "  DESCRIPTION: A tool to convert the template instantiation profiles produced by the templight tool.\n"
      "  USAGE: templight-convert [options] [input-file]\n" << std::endl;
    std::cout << cmdline_options << std::endl;
    return 0;
  }
  
  std::vector<std::string> in_files;
  if ( vm.count("input") == 0 ) {
    in_files.push_back("-");
  } else {
    in_files = vm["input"].as< std::vector<std::string> >();
  }
  
  std::string OutputFilename = vm["output"].as<std::string>();
//   fs::create_directory(fs::path(OutputFilename).parent_path());
  
  
  EntryPrinter printer(vm["output"].as<std::string>());
  
  if ( !printer.getTraceStream() ) {
    std::cerr << "Error: [Templight-Convert] Failed to create templight trace file!" << std::endl;
    return 1;
  }
  
  std::string Format = vm["format"].as<std::string>();
  int Compression = vm["compression"].as<int>();
  
  if ( ( Format.empty() ) || ( Format == "protobuf" ) ) {
    printer.takeWriter(new ProtobufWriter(*printer.getTraceStream(),Compression));
  }
  else if ( Format == "xml" ) {
    printer.takeWriter(new XmlWriter(*printer.getTraceStream()));
  }
  else if ( Format == "text" ) {
    printer.takeWriter(new TextWriter(*printer.getTraceStream()));
  }
  else if ( Format == "graphml" ) {
    printer.takeWriter(new GraphMLWriter(*printer.getTraceStream()));
  }
  else if ( Format == "graphviz" ) {
    printer.takeWriter(new GraphVizWriter(*printer.getTraceStream()));
  }
  else if ( Format == "nestedxml" ) {
    printer.takeWriter(new NestedXMLWriter(*printer.getTraceStream()));
  }
  else if ( Format == "graphml-cg" ) {
    printer.takeWriter(new GraphMLCGWriter(*printer.getTraceStream()));
  }
  else if ( Format == "graphviz-cg" ) {
    double time_threshold = vm["time-threshold"].as<double>();
    uint64_t memory_threshold = vm["mem-threshold"].as<uint64_t>();
    printer.takeWriter(new GraphVizCGWriter(
          *printer.getTraceStream(), time_threshold, memory_threshold));
  }
  else if ( Format == "callgrind" ) {
    printer.takeWriter(new CallGrindWriter(*printer.getTraceStream()));
  }
  else if ( Format == "yaml" ) {
    printer.takeWriter(new YamlWriter(*printer.getTraceStream()));
  }
  else {
    std::cerr << "Error: [Templight-Convert] Unrecognized templight trace format: " << Format << std::endl;
    return 2;
  }
  
  bool was_inited = false;
  
  if( vm.count("blacklist") ) {
    printer.readBlacklists(vm["blacklist"].as<std::string>());
  }
  
//   if( vm.count("inst-only") ) {
//     printer.setInstOnly(true);
//   }
  
  for(unsigned int i = 0; i < in_files.size(); ++i) {
    std::istream* p_buf = nullptr;
    if( in_files[i] == "-" )
      p_buf = &std::cin;
    else {
      p_buf = new std::ifstream(in_files[i], std::ios_base::in | std::ios_base::binary);
      if( p_buf && !(*p_buf) ) {
        std::cerr << "Warning: [Templight-Convert] Could not open the templight trace file: " << in_files[i] << std::endl;
        delete p_buf;
        p_buf = nullptr;
        continue;
      }
    }
    ProtobufReader pbf_reader;
    pbf_reader.startOnBuffer(*p_buf);
    while ( pbf_reader.LastChunk != ProtobufReader::EndOfFile ) {
      switch ( pbf_reader.LastChunk ) {
        case ProtobufReader::EndOfFile:
          break;
        case ProtobufReader::Header:
          if ( was_inited ) 
            printer.finalize();
          printer.initialize(pbf_reader.SourceName);
          was_inited = true;
          pbf_reader.next();
          break;
        case ProtobufReader::BeginEntry:
          printer.printEntry(pbf_reader.LastBeginEntry);
          pbf_reader.next();
          break;
        case ProtobufReader::EndEntry:
          printer.printEntry(pbf_reader.LastEndEntry);
          pbf_reader.next();
          break;
        case ProtobufReader::Other:
        default:
          pbf_reader.next();
          break;
      }
    }
    if( p_buf && (p_buf != &std::cin) )
      delete p_buf;
  }
  
  if ( was_inited )
    printer.finalize();
  
  return 0;
}


