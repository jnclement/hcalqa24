#include "TApplication.h"
#include "TROOT.h"
#include "sPhenixStyle.C"
#include "sPhenixStyle.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TH1F.h"
#include "TRandom2.h"
#include "TMath.h"
#include <utility>  // for std::pair
#include <cstdio>
#include <iostream>
#include "TGraph.h"
#include "TTree.h"
#include "TLegend.h"
#include "TLatex.h"
#include <TColor.h>
#include "stdlib.h"
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <math.h>
#include <TMinuit.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TString.h>
#include <TAxis.h>
#include <TLine.h>
#include <TFile.h>
#include <algorithm>
#include "dlUtility.h"

float get_eta(float eta)
{
  return (eta+1.1)*24/2.2;
}
float get_phi(float phi)
{
  return (phi+M_PI)*64/(2*M_PI);
}
float bintoeta_hc(int etabin)
{
  return (2.2*etabin)/24 - 1.1;
}
float bintophi_hc(int phibin)
{
  return (2*M_PI*phibin)/64;
}
float bintoeta_em(int etabin)
{
  return (2.2*etabin)/96 - 1.1;
}
float bintophi_em(int phibin)
{
  return (2*M_PI*phibin)/256;
}
int save_etaphiE(int etabins[], int phibins[], float energies[], int nsec, string filename, int em)
{
  ofstream thefile;
  thefile.open(filename);
  for(int i=0; i<nsec; ++i)
    {
      thefile << (i>0?",":"") <<"{\"eta\": " << (em?bintoeta_em(etabins[i]):bintoeta_hc(etabins[i])) << ", \"phi\": " << (em?bintophi_em(phibins[i]):bintophi_hc(phibins[i])) << ", \"e\": " << (energies[i]>0?energies[i]:0) << ", \"event\": 0}" << endl;
    }
  thefile.close();
  return 0;
}

