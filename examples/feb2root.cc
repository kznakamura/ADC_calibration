#include "FebReader.hh"
#include <iostream>

#include <TFile.h>
#include <TTree.h>

using namespace std;

class Analizer{
public:
  Analizer(string input_filename, 
	   string output_filename);

private:
  string m_input_filename;
  string m_output_filename;

  //==== branch for feb_tree ====//
  bool m_adc_type;
  int m_event_id;
  int m_board_id;
  int m_ch;
  int m_depth;
  int m_rawwave[MAXDEPTH];
};

Analizer::Analizer(string input_filename,
		   string output_filename){

  m_input_filename = input_filename;
  m_output_filename = output_filename;
  
  TFile *ofile = new TFile(m_output_filename.c_str(), "recreate");
  TTree *feb_tree = new TTree("feb_tree", "feb_tree");
  feb_tree -> Branch("adc_type", &m_adc_type);
  feb_tree -> Branch("event_id", &m_event_id);
  feb_tree -> Branch("board_id", &m_board_id);
  feb_tree -> Branch("ch", &m_ch);
  feb_tree -> Branch("depth", &m_depth);
  feb_tree -> Branch("rawwave", m_rawwave, "rawwave[depth]/I");
  
  FebReader *feb = new FebReader(input_filename, false);
  while(feb -> readEvent()){
    //feb -> showData();
    m_adc_type = feb->getAdcType();
    m_event_id = feb->getEventId();
    m_board_id = feb->getBoardId();
    m_depth = feb->getDepth();
    for(int ch=0; ch<MAXCH; ch++){
      if(feb->getChMask(ch) == true){
	m_ch = ch;
	int *read_rawwave = feb->getData(ch);
	for(int dp=0; dp<m_depth; dp++){
	  m_rawwave[dp] = read_rawwave[dp];
	}
	feb_tree -> Fill();
      }
    }  
  }
  
  feb_tree -> Write();
  ofile -> Close();
  
  delete feb;
}

int main(int argc, char* argv[]){
  if(argc!=4){
    cout << "Usage: " 
	 << argv[0]
	 << " [path/to/inputdir/] [FEB***.atk] [path/to/outputdir/]"
	 << endl;
    return -1;
  }
  string input_dir = argv[1];
  string input_filename = argv[2];
  string output_dir = argv[3];

  //--- FEB***.atk --> FEB***.root ---//
  string input_filepath = input_dir + input_filename;
  string output_filename = input_filename.substr(0, input_filename.length()-4) + ".root";
  string output_filepath = output_dir + output_filename;

  cout << "input_filepath: " << input_filepath << endl;
  cout << "output_filepath: " << output_filepath << endl;

  Analizer *anal = new Analizer(input_filepath,
				output_filepath);
  
  delete anal;

}
