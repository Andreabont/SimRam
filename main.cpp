 
//============================================================================
// Name        : SimRam
// Author      : Andrea Bontempi
// Version     : 0.1
// Copyright   : GNU GPL3
// Description : RAM Simulator (Von Neumann Machine)
//============================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace std;
using namespace boost::program_options;

int main(int argc, char **argv) {
        options_description desc("SimRam - RAM Simulator");
        desc.add_options()
                ("help", "prints this")
		("verbose", "enable verbose mode")
                ("reg", value<int>(), "set max number of virtual registers")
		("input-file", value<string>(), "input file")
	;
	
	positional_options_description p;
	p.add("input-file", -1);

	variables_map vm;
	store(command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);
	
        notify(vm);

	if(vm.count("help"))
        {
            cout<<desc<<"\n";
            return 1;
        }

        int regnumber = 5;
	
	if(vm.count("reg"))
	{
	    regnumber = vm["reg"].as<int>();
	    if(vm.count("verbose")) cout<<"The number of virtual registers was set to "<<regnumber<<endl;
	}
	
	string filename = "input.ram";
	
	if(vm.count("input-file"))
	{
	    filename = vm["input-file"].as<string>();
	    if(vm.count("verbose")) cout<<"The source code was set to '"<<filename<<"'"<<endl;
	}
	
	ifstream source_file (filename.c_str());
	
	if (!source_file.is_open())
	{
	    cout << "<!> Unable to open file"<<endl;
	    return EXIT_FAILURE;
	}

	int registers[regnumber];
	
	memset(registers,0,sizeof(int)*regnumber);
	
	int inpointer = 0;
	
	string intape;
	string outtape;
	
	cout<<"Insert input tape: ";
	cin>>intape;
	
	string fetch;
	
	while(source_file.good())
	{
	  // Fetch
	  
	  getline(source_file, fetch);
	  
	  if(fetch == "halt")
	  {
	    cout<<"Completed execution with output: "<<outtape<<endl;
	    source_file.close();
	    return EXIT_SUCCESS;
	  }
	  
	  vector<string> line;
	  boost::algorithm::split(line, fetch, boost::algorithm::is_any_of(" "));
	  
	  // Decode / Execute
	  
	  // TODO block work on M[0]
	  // TODO remove pointer overflow on registers
	  
	  if(line[0] == "read")
	  {
	      if(vm.count("verbose")) cout<<"Read: "<<intape[inpointer]<<" --> M["<<line[1]<<"]"<<endl;
	      registers[boost::lexical_cast<int>(line[1])] = boost::lexical_cast<int>(intape[inpointer]);
	      inpointer++;
	      
	  } else if(line[0] == "write")
	  {
	      if(vm.count("verbose")) cout<<"Write: M["<<line[1]<<"] --> "<<registers[boost::lexical_cast<int>(line[1])]<<endl;
	      outtape += boost::lexical_cast<char>(registers[boost::lexical_cast<int>(line[1])]);
	    
	  } else if(line[0] == "load")
	  {
	      if(vm.count("verbose")) cout<<"M["<<line[1]<<"] --> Working Memory"<<endl;
	      registers[0] = registers[boost::lexical_cast<int>(line[1])];
	      
	  } else if(line[0] == "store")
	  {
	      if(vm.count("verbose")) cout<<"Working Memory --> M["<<line[1]<<"]"<<endl;
	      registers[boost::lexical_cast<int>(line[1])] = registers[0];
	    
	  } else if(line[0] == "debug")
	  {
	      if(!vm.count("verbose")) break;
	      switch(boost::lexical_cast<int>(line[1]))
	      {
		case 1:
		    cout<<"Registers dump: "<<endl;
		    for(int i = 0; i < regnumber; i++)
		    {
			cout<<"         M["<<i<<"] = "<<registers[i]<<endl;
		    }
		break;
		default:
		    cerr<<"<!> Invalid debug option '"<<line[1]<<"'"<<endl;
		return EXIT_FAILURE;
	      }
	    
	  } else
	  {
	      cerr<<"<!> Syntax error '"<<fetch<<"'"<<endl;
	      return EXIT_FAILURE;
	  }
	  
	}
	
	source_file.close();
	
return EXIT_SUCCESS;	
}