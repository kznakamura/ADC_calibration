#ifndef HULREADER_HH
#define HULREADER_HH

#include <fstream>

class HulReader{
public:
  //==== Constructor and Destructor ====//
  HulReader(std::string filename, const bool is_debug=false);
  ~HulReader();
  bool readEvent();
  void showData();
  void setDebug(const bool is_debug){m_is_debug = is_debug;} 

  //==== accesser ====//
  int getTrigType(){return m_trigger_type;}
  int getTrigId(){return m_trigger_id;}
  int getEventId(){return m_event_id;}
  long getUnixtime(){return m_unixtime;}
  long getTimestamp(){return m_timestamp;}
  long getTrigLivetime(){return m_trigger_livetime;}
  long getPosttrigVetoTime(){return m_posttrigger_veto_time;}
  long getBufferfulBusyTime(){return m_bufferful_busy_time;}
  
private:
  //==== Input File ====//
  std::string m_filename;
  bool m_is_debug;
  bool openFile(std::string filename);
  std::ifstream *m_fin;
  
  //==== read paramaters ====//
  int m_trigger_type;
  int m_trigger_id;
  int m_event_id;
  long m_unixtime;
  long m_timestamp;
  long m_trigger_livetime;
  long m_posttrigger_veto_time;
  long m_bufferful_busy_time;

  int m_current_trigger_id;

  long decodeData(unsigned char *pointer, int size);

  //---- data format (byte unit) ---//
  //  const int m_event_header_size = 48;

    };


#endif
