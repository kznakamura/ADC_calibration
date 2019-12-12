#ifndef FEBREADER_HH
#define FEBREADER_HH

#include <fstream>

const int MAXCH = 56;
const int MAXDEPTH = 8192;

class FebReader{
public:
  //==== Constructor and Destructor ====//
  FebReader(std::string filename, const bool is_debug=false);
  ~FebReader();
  bool readEvent();
  void showHeader();
  bool showData(int show_depth=20);
  void setDebug(const bool is_debug){m_is_debug = is_debug;} 

  //==== accesser ====//
  //--- read data ---//
  bool getAdcType(){return m_adc_type;}
  int getEventId(){return m_event_id;}
  int getBoardId(){return m_board_id;}
  int getNumOfCh(){return m_num_of_ch;}
  bool getChMask(int ch){return m_ch_mask[ch];}
  int getDepth(){return m_depth;}
  int *getData(int ch){return m_data[ch];}
  
private:
  //==== Input File ====//
  std::string m_filename;
  bool m_is_debug;
  bool openFile(std::string filename);
  std::ifstream *m_fin;
  
  //==== read paramaters ====//
  bool m_adc_type;
  int m_event_id;
  int m_board_id;
  int m_num_of_ch;
  bool m_ch_mask[MAXCH];
  int m_depth;
  int **m_data;//[MAXCH][MAXDEPTH];
  
  int decodeData(unsigned char *pointer, int size);
};


#endif