int quickroot(string filebase="", int njob=0)
{
  gROOT->ProcessLine( "gErrorIgnoreLevel = 1001;");
  //gStyle->SetPalette(kOcean);
  gROOT->SetStyle("Plain");
  //SetsPhenixStyle();
  gStyle->SetOptStat(0);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetOptTitle(0);
  string filename=filebase;
  TChain* tree[2];
  TChain* tree2[2];
  ifstream list[2];
  string line;
  int nosim = 1;

  list[1].open(filename,ifstream::in);
  string runnum = filename.substr(6,5);
  int rn = stoi(runnum);
  string datstr = "dat";
  string idstr;
  idstr = datstr;
  for(int h=nosim; h < 2; ++h)
    {
      int counter = 0;
      tree[h] = new TChain("ttree");
      tree2[h] = new TChain("ttree2");
      if(!list[h])
	{
	  cout << "nosimlist" << endl;
	  exit(1);
	}
      /*
      while(getline(list[h],line))
	{
	}
      */
      for(int i=0; i<10*njob; ++i)
	{
	  getline(list[h],line);
	}
      for(int i=10*njob; i<10*(njob+1); ++i)
	{
	  int breakit = 0;
	  getline(list[h],line);
	  if(list[h].eof()) breakit = 1;
	  try
	    {
	      tree[h]->Add(line.c_str());
	      tree2[h]->Add(line.c_str());
	    }
	  catch(...)
	    {
	      continue;
	    }
	  if(breakit) break;
	}
    }
  //cout << "test-1" << endl;
  long long unsigned int nevents[2] = {0};
  int evtct[2];
  int mbevt[2];
  long long unsigned int nmb[2] = {0};
  for(int h=nosim; h<2; ++h)
    {
      tree2[h]->SetBranchAddress("_evtct",&evtct[h]);
      tree2[h]->SetBranchAddress("mbevt",&mbevt[h]);
      for(int i=0; i<tree2[h]->GetEntries(); ++i)
	{
	  tree2[h]->GetEntry(i);
	  nevents[h] += evtct[h];
	  nmb[h] += mbevt[h];
	  //cout << mbevt << endl;
	}
      cout << "Total " << nevents[h] << " events." << endl;
      //if(h==0) nmb[h] = nevents[h];
      cout << "with " << nmb[h] << " minbias events." << endl;
    }
  TFile* outfile = TFile::Open(("output/root/run_"+runnum+"_"+to_string(njob)+"_fullfile.root").c_str(),"RECREATE");
  cout << outfile->GetName() << endl;
  TTree* outt = new TTree("outt","output tree");
  int trigs[4] = {0};
  trigs[0] = nmb[1];
  int outevt = nevents[1];
  int ntg[4] = {0};
  
  outt->Branch("outtrig",trigs,"outtrig[4]/I");
  outt->Branch("outevt",&outevt,"outevt/I");
  outt->Branch("ntg",ntg,"ntg[4]/I");
  outt->Branch("rn",&rn,"rn/I");
  TH1D* h1_tower_E[4][24][64];
  TH1D* h1_calo_E[4];
  TH1D* h1_etaslice_E[4][24];
  TH1D* h1_tower_timedist[4][24][64];
  TH1D* h1_calo_timedist[4];
  TH1D* h1_etaslice_timedist[4][24];
  unsigned long int brtime = 9*(1721285685/10);
  unsigned long int ertime = (11*((unsigned long int)1721308753))/10;
  unsigned long int range = ertime - brtime;

  for(int h=nosim; h<2; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  h1_calo_E[i] = new TH1D(("h1_calo_E"+to_string(i)).c_str(),"",1050,-5,100);
	  h1_calo_timedist[i] = new TH1D(("h1_calo_timedist"+to_string(i)).c_str(),"",300,-12,18);
	  for(int j=0; j<24; ++j)
	    {
	      h1_etaslice_E[i][j] = new TH1D(("h1_etaslice_E"+to_string(i)+"_"+to_string(j)).c_str(),"",1050,-5,100);
	      h1_etaslice_timedist[i][j] = new TH1D(("h1_etaslice_timedist"+to_string(i)+"_"+to_string(j)).c_str(),"",300,-12,18);
	      for(int k=0; k<64; ++k)
		{
		  h1_tower_E[i][j][k] = new TH1D(("h1_tower_E"+to_string(i)+"_"+to_string(j)+"_"+to_string(k)).c_str(),"",1050,-5,100);
		  h1_tower_timedist[i][j][k] = new TH1D(("h1_tower_timedist"+to_string(i)+"_"+to_string(j)+"_"+to_string(k)).c_str(),"",300,-12,18);
		}
	    }
	}
    }
  int evtnum[2];
  long unsigned int trigvec = 0;
  int ismb[2];
  float vtx[3];
  float mbdq[128];
  float caloE;
  int nsec;
  int etabin[1536];
  int phibin[1536];
  float calen[1536];
  float calt[1536];
  tree[1]->SetBranchAddress("triggervec",&trigvec);
  tree[1]->SetBranchAddress("ohetot",&caloE);
  for(int h=nosim; h<2; ++h)
    {
      tree[h]->SetBranchAddress("mbenrgy",mbdq);
      tree[h]->SetBranchAddress("vtx",vtx);
      tree[h]->SetBranchAddress("ismb",&ismb[h]);
      tree[h]->SetBranchAddress("_evtnum",&evtnum[h]);
      tree[h]->SetBranchAddress("sectoroh",&nsec);
      tree[h]->SetBranchAddress("ohcaletabin",etabin);
      tree[h]->SetBranchAddress("ohcalphibin",phibin);
      tree[h]->SetBranchAddress("ohcalen",calen);
      tree[h]->SetBranchAddress("ohcalt",calt);
    }

  TCanvas* c = new TCanvas("","",800,480);
  TCanvas* d = new TCanvas("","",1000,1000);
  TH1D* h1_zdist = new TH1D("h1_zdist","",200,-100,100);
  TH1D* h1_mbdq = new TH1D("h1_mbdq","",1000,0,10);
  for(int h=nosim; h<2; ++h)
    {
      for(int i=0; i<tree[h]->GetEntries(); ++i)
	{
	  tree[h]->GetEntry(i);
	  
	  if(ismb[h]) h1_zdist->Fill(vtx[2]);
	  int dtrigs[4] = {0};
	  dtrigs[0] = (trigvec >> 10) & 1;
	  dtrigs[1] = (trigvec >> 17) & 1;
	  dtrigs[2] = (trigvec >> 18) & 1;
	  dtrigs[3] = (trigvec >> 19) & 1;
	  for(int j=0; j<4; ++j)
	    {
	      if(dtrigs[j])
		{
		  if(j>0) ++trigs[j];
		  for(int k = 0; k<nsec; ++k)
		    {
		      if(calen[k] > 0.5) ntg[j]++;
		      h1_calo_E[j]->Fill(calen[k]);
		      h1_etaslice_E[j][etabin[k]]->Fill(calen[k]);
		      h1_tower_E[j][etabin[k]][phibin[k]]->Fill(calen[k]);
		      if(calen[k] > 1)
			{
			  h1_calo_timedist[j]->Fill(calt[k]);
			  h1_etaslice_timedist[j][etabin[k]]->Fill(calt[k]);
			  h1_tower_timedist[j][etabin[k]][phibin[k]]->Fill(calt[k]);		 
			}
		    }
		}
	    }
	  for(int j=0; j<128; ++j)
	    {
	      h1_mbdq->Fill(mbdq[j]);
	    }	  
	}
    }
  outt->Fill();
  outfile->WriteObject(outt,outt->GetName());
  outfile->WriteObject(h1_zdist,h1_zdist->GetName());
  outfile->WriteObject(h1_mbdq,h1_mbdq->GetName());
  for(int i=0; i<4; ++i)
    {
      outfile->WriteObject(h1_calo_E[i],h1_calo_E[i]->GetName());
      outfile->WriteObject(h1_calo_timedist[i],h1_calo_timedist[i]->GetName());
      for(int j=0; j<24; ++j)
	{
	  outfile->WriteObject(h1_etaslice_E[i][j],h1_etaslice_E[i][j]->GetName());
	  outfile->WriteObject(h1_etaslice_timedist[i][j],h1_etaslice_timedist[i][j]->GetName());
	  for(int k=0; k<64; ++k)
	    {
	      outfile->WriteObject(h1_tower_E[i][j][k],h1_tower_E[i][j][k]->GetName());
	      outfile->WriteObject(h1_tower_timedist[i][j][k],h1_tower_timedist[i][j][k]->GetName());
	    }
	}
    }
  return 0;
}
