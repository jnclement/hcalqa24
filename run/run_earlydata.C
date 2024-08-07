
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/SubsysReco.h>
#include <jetbase/FastJetAlgo.h>
#include <jetbase/JetReco.h>
#include <jetbase/TowerJetInput.h>
#include <g4jets/TruthJetInput.h>
//#include <caloreco/CaloTowerStatus.h>
#include <jetbackground/CopyAndSubtractJets.h>
#include <jetbackground/DetermineTowerBackground.h>
#include <jetbackground/FastJetAlgoSub.h>
#include <jetbackground/RetowerCEMC.h>
#include <jetbackground/SubtractTowers.h>
#include <jetbackground/SubtractTowersCS.h>
//#include <jetbase/FastJetAlgo.h>
//#include <jetbase/JetReco.h>
//#include <jetbase/TowerJetInput.h>
//#include <g4jets/TruthJetInput.h>
#include <fstream>
#include <phool/recoConsts.h>
#include <TSystem.h>
//#include <caloreco/CaloTowerCalib.h>
#include <g4mbd/MbdDigitization.h>
#include <mbd/MbdReco.h>
#include <frog/FROG.h>
#include <ffamodules/CDBInterface.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <g4centrality/PHG4CentralityReco.h>
#include <centrality/CentralityReco.h>

#include <r24earlytreemaker/R24hcalQA.h>
//#include <chi2checker/Chi2checker.h>
#include </sphenix/user/jocl/projects/macros/common/Calo_Calib.C>
#include <CaloTowerCalib.h>
//#include <G4Setup_sPHENIX.C>
using namespace std;
//R__LOAD_LIBRARY(libchi2checker.so)
R__LOAD_LIBRARY(libr24hcalqa.so)
R__LOAD_LIBRARY(libg4centrality.so)
R__LOAD_LIBRARY(libFROG.so)
R__LOAD_LIBRARY(libg4jets.so)
R__LOAD_LIBRARY(libg4vertex.so)
R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libcalo_io.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libg4mbd.so)
R__LOAD_LIBRARY(libmbd_io.so)
R__LOAD_LIBRARY(libmbd.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libg4jets.so)
R__LOAD_LIBRARY(libjetbackground.so)
bool file_exists(const char* filename)
{
  std::ifstream infile(filename);
  return infile.good();
}
int run_earlydata(string tag = "", int nproc = 0, int debug = 0, int nevt = 0, int rn = 0, int szs = 0, int datorsim = 1, int chi2check = 0)
{
  cout << "test0" << endl;
  int verbosity = 0;
  string filename = "/sphenix/tg/tg01/jets/jocl/hcalqa/evt/"+to_string((datorsim?rn:nproc))+"/events_"+tag+(tag==""?"":"_");
  //filename += (szs?"yszs_":"nszs_")+to_string(szs)+"_"+
  filename += to_string(rn)+"_";
  filename += to_string(nproc)+"_";
  filename += to_string(nevt);
  filename += ".root";
  FROG *fr = new FROG();
  cout << "test0.5" << endl;
  gSystem->Load("libfun4all.so");
  gSystem->Load("libg4detectors.so");
  gSystem->Load("libjetbackground.so");
  gSystem->Load("libcalo_io.so");
  gSystem->Load("libg4dst.so");
  cout << "test1" << endl;
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity( verbosity );
  // just if we set some flags somewhere in this macro
  recoConsts *rc =  recoConsts::instance();
  if(datorsim) rc->set_StringFlag("CDB_GLOBALTAG","ProdA_2024");
  if(datorsim) rc->set_uint64Flag("TIMESTAMP",rn);
  ifstream list1;
  string line1;
  ifstream list2;
  string line2;
  ifstream list3;
  string line3;
  list1.open(datorsim?("./lists/"+to_string(rn)+".list"):"lists/dst_calo_waveform.list", ifstream::in);
  if(!datorsim) list2.open("lists/dst_global.list",ifstream::in);
  if(!datorsim) list3.open("lists/dst_truth_jet.list",ifstream::in);
  if(!list1)
    {
      cout << "nolist!" << endl;
      exit(1);
    }
  Fun4AllInputManager *in_1 = new Fun4AllDstInputManager("DSTin1");
  Fun4AllInputManager *in_2 = new Fun4AllDstInputManager("DSTin2");
  Fun4AllInputManager *in_3 = new Fun4AllDstInputManager("DSTin3");
  for(int i=0; i<nproc+1; i++)
    {
      getline(list1, line1);
      if(!datorsim) getline(list2, line2);
      if(!datorsim) getline(list3, line3);
    }
  in_1->AddFile(line1);
  if(!datorsim) in_2->AddFile(line2);
  if(!datorsim) in_3->AddFile(line3);
  se->registerInputManager( in_1 );
  if(!datorsim) se->registerInputManager( in_2 );
  if(!datorsim) se->registerInputManager( in_3 );
  // this points to the global tag in the CDB
  //rc->set_StringFlag("CDB_GLOBALTAG","");//"ProdA_2023");                                     
  // The calibrations have a validity range set by the beam clock which is not read out of the prdfs as of now
  Process_Calo_Calib();
  /////////////////////////////////////////////////////////////////////////////////////////////////


  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  CDBInterface::instance()->Verbosity(0);
  int cont = 0;
  cout << "test1.5" << endl;
  //Chi2checker* chi2c;
  //if(chi2check) chi2c = new Chi2checker("chi2checker",debug);
  //if(chi2check) se->registerSubsystem(chi2c);
  cout << "test2" << endl;
  R24hcalQA *tt = new R24hcalQA(filename, debug, datorsim, 1);
  cout << "test3" << endl;
  se->registerSubsystem( tt );
  cout << "test4" << endl;
  se->Print("NODETREE");
  se->run(nevt);
  se->Print("NODETREE");
  cout << "Ran all events" << endl;
  se->End();
  se->Print("NODETREE");
  cout << "Ended server" << endl;
  delete se;
  cout << "Deleted server" << endl;
  gSystem->Exit(0);
  cout << "Exited gSystem" << endl;
  return 0;

}
