#include "../include/FebReader.hh"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <bitset>
#include <algorithm>

using namespace std;

FebReader::FebReader(string filename, const bool is_debug){

  m_filename = filename;
  m_is_debug = is_debug;

  m_fin = nullptr;
  m_data = new int*[MAXCH];
  for(int ch=0; ch<MAXCH; ch++){
    m_data[ch] = new int[MAXDEPTH];
  }
  
  if(!openFile(m_filename)){
    cerr << "#Error: connot open file" 
	 << endl;
    exit(-1);
  }
}

FebReader::~FebReader(){
  delete m_fin;
  for(int ch=0; ch<MAXCH; ch++){
    delete[] m_data[ch];
  }
  delete[] m_data;
  if(m_is_debug){
    cout << "# Debug: opened binary file "
	 << "\"" << m_filename << "\" "
	 << " is deleted"
	 << endl;
  }
}

bool FebReader::openFile(string filename){
  
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

  m_fin -> seekg(0, ios_base::beg);
  if(m_is_debug){
    cout << "# Debug: current_file_pointer = 0"
	 << endl;
  }
  
  return true;
}

bool FebReader::readEvent(){
  unsigned char read_header[16] = {};
  m_fin -> read((char*)read_header, sizeof(read_header));
  if(m_fin->tellg()<0){
    if(m_is_debug){
      cout << "# Debug: " 
	   << "\"End of this file\""
	   << endl;
    }
    return false;
  }
  m_adc_type = (bool)decodeData(read_header, 1);
  m_event_id = decodeData(read_header+1, 2);
  m_board_id = decodeData(read_header+3, 1);
  m_num_of_ch = decodeData(read_header+4, 1);
  long ch_mask 
    = ((long)read_header[5]<<48) 
    + ((long)read_header[6]<<40)
    + ((long)read_header[7]<<32)
    + ((long)read_header[8]<<24)
    + ((long)read_header[9]<<16)
    + ((long)read_header[10]<<8)
    + ((long)read_header[11]);  
  
  for(int ch=0; ch<MAXCH; ch++){
    if((ch_mask>>ch)&0b1){
      m_ch_mask[ch]=true;
    }else{
       m_ch_mask[ch]=false;
    }
  }
  m_depth = decodeData(read_header+12, 2);
  
  if(m_is_debug){
    showHeader();
  }

  //---- check dummy ----//
  for(int i=0; i<2; i++){
    if(read_header[14+i] !=0){
      cerr << "# Error: "
	   << "\"HUL header format is broken\""
	   << endl;
      return false;
    }
  }
  
  //---- read sampling ----//
  for(int dp=0; dp<MAXDEPTH; dp++){
    for(int ch=0; ch<MAXCH; ch++){
      m_data[ch][dp] = 0;
    }
  }
  
  unsigned char read_data[2*MAXCH] = {};
  for(int dp=0; dp<m_depth; dp++){
    if(m_num_of_ch%2==0){
      //      cout << "konbanha. m_numof_ch: " << m_num_of_ch << endl;
      //      return false;
      m_fin -> read((char*)read_data, 1.5*m_num_of_ch);
    }else{
      m_fin -> read((char*)read_data, 1.5*(m_num_of_ch+1));
    }
    int read_ch = 0;  
    for(int ch=MAXCH-1; ch>=0; ch--){
      if(m_ch_mask[ch]==false) continue;
      if(read_ch%2==0){
	m_data[ch][dp] 
	  = (int)(
		  (read_data[read_ch*3/2]<<4)
		  + (read_data[read_ch*3/2+1]>>4)
		  );
	if(m_adc_type && m_data[ch][dp]>2047) m_data[ch][dp]-=4096;
      }else{
	m_data[ch][dp]
	  = (int)(
		  ((0x0f&read_data[read_ch+(read_ch-1)/2])<<8)
		  + read_data[read_ch+(read_ch-1)/2+1]
		  );
	if(m_adc_type && m_data[ch][dp]>2047) m_data[ch][dp]-=4096;
      }
      read_ch++;
    }

  }
  return true;
}

int FebReader::decodeData(unsigned char *pointer, int size){
  int data = 0;
  for(int i=0; i<size; i++){
    data += (int)(pointer[i]<<(8*(size-1-i)));
  }

  return data;
}

void FebReader::showHeader(){
  cout << "\n######### ADC type / event ID : " 
       << m_adc_type << " / " << m_event_id
       << " ##########" 
       << endl;  

    cout << setw(15) 
	 << "adc_type" << " | "
	 << "0x" << hex
	 << m_adc_type << m_adc_type 
	  << endl;
    cout << setw(15) 
	 << "event_id"  << " | " 
	 << dec <<m_event_id   << endl;
    cout << setw(15) 
	 << "board_id"  << " | " 
	 << m_board_id  << endl;
    cout << setw(15) 
	 << "num_of_ch"  << " | " 
	 <<  m_num_of_ch << endl;
    cout << setw(15) 
	 << "ch_mask"    << " | "; 
    for(int ch=0; ch<56; ch++){
      cout << m_ch_mask[ch];
	} 
    cout << endl;
    cout << setw(15) 
	 << "depth"      << " | " 
	 << m_depth << endl;
}

bool FebReader::showData(int show_depth){
  if(show_depth>m_depth){
    cerr << "# Error: input depth(=" << show_depth << ") "
	 << "is too large.\n"
	 <<" Should be <" << m_depth
	 << endl;
    return false;
  }
  cout << "\n";
  for(int ch=0; ch<MAXCH; ch++){
    cout << setw(5) 
	 << "ch" << ch << " | ";
    for(int dp=0; dp<show_depth; dp++){
      cout << m_data[ch][dp] << ", ";
    }
    cout << endl;
  }
  return true;
}
