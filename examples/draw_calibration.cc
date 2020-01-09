#include <iostream>

#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TApplication.h>


using namespace std;

class Analizer{
public:
  Analizer(string input_filename, 
	   string canvas_dir,
	   bool save_canvas);

private:
  string m_input_filename;
  string m_canvas_dir;
  bool m_save_canvas;

  int m_board_list[3] = {1, 17, 2};
};

Analizer::Analizer(string input_filename,
		   string canvas_dir,
		   bool save_canvas){

  m_input_filename = input_filename;
  m_canvas_dir = canvas_dir;
  m_save_canvas = save_canvas;

  TFile *ifile = new TFile(input_filename.c_str());
  TTree *adc_calibration_tree = (TTree*)ifile->Get("adc_calibration_tree");
  
  //TApplication app("app", 0, 0, 0, 0);

  TCanvas *c_1 = new TCanvas("c_1", "c_1", 100, 100, 700, 500);
  c_1 -> cd();
  TH2D *frame_1 = new TH2D("frame_1", "adc fit mean;low_fit_mean;high_fit_mean", 100, 0, 100, 20000, 0 ,20000);
  frame_1 -> SetStats(0);
  frame_1->GetYaxis()->SetTitleOffset(1.2);
  frame_1 -> Draw();

  TCanvas *c_2 = new TCanvas("c_2", "c_2", 100, 100, 700, 500);
  c_2 -> cd();
  TH2D *frame_2 = new TH2D("frame_2", "gain ratio (integral_high/40)/(low/5);ch;gain ratio", 56, 0, 56, 60, 0 ,60);
  frame_2 -> SetStats(0);
  frame_2->GetYaxis()->SetTitleOffset(1.2);
  frame_2 -> Draw();

  int board_list_length = end(m_board_list) - begin(m_board_list);
  TLegend *legend = new TLegend(0.6, 0.38 - 0.05*board_list_length, 0.8, 0.38);
  adc_calibration_tree -> SetMarkerStyle(20);

  int board_num = 0;
  for(auto board : m_board_list){
    board_num++;
    adc_calibration_tree -> SetMarkerColor(board_num);
    c_1 -> cd();
    adc_calibration_tree ->Draw(Form("high_fit_mean:low_fit_mean>>h_%d",board), Form("board_id==%d", board), "same");
    legend->AddEntry( gROOT->FindObject(Form("h_%d", board)), Form("board_id=%d", board) , "p");
    legend -> Draw();
    c_2 -> cd();
    adc_calibration_tree ->Draw(Form("(high_fit_mean/40)/(low_fit_mean/5):ch>>h_%d",board), Form("board_id==%d", board), "same");
    legend -> Draw();
  }

  //app.Run();
  if(m_save_canvas==true){
    string canvas_name_1 = m_canvas_dir + "adc_fit_mean.png";
    string canvas_name_2 = m_canvas_dir + "gain_ratio.png";
    c_1 -> Print(canvas_name_1.c_str());
    c_2 -> Print(canvas_name_2.c_str());
    cout << "# Message: "
	 << canvas_name_1 << " is saved"
	 << endl;
    cout << "# Message: "
	 << canvas_name_2 << " is saved"
	 << endl;
	
  }

  
}

int main(int argc, char* argv[]){
  if(argc!=3){
    cout << "Usage: " 
	 << argv[0]
	 << " [path/to/adc_calibration.root] [path/to/canvasdir/]"
	 << endl;
    return -1;
  }

  string input_filename = argv[1];
  string canvas_dir = argv[2];
  canvas_dir += "/";
  bool save_canvas = true;

  Analizer *anal = new Analizer(input_filename,
				canvas_dir,
				save_canvas);
  
  delete anal;

}
