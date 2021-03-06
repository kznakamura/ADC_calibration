#include "FebReader.hh"
#include "MyFunction.hh"
#include <iostream>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>

using namespace std;

class Analizer{
public:
  Analizer(string input_dir, 
	   string output_filename,
	   bool is_debug = false);

private:
  string m_input_dir;
  string m_output_filename;
  bool m_is_debug;

  vector<string> m_fileheader_list = {"FEB001", "FEB017", "FEB002"};
  vector<string> m_adc_type_list = {"high", "low"};


  //==== branch for adc_integral_tree ====//
  bool m_adc_type;
  int m_event_id;
  int m_board_id;
  int m_ch;
  int m_depth;
  double m_baseline;
  int m_pulse_timing;
  double m_integral_event;
  bool m_noise_flag;

  //==== branch for adc_integral_header ====//
  int m_high_analysis_range_min = 100, m_high_analysis_range_max = 8192-100;
  int m_low_analysis_range_min = 10, m_low_analysis_range_max = 1024-10;
  int m_high_event_threshold = 400, m_low_event_threshold = 5;
  int m_high_pre_integral = 30, m_low_pre_integral = 10;  
  int m_high_integral_width = 100, m_low_integral_width = 30;
};

Analizer::Analizer(string input_dir,
		   string output_filename,
		   bool is_debug){

  m_input_dir = input_dir;
  m_output_filename = output_filename;
  m_is_debug = is_debug;
  
  TFile *ofile = new TFile(m_output_filename.c_str(), "recreate");
  TTree *adc_integral_header = new TTree("adc_integral_header", "adc_integral_header");
  adc_integral_header -> Branch("high_analysis_range_min", &m_high_analysis_range_min);
  adc_integral_header -> Branch("high_analysis_range_max", &m_high_analysis_range_max);
  adc_integral_header -> Branch("high_event_threshold", &m_high_event_threshold);
  adc_integral_header -> Branch("high_pre_integral", &m_high_pre_integral);
  adc_integral_header -> Branch("high_integral_width", &m_high_integral_width);

  adc_integral_header -> Branch("low_analysis_range_min", &m_low_analysis_range_min);
  adc_integral_header -> Branch("low_analysis_range_max", &m_low_analysis_range_max);
  adc_integral_header -> Branch("low_event_threshold", &m_low_event_threshold);
  adc_integral_header -> Branch("low_prei_integral", &m_low_pre_integral);
  adc_integral_header -> Branch("low_integral_width", &m_low_integral_width);

  adc_integral_header -> Fill();
  adc_integral_header -> Write();
  TTree *adc_integral_tree = new TTree("adc_integral_tree", "adc_integral_tree");
  adc_integral_tree -> Branch("adc_type", &m_adc_type);
  adc_integral_tree -> Branch("event_id", &m_event_id);
  adc_integral_tree -> Branch("board_id", &m_board_id);
  adc_integral_tree -> Branch("ch", &m_ch);
  adc_integral_tree -> Branch("depth", &m_depth);
  adc_integral_tree -> Branch("baseline", &m_baseline);
  adc_integral_tree -> Branch("pulse_timing", &m_pulse_timing);
  adc_integral_tree -> Branch("integral_event", &m_integral_event);
  adc_integral_tree -> Branch("noise_flag", &m_noise_flag);

  for(auto file_header: m_fileheader_list){
    for(auto adc_type : m_adc_type_list){
      for(int mpxch=0; mpxch<8; mpxch++){
	string path_to_inputdir = m_input_dir + file_header + "_";
	string input_filepath;
	if(adc_type == "high"){
	  input_filepath = m_input_dir + file_header + "_" + adc_type + "_" + to_string(mpxch) + ".atk";
	}else{
	  input_filepath = m_input_dir + file_header + "_" + adc_type + ".atk";
	}
	cout << "input_filepath: " << input_filepath << endl;
	  
	FebReader *feb = new FebReader(input_filepath, false);
	while(feb -> readEvent()){
	  m_adc_type = feb->getAdcType();
	  m_event_id = feb->getEventId();
	  m_board_id = feb->getBoardId();
	  m_depth = feb->getDepth();
	  
	  int analysis_range_min, analysis_range_max;
	  int event_threshold;
	  int pre_integral;
	  int integral_width;
	  
	  if(m_adc_type==true){
	    analysis_range_min = m_high_analysis_range_min;
	    analysis_range_max = m_high_analysis_range_max;
	    event_threshold = m_high_event_threshold;
	    pre_integral = m_high_pre_integral;
	    integral_width = m_high_integral_width;
	  }else{
	    analysis_range_min = m_low_analysis_range_min;
	    analysis_range_max = m_low_analysis_range_max;
	    event_threshold = m_low_event_threshold;
	    pre_integral = m_low_pre_integral;
	    integral_width = m_low_integral_width;
	  }
	  
	  for(int ch=0; ch<MAXCH; ch++){
	    if(feb->getChMask(ch)==false) continue;
	    m_ch = ch;
	    int *rawwave = feb->getData(ch);
	    m_baseline = averageWave(rawwave, 0, m_depth);
	    m_pulse_timing = 0;
	    for(int smp=0; smp<m_depth; smp++){
	      int pulse_height = rawwave[smp] - m_baseline;
	      if(pulse_height > event_threshold){
		m_pulse_timing = smp;
		break;
	      }
	    }

	    int integral_start = m_pulse_timing - pre_integral;
	    int integral_end = integral_start + integral_width;
	    if( integral_start<analysis_range_min || integral_end>analysis_range_max )continue;
	    // パルスがある部分を除いてベースラインを計算しなおす
	    m_baseline = averageWave(rawwave, 0, m_depth, integral_start, integral_end);

	    m_integral_event = 0;
	    m_noise_flag = false;
	    for(int smp=integral_start; smp<integral_end; smp++){
	      m_integral_event += rawwave[smp] - m_baseline;
	      if( (rawwave[smp]-m_baseline) < -event_threshold ) m_noise_flag = true;
	    }

	    if( m_is_debug && m_noise_flag==true ){
	      cout << "# Message: noise_flag is raised ("
		   << " adc_type: " << adc_type
		   << ", ch: " << ch
		   << " )" << endl;	      
	    }
	 
	    adc_integral_tree -> Fill();	    
	  }
	}
	delete feb;
	if(adc_type =="low") break;
      }
    }
  }
  
  adc_integral_tree -> Write();
  ofile -> Close();
  
}

int main(int argc, char* argv[]){
  if(argc!=3){
    cout << "Usage: " 
	 << argv[0]
	 << " [path/to/inputdir/] [path/to/outputdir/]"
	 << endl;
    return -1;
  }
  string input_dir = argv[1];
  input_dir += "/";
  string output_dir = argv[2];
  bool is_debug = true;

  string output_filename = output_dir + "/adc_integral.root";

  cout << "output_filename: " << output_filename << endl;

  Analizer *anal = new Analizer(input_dir,
  				output_filename,
				is_debug);
  
  delete anal;
    
}
