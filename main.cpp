 
//============================================================================
// Name        : SimRam
// Author      : Andrea Bontempi
// Version     : 0.1
// Copyright   : GNU GPL3
// Description : RAM Emulator (Von Neumann Machine)
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

int regnumber = 5;		// Default number of registers.

bool error_registers(int);

int main(int argc, char **argv) {
        options_description desc("SimRam - RAM Emulator");
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
	
	// TODO search LABEL.
	
	map<string, int> know_label;
	
	while(!source_file.eof())
	{
	  getline(source_file, fetch);

	  if(fetch == "halt" || fetch[0] == '#') continue;
	  
	  vector<string> line;
	  boost::algorithm::split(line, fetch, boost::algorithm::is_any_of(" "));
	  
	  if(line[0] == "label") know_label[line[1]] = source_file.tellg();
	}
	
	source_file.clear();
	source_file.seekg(0, ios::beg);
	
	while(!source_file.eof())
	{
	  // Fetch
	  
	  getline(source_file, fetch);
	  
	  if(fetch == "halt")
	  {
	    cout<<"Completed execution with output: "<<outtape<<endl;
	    source_file.close();
	    return EXIT_SUCCESS;
	  } else if(fetch == "" || fetch[0] == '#')
	  {
	    continue;
	  }
	  
	  vector<string> line;
	  boost::algorithm::split(line, fetch, boost::algorithm::is_any_of(" "));
	  
	  // Decode / Execute
	  
	  // TODO remove pointer overflow on registers
	  
	  if(line[0] == "read")
	  {
	      if(vm.count("verbose")) cout<<"Read: "<<intape[inpointer]<<" --> M["<<line[1]<<"]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(intape[inpointer] == NULL)
	      {
		  cerr<<"<!> The input tape is finished."<<endl;
		  return EXIT_FAILURE;
	      }
	      registers[boost::lexical_cast<int>(line[1])] = boost::lexical_cast<int>(intape[inpointer]);
	      inpointer++;
	      
	  } else if(line[0] == "read*")
	  {
	      if(vm.count("verbose")) cout<<"Read: "<<intape[inpointer]<<" --> M[M["<<line[1]<<"]]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      if(intape[inpointer] == NULL)
	      {
		  cerr<<"<!> The input tape is finished."<<endl;
		  return EXIT_FAILURE;
	      }
	      registers[registers[boost::lexical_cast<int>(line[1])]] = boost::lexical_cast<int>(intape[inpointer]);
	      inpointer++;
	    
	  } else if(line[0] == "write")
	  {
	      if(vm.count("verbose")) cout<<"Write: M["<<line[1]<<"] = "<<registers[boost::lexical_cast<int>(line[1])]<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      outtape += boost::lexical_cast<string>(registers[boost::lexical_cast<int>(line[1])]);
	    
	  } else if(line[0] == "write=")
	  {
	      if(vm.count("verbose")) cout<<"Write: "<<line[1]<<endl;
	      outtape += line[1];
	    
	  } else if(line[0] == "write*")
	  {
	      if(vm.count("verbose")) cout<<"Write: M[M["<<line[1]<<"]] = "<<registers[registers[boost::lexical_cast<int>(line[1])]]<<endl;
	      outtape += boost::lexical_cast<string>(registers[registers[boost::lexical_cast<int>(line[1])]]);
	    
	  } else if(line[0] == "load")
	  {
	      if(vm.count("verbose")) cout<<"M["<<line[1]<<"] --> Working Memory"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      registers[0] = registers[boost::lexical_cast<int>(line[1])];
	      
	  } else if(line[0] == "load=")
	  {
	      if(vm.count("verbose")) cout<<"Value: "<<line[1]<<" --> Working Memory"<<endl;
	      registers[0] = boost::lexical_cast<int>(line[1]);
	      
	  } else if(line[0] == "load*")
	  {
	      if(vm.count("verbose")) cout<<"M[M["<<line[1]<<"]] --> Working Memory"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      registers[0] = registers[registers[boost::lexical_cast<int>(line[1])]];
	      
	  } else if(line[0] == "store")
	  {
	      if(vm.count("verbose")) cout<<"Working Memory --> M["<<line[1]<<"]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      registers[boost::lexical_cast<int>(line[1])] = registers[0];
	    
	  } else if(line[0] == "store*")
	  {
	      if(vm.count("verbose")) cout<<"Working Memory --> M[M["<<line[1]<<"]]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      registers[registers[boost::lexical_cast<int>(line[1])]] = registers[0];
	    
	  } else if(line[0] == "add")
	  {
	      if(vm.count("verbose")) cout<<"M[0] + M["<<line[1]<<"] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      registers[0] = registers[0] + registers[boost::lexical_cast<int>(line[1])];
	    
	  } else if(line[0] == "add=")
	  {
	      if(vm.count("verbose")) cout<<"M[0] + "<<line[1]<<" --> M[0]"<<endl;
	      registers[0] = registers[0] + boost::lexical_cast<int>(line[1]);
	    
	  } else if(line[0] == "add*")
	  {
	      if(vm.count("verbose")) cout<<"M[0] + M[M["<<line[1]<<"]] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      registers[0] = registers[0] + registers[registers[boost::lexical_cast<int>(line[1])]];
	    
	  } else if(line[0] == "sub")
	  {
	      if(vm.count("verbose")) cout<<"M[0] - M["<<line[1]<<"] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      registers[0] = registers[0] - registers[boost::lexical_cast<int>(line[1])];	    
	    
	  } else if(line[0] == "sub=")
	  {
	      if(vm.count("verbose")) cout<<"M[0] - "<<line[1]<<" --> M[0]"<<endl;
	      registers[0] = registers[0] - boost::lexical_cast<int>(line[1]);	    
	    
	  } else if(line[0] == "sub*")
	  {
	      if(vm.count("verbose")) cout<<"M[0] - M[M["<<line[1]<<"]] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      registers[0] = registers[0] - registers[registers[boost::lexical_cast<int>(line[1])]];	    
	    
	  } else if(line[0] == "mul")
	  {
	      if(vm.count("verbose")) cout<<"M[0] * M["<<line[1]<<"] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      registers[0] = registers[0] * registers[boost::lexical_cast<int>(line[1])];
	    
	  } else if(line[0] == "mul=")
	  {
	      if(vm.count("verbose")) cout<<"M[0] * "<<line[1]<<" --> M[0]"<<endl;
	      registers[0] = registers[0] * boost::lexical_cast<int>(line[1]);
	    
	  } else if(line[0] == "mul*")
	  {
	      if(vm.count("verbose")) cout<<"M[0] * M[M["<<line[1]<<"]] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      registers[0] = registers[0] * registers[registers[boost::lexical_cast<int>(line[1])]];
	    
	  } else if(line[0] == "div")
	  {
	      if(vm.count("verbose")) cout<<"M[0] / M["<<line[1]<<"] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      registers[0] = registers[0] / registers[boost::lexical_cast<int>(line[1])];
	      
	  } else if(line[0] == "div=")
	  {
	      if(vm.count("verbose")) cout<<"M[0] / "<<line[1]<<" --> M[0]"<<endl;
	      registers[0] = registers[0] / boost::lexical_cast<int>(line[1]);
	      
	  } else if(line[0] == "div*")
	  {
	      if(vm.count("verbose")) cout<<"M[0] / M[M["<<line[1]<<"]] --> M[0]"<<endl;
	      if(error_registers(boost::lexical_cast<int>(line[1]))) return EXIT_FAILURE;
	      if(error_registers(registers[boost::lexical_cast<int>(line[1])])) return EXIT_FAILURE;
	      registers[0] = registers[0] / registers[registers[boost::lexical_cast<int>(line[1])]];
	      
	  } else if(line[0] == "jmp" || line[0] == "jump")
	  {
	      if(vm.count("verbose")) cout<<"Jump to LABEL '"<<line[1]<<"'"<<endl;
	      if(know_label[line[1]] == NULL)
	      {
		cerr<<"<!> Label '"<<line[1]<<"' not found."<<endl;
		return EXIT_FAILURE;
	      }
	      source_file.seekg(know_label[line[1]], ios::beg);
	    
	  } else if(line[0] == "jz")
	  {
	      if(registers[0] == 0)
	      {
		if(vm.count("verbose")) cout<<"Jump to LABEL '"<<line[1]<<"'"<<endl;
		if(know_label[line[1]] == NULL)
		{
		  cerr<<"<!> Label '"<<line[1]<<"' not found."<<endl;
		  return EXIT_FAILURE;
		}
		source_file.seekg(know_label[line[1]], ios::beg);
	      }
	      else
	      {
		if(vm.count("verbose")) cout<<"Can't jump: M[0] != 0 "<<endl;
	      }
	    
	  } else if(line[0] == "jgz")
	  {
	      if(registers[0] > 0)
	      {
		if(vm.count("verbose")) cout<<"Jump to LABEL '"<<line[1]<<"'"<<endl;
	        if(know_label[line[1]] == NULL)
	        {
		  cerr<<"<!> Label '"<<line[1]<<"' not found."<<endl;
		  return EXIT_FAILURE;
	        }
		source_file.seekg(know_label[line[1]], ios::beg);
	      }
	      else
	      {
		if(vm.count("verbose")) cout<<"Can't jump: M[0] < 0 "<<endl;
	      }
	    
	  } else if(line[0] == "label")
	  {
	      if(vm.count("verbose")) cout<<"Found LABEL '"<<line[1]<<"'"<<endl;
	      
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

bool error_registers(int n)
{
  
  if(n>(regnumber-1))
  {
    cerr<<"<!> Overflow. Register '"<<n<<"' does not exist."<<endl;
    return true;
  }
  
  return false;
  
}