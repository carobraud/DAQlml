#include <iostream>
#include <string>
#include <netcdfcpp.h>

#include "struct_parameter_NetCDF.h"

using namespace std;

int main()
{
  ///file name
  string fi="parameters.nc";//=cimg_option("-p","parameters.nc","comment");
  ///parameters
  float z_o; string z_o_name="z_init";
  std::vector<float> z_position;string z_position_name="z_position";

  ///parameter class
  CParameterNetCDF fp;
  //open file
  int error=fp.loadFile((char *)fi.c_str());
  if(error){cerr<<"loadFile return "<< error <<endl;return error;}
  //load single variable
  error=fp.loadVar(z_o,&z_o_name);
  if(error){cerr<<"loadVar return "<< error <<endl;return error;}
  cout<<z_o_name<<"="<<z_o<<endl;  
  //load vector variable
  error=fp.loadVar(z_position,&z_position_name);
  if(error){cerr<<"loadVar return "<< error <<endl;return error;}
  cout<<z_position_name<<"=";
  for(int i=0;i<z_position.size();i++) cout<<z_position[i]<<", ";cout<<endl;


  float process; string process_name="acquisition";
  //load process variable
  error=fp.loadVar(process,&process_name);
  if(error){cerr<<"loadVar return "<< error <<endl;return error;}
cout<<process_name<<"="<<process<<endl;  
  std::vector<int> channel_index;string channel_index_name="channels";
  if (error = fp.loadAttribute(channel_index_name,channel_index)!=0){
    cerr<< "Error while loading "<<process_name<<":"<<channel_index_name<<" attribute"<<endl;
    return error;
  }
    cout<<channel_index_name<<"=";
  for(int i=0;i<channel_index.size();i++) cout<<channel_index[i]<<", ";cout<<endl;

  return 0;
}

