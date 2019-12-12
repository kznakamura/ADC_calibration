//#include "FebReader.hh"
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

  int m_board_list[3] = {1, 2, 17};
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

  TCanvas *c = new TCanvas("c", "c", 100, 100, 700, 500);
  c -> cd();
  TH2D *frame = new TH2D("frame", ";low_fit_mean;high_fit_mean", 79, 0, 70, 60000, 0 ,60000);
  frame -> SetStats(0);
  frame->GetYaxis()->SetTitleOffset(1.2);
  frame -> Draw();
  
  int board_list_length = end(m_board_list) - begin(m_board_list);
  TLegend *legend = new TLegend(0.6, 0.88 - 0.05*board_list_length, 0.8, 0.88);
  adc_calibration_tree -> SetMarkerStyle(20);

  int board_num = 0;
  for(auto board : m_board_list){
    board_num++;
    adc_calibration_tree -> SetMarkerColor(board_num);
    adc_calibration_tree ->Draw(Form("high_fit_mean:low_fit_mean>>h_%d",board), Form("board_id==%d", board), "same");
    legend->AddEntry( gROOT->FindObject(Form("h_%d", board)), Form("board_id=%d", board) , "p");
    legend -> Draw();
  }

  //app.Run();
  if(m_save_canvas==true){
    string canvas_name = m_canvas_dir + "adc_fit_mean.png";
    c -> Print(canvas_name.c_str());
    cout << "# Message: "
	 << canvas_name << " is saved"
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
  bool save_canvas = true;

  Analizer *anal = new Analizer(input_filename,
				canvas_dir,
				save_canvas);
  
  delete anal;

}
