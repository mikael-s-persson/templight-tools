
add_library(templight STATIC 
  "CallGraphWriters.cpp"
  "EntryPrinter.cpp"
  "ExtraWriters.cpp"
  "ProtobufReader.cpp"
  "ProtobufWriter.cpp"
)
templight_setup_static_library(templight)
target_link_libraries(templight ${Boost_LIBRARIES})

