#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>

using namespace std;

class Analizer{
public:
  Analizer(string input_mppc_filename,
	   string input_correction_filename,
	   string output_filename,
	   bool is_debug = false);
  void inputMppcConfig();
  void outputMppcConfig();

private:
  string m_input_mppc_filename;
  string m_input_correction_filename;
  string m_output_filename;
  bool m_is_debug;

  int board_id_order[3] = {2, 1, 17};

  //==== branch for adc_calibration_tree ====//
  double m_high_fit_mean;
  double m_low_fit_mean;

  //==== ADC parameters ====//
  double m_high_gain = 165.0, m_low_gain = 5.0;
  double m_high_sampling = 40.0, m_low_sampling = 5.0;
  
  //==== mppc_config.dat ====//
  string m_header;
  vector<string> m_serial_ch;
  vector<int> m_board;
  vector<int> m_board_ch;
  vector<int> m_serial_num;
  vector<double> m_effgain;
  vector<double> m_dark_rate;
  vector<double> m_tau1;
  vector<double> m_tau2;
  vector<double> m_alpha;
  vector<double> m_beta;
};

Analizer::Analizer(string input_mppc_filename,
		   string input_correction_filename,
		   string output_filename,
		   bool is_debug){

  m_input_mppc_filename = input_mppc_filename;
  m_input_correction_filename = input_correction_filename;
  m_output_filename = output_filename;
  m_is_debug = is_debug;
  
  inputMppcConfig();
  
  TFile *ifile = new TFile(m_input_correction_filename.c_str());
  TTree *adc_calibration_tree = (TTree*)ifile->Get("adc_calibration_tree");
  int serial_ch = 0;
  
  for(auto id : board_id_order){
    int maxch = adc_calibration_tree -> GetMaximum("ch") + 1;
    for(int ch=0; ch<maxch; ch++){
      TTree *copy_adc_calibration = (TTree*)adc_calibration_tree -> CopyTree(Form("board_id==%d && ch==%d", id, ch));
      copy_adc_calibration -> SetBranchAddress("high_fit_mean", &m_high_fit_mean);
      copy_adc_calibration -> SetBranchAddress("low_fit_mean", &m_low_fit_mean);
      copy_adc_calibration -> GetEntry(0);
      m_effgain[serial_ch] = m_effgain[serial_ch] 
	*((m_high_gain*m_high_sampling)/(m_low_gain*m_low_sampling))
	*(m_low_fit_mean/m_high_fit_mean);
      serial_ch++;
    }
  }

  outputMppcConfig();

  ifile -> Close();
}

void Analizer::inputMppcConfig(){
  ifstream input_mppc_config(m_input_mppc_filename.c_str());
  getline(input_mppc_config, m_header);
  stringstream header_ss;
  header_ss << setw(10) << ""
	    << setw(10) << "board"
	    << setw(15) << "ch(in board)"
	    << setw(10) << "serial"
	    << setw(15) << "effgain"
	    << setw(10) << "darkrate"
	    << setw(10) << "tau1"
	    << setw(10) << "tau2"
	    << setw(10) << "alpha"
	    << setw(10) << "beta";
  m_header = header_ss.str();

  string serial_ch;
  int board, board_ch, serial_num;
  double effgain, dark_rate, tau1, tau2, alpha, beta;
  stringstream input_ss;
  input_ss << m_header << endl;

  while(input_mppc_config 
	>> serial_ch
	>> board
	>> board_ch
	>> serial_num
	>> effgain
	>> dark_rate
	>> tau1
	>> tau2
	>> alpha
	>> beta){
    input_ss << setw(10) << serial_ch
	     << setw(10) << board
	     << setw(15) << board_ch
	     << setw(10) << serial_num
	     << setw(15) << effgain
	     << setw(10) << dark_rate
	     << setw(10) << tau1
	     << setw(10) << tau2
	     << setw(10) << alpha
	     << setw(10) << beta
	     << endl;    

    m_serial_ch.push_back(serial_ch);
    m_board.push_back(board);
    m_board_ch.push_back(board_ch);
    m_serial_num.push_back(serial_num);
    m_effgain.push_back(effgain);
    m_dark_rate.push_back(dark_rate);
    m_tau1.push_back(tau1);
    m_tau2.push_back(tau2);
    m_alpha.push_back(alpha);
    m_beta.push_back(beta);
  }
  if(m_is_debug){
    cout << "#### input parameters ####" << endl;
    cout << input_ss.str() << endl;
  }

  input_mppc_config.close();
}

void Analizer::outputMppcConfig(){
 
  stringstream output_ss;
  output_ss << m_header << endl;
  size_t max_serial_ch = end(m_serial_ch) - begin(m_serial_ch);
  
  for(size_t ch=0; ch<max_serial_ch; ch++){
    output_ss << setw(10) << m_serial_ch[ch]
	      << setw(10) << m_board[ch]
	      << setw(15) << m_board_ch[ch]
	      << setw(10) << m_serial_num[ch]
	      << setw(15) << m_effgain[ch]
	      << setw(10) << m_dark_rate[ch]
	      << setw(10) << m_tau1[ch]
	      << setw(10) << m_tau2[ch]
	      << setw(10) << m_alpha[ch]
	      << setw(10) << m_beta[ch]
	      << endl;
  }
  
  if(m_is_debug){
    cout << "#### output parameters ####" << endl;
    cout << output_ss.str() << endl;
  }
  ofstream output_mppc_config(m_output_filename.c_str());
  output_mppc_config << output_ss.str() << endl;
  output_mppc_config.close();
}


int main(int argc, char* argv[]){
  if(argc!=4){
    cout << "Usage: " 
	 << argv[0]
	 << " [path/to/mppc_config.dat] [path/to/adc_calibration.root] [path/to/outputdir/]"
	 << endl;
    return -1;
  }
  string input_mppc_filename = argv[1];
  string input_correction_filename = argv[2];
  string output_dir = argv[3];
  bool is_debug = true;

  string output_filename = output_dir + "mppc_config_adc_correction.dat";

  Analizer *anal = new Analizer(input_mppc_filename,
				input_correction_filename,
  				output_filename,
				is_debug);
  
  delete anal;
    
}
