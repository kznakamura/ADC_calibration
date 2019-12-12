#include "../include/HulReader.hh"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cmath>

using namespace std;

HulReader::HulReader(string filename, const bool is_debug){

  m_filename = filename;
  m_is_debug = is_debug;

  m_fin = nullptr;
  
  if(!openFile(m_filename)){
    cerr << "#Error: connot open file" 
	 << endl;
    exit(-1);
  }
}

HulReader::~HulReader(){
  delete m_fin;
  if(m_is_debug){
    cout << "# Debug: opened binary file "
	 << "\"" << m_filename << "\" "
	 << " is deleted"
	 << endl;
  }
}

bool HulReader::openFile(string filename){
  
  if(m_fin!=nullptr){
    cerr << "# Error: "
	 << "\"" << m_filename << "\" "
	 << "is already opened" 
	 << endl;
    return false;
  }

  m_fin = new ifstream(filename.c_str());

  if(!m_fin->good()){
    cerr << "# Error: failer to open "
	 << "\"" << m_filename << "\"" 
	 << endl;
    m_fin = nullptr;
    return false;
  }
  
  if(m_is_debug){
    cout << "# Debug: opened binary file "
	 << "\"" << filename << "\"" << endl;
  }

  m_current_trigger_id = 0;
  m_fin -> seekg(0, ios_base::beg);
  if(m_is_debug){
    cout << "# Debug: current_file_pointer = 0"
	 << endl;
  }
  
  return true;
}

bool HulReader::readEvent(){
  unsigned char read_event[48] = {};
  m_fin -> read((char*)read_event, sizeof(read_event));
  m_current_trigger_id++;

  m_trigger_type = (int)decodeData(read_event, 2);
  m_trigger_id = (int)decodeData(read_event+2, 4); 
  //負が含まれているため二段階でキャスト
  m_event_id = (int)((short)decodeData(read_event+6, 2)); 
  m_unixtime = decodeData(read_event+8, 8);
  m_timestamp = decodeData(read_event+16, 6);
  m_trigger_livetime = decodeData(read_event+22, 6);
  m_posttrigger_veto_time = decodeData(read_event+28, 6);
  m_bufferful_busy_time = decodeData(read_event+34, 6);

  if(m_is_debug){
    showData();
  }
 
  for(int i=0; i<4; i++){
    if(decodeData(read_event+40+i*2, 2) !=511){
      cerr << "# Message: "
	   << "\"HUL header format is broken\""
	   << " or " 
	   << "\"End of this file\""
	   << endl;
      return false;
    }
  }
  
  return true;
}

long HulReader::decodeData(unsigned char *pointer, int size){
  long data = 0;
  for(int i=0; i<size; i++){
    data += (long)(pointer[i]<<(8*i));
  }
  return data;
}

void HulReader::showData(){
  cout << "\n######### Trigger ID: " 
       << m_current_trigger_id 
       << " ##########" 
       << endl;  

    cout << setw(25) 
	 << "trigger_type" << " | "
	 << "0x" << hex << setw(4)
	 << setfill('0') << m_trigger_type 
	 << setfill(' ') << endl;
    cout << setw(25) 
	 << "trigger_id"   << " | " 
	 << dec <<m_trigger_id   << endl;
    cout << setw(25) 
	 << "event_id"     << " | " 
	 << m_event_id     << endl;
    cout << setw(25) 
	 << "unixtime"     << " | " 
	 <<  m_unixtime     << endl;
    cout << setw(25) 
	 << "timestamp"    << " | " 
	 << m_timestamp    << endl;
    cout << setw(25) 
	 << "trigger_livetime"      << " | " 
	 << m_trigger_livetime << endl;
    cout << setw(25) 
	 << "posttrigger_veto_time" << " | " 
	 << m_posttrigger_veto_time << endl;
    cout << setw(25) 
	 << "bufferful_busy_time"   << " | " 
	 << m_bufferful_busy_time << endl;
}
