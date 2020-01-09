#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TH1.h>
#include <TF1.h>
#include <TCanvas.h>

using namespace std;

class Analizer{
public:
  Analizer(string input_filename, 
	   string output_dir,
	   bool is_debug = false);

private:
  string m_input_filename;
  string m_output_dir;
  bool m_is_debug;

  int board_id_list[3] = {1, 17, 2};

  //==== branch for adc_calibration_tree ====//
  int m_board_id;
  int m_ch;
  double m_high_fit_mean;
  double m_low_fit_mean;
  double m_high_fit_chisq;
  double m_low_fit_chisq;
  double m_high_fit_ndf;
  double m_low_fit_ndf;

  //==== branch for adc_calibration_header ====//
  int m_high_histrange_min = 0, m_high_histrange_max = 60e3;
  int m_low_histrange_min = 0, m_low_histrange_max = 500;
  string m_cut = "noise_flag==0";
};

Analizer::Analizer(string input_filename,
		   string output_dir,
		   bool is_debug){

  m_input_filename = input_filename;
  m_output_dir = output_dir;
  m_is_debug = is_debug;

  string output_filename = m_output_dir + "/adc_calibration.root";
  cout << "output_filename=" << output_filename << endl;

  TFile *ifile = new TFile(m_input_filename.c_str());
  TFile *ofile = new TFile(output_filename.c_str(), "recreate");
  TTree *adc_integral_header = (TTree*)ifile -> Get("adc_integral_header") -> Clone();
  TTree *adc_integral_tree = (TTree*)ifile -> Get("adc_integral_tree");
  adc_integral_header -> Write();

  TTree *adc_calibration_header = new TTree("adc_calibration_header", "adc_calibration_header");
  adc_calibration_header -> Branch("high_histrange_min", &m_high_histrange_min);
  adc_calibration_header -> Branch("high_histrange_max", &m_high_histrange_max);
  adc_calibration_header -> Branch("low_histrange_min", &m_low_histrange_min);
  adc_calibration_header -> Branch("low_histrange_max", &m_low_histrange_max);
  adc_calibration_header -> Branch("cut_condition", &m_cut);
  adc_calibration_header -> Fill();
  adc_calibration_header -> Write();
  TTree *adc_calibration_tree = new TTree("adc_calibration_tree", "adc_calibration_tree");
  adc_calibration_tree -> Branch("board_id", &m_board_id);
  adc_calibration_tree -> Branch("ch", &m_ch);
  adc_calibration_tree -> Branch("high_fit_mean", &m_high_fit_mean);
  adc_calibration_tree -> Branch("low_fit_mean", &m_low_fit_mean);
  adc_calibration_tree -> Branch("high_fit_chisq", &m_high_fit_chisq);
  adc_calibration_tree -> Branch("low_fit_chisq", &m_low_fit_chisq);
  adc_calibration_tree -> Branch("high_fit_ndf", &m_high_fit_ndf);
  adc_calibration_tree -> Branch("low_fit_ndf", &m_low_fit_ndf);
  
  
  for(auto id : board_id_list){
    int maxch = adc_integral_tree -> GetMaximum("ch") + 1;
    for(int ch=0; ch<maxch; ch++){
      m_board_id = id;
      m_ch = ch;
      
      double high_integral_event, low_integral_event;

      TTree *copy_adc_integral = (TTree*)adc_integral_tree -> CopyTree(Form("board_id==%d && ch==%d && %s", id, ch, m_cut.c_str()));
      if( copy_adc_integral -> GetEntries() == 0 ) continue;

      TTree *adc_high_integral = (TTree*)copy_adc_integral -> CopyTree("adc_type==1");
      adc_high_integral -> SetBranchAddress("integral_event", &high_integral_event);
      TTree *adc_low_integral = (TTree*)copy_adc_integral -> CopyTree("adc_type==0");
      adc_low_integral -> SetBranchAddress("integral_event", &low_integral_event);

      TH1D *h_high_integral = new TH1D("h_high_integral", "h_high_integral", 
				       (m_high_histrange_max - m_high_histrange_min)/50,
				       m_high_histrange_min, m_high_histrange_max);
      TH1D *h_low_integral = new TH1D("h_low_integral", "h_low_integral", 
				      (m_low_histrange_max - m_low_histrange_min),
				      m_low_histrange_min, m_low_histrange_max);

      int high_entry_num = adc_high_integral -> GetEntries();
      int low_entry_num = adc_low_integral -> GetEntries();
      
      for(int ev=0; ev<high_entry_num; ev++){
	adc_high_integral -> GetEntry(ev);
	h_high_integral -> Fill(high_integral_event);
      }
      for(int ev=0; ev<low_entry_num; ev++){
	adc_low_integral -> GetEntry(ev);
	h_low_integral -> Fill(low_integral_event);
      }

      TF1 *f_high_integral = new TF1("f_high_integral","gaus", m_high_histrange_min, m_high_histrange_max);
      TF1 *f_low_integral = new TF1("f_low_integral","gaus", m_low_histrange_min, m_low_histrange_max);

      double high_peak_bin = m_high_histrange_min + h_high_integral->GetBinWidth(0)*h_high_integral->GetMaximumBin();
      double low_peak_bin = m_low_histrange_min + h_low_integral->GetBinWidth(0)*h_low_integral->GetMaximumBin();
      double high_hist_sigma = h_high_integral->GetStdDev();
      double low_hist_sigma = h_low_integral->GetStdDev();

      int fit_result=0;
      fit_result = h_high_integral -> Fit("f_high_integral", "Q", "", high_peak_bin - high_hist_sigma*3, high_peak_bin + high_hist_sigma*3);
      if( fit_result<0 ){
		cout << "# Warning: "
	     << "ADCH fit result is empty ( "
	     << "board: " << id
	     << ", ch: " << ch
	     << " )" << endl;
      }
	
      fit_result = h_low_integral -> Fit("f_low_integral", "Q", "", low_peak_bin - low_hist_sigma*3, low_peak_bin + low_hist_sigma*3);
      if( fit_result<0 ){
	cout << "# Warning: "
	     << "ADCL fit result is empty ( "
	     << "board: " << id
	     << ", ch: " << ch
	     << " )" << endl;
      }
      
      m_high_fit_mean = f_high_integral -> GetParameter(1);
      m_low_fit_mean = f_low_integral -> GetParameter(1);
      
      m_high_fit_chisq = f_high_integral -> GetChisquare();
      m_high_fit_ndf = f_high_integral -> GetNDF();
      m_low_fit_chisq = f_low_integral -> GetChisquare();
      m_low_fit_ndf = f_low_integral -> GetNDF();
      
      adc_calibration_tree -> Fill();
      
      delete h_high_integral;
      delete h_low_integral;
      delete f_high_integral;
      delete f_low_integral;
    }
  }



  adc_calibration_tree -> Write();
  ofile -> Close();
}

int main(int argc, char* argv[]){
  if(argc!=3){
    cout << "# Usage: " 
	 << argv[0]
	 << " [path/to/adc_integral.root] [path/to/outputdir/]"
	 << endl;
    return -1;
  }
  string input_filename = argv[1];
  string output_dir = argv[2];
  output_dir += "/";
  bool is_debug = true;

  Analizer *anal = new Analizer(input_filename,
  				output_dir,
				is_debug);
  
  delete anal;
    
}
