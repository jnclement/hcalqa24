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
  int goodevt;
  int totalgood = 0;
  long long unsigned int nmb[2] = {0};
  for(int h=nosim; h<2; ++h)
    {
      tree2[h]->SetBranchAddress("_evtct",&evtct[h]);
      tree2[h]->SetBranchAddress("mbevt",&mbevt[h]);
      tree2[h]->SetBranchAddress("goodevt",&goodevt);
      for(int i=0; i<tree2[h]->GetEntries(); ++i)
	{
	  tree2[h]->GetEntry(i);
	  totalgood += goodevt;
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
  int ntg[3][4] = {0};
  
  outt->Branch("outtrig",trigs,"outtrig[4]/I");
  outt->Branch("outevt",&outevt,"outevt/I");
  outt->Branch("ntg",ntg,"ntg[3][4]/I");
  outt->Branch("rn",&rn,"rn/I");
  TH1D* h1_tower_E[3][4][96][256];
  TH1D* h1_calo_E[3][4];
  TH1D* h1_etaslice_E[3][4][96];
  TH1D* h1_tower_timedist[3][4][96][256];
  TH1D* h1_calo_timedist[3][4];
  TH1D* h1_etaslice_timedist[3][4][96];
  TH1D* h1_calo_slopes[3][4];
  TH1D* h1_etaslice_slopes[3][4][96];
  TH1D* h1_slopes_etaslice[3][4];
  unsigned long int brtime = 9*(1721285685/10);
  unsigned long int ertime = (11*((unsigned long int)1721308753))/10;
  unsigned long int range = ertime - brtime;

  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  h1_calo_slopes[h][i] = new TH1D(("h1_calo_slopes"+to_string(h)+"_"+to_string(i)).c_str(),"",1000,-10,0);
	  h1_slopes_etaslice[h][i] = new TH1D(("h1_slopes_etaslice"+to_string(h)+"_"+to_string(i)).c_str(),"",1000,-10,0);
	  h1_calo_E[h][i] = new TH1D(("h1_calo_E"+to_string(h)+"_"+to_string(i)).c_str(),"",1050,-5,100);
	  h1_calo_timedist[h][i] = new TH1D(("h1_calo_timedist"+to_string(h)+"_"+to_string(i)).c_str(),"",300,-12,18);
	  for(int j=0; j<96; ++j)
	    {
	      h1_etaslice_slopes[h][i][j] = new TH1D(("h1_etaslice_slopes"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str(),"",1000,-10,0);
	      h1_etaslice_E[h][i][j] = new TH1D(("h1_etaslice_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str(),"",1050,-5,100);
	      h1_etaslice_timedist[h][i][j] = new TH1D(("h1_etaslice_timedist"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str(),"",300,-12,18);
	      for(int k=0; k<256; ++k)
		{
		  h1_tower_E[h][i][j][k] = new TH1D(("h1_tower_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)+"_"+to_string(k)).c_str(),"",1050,-5,100);
		  h1_tower_timedist[h][i][j][k] = new TH1D(("h1_tower_timedist"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)+"_"+to_string(k)).c_str(),"",300,-12,18);
		}
	    }
	}
    }
  int evtnum[2];
  long unsigned int trigvec = 0;
  int ismb[2];
  float vtx[3];
  float mbdq[128];
  float caloE[3];
  int nsec[3];
  int etabin[3][24576];
  int phibin[3][24576];
  float calen[3][24576];
  float calt[3][24576];
  tree[1]->SetBranchAddress("triggervec",&trigvec);
  tree[1]->SetBranchAddress("ohetot",&caloE);
  for(int h=nosim; h<2; ++h)
    {
      tree[h]->SetBranchAddress("mbenrgy",mbdq);
      tree[h]->SetBranchAddress("vtx",vtx);
      tree[h]->SetBranchAddress("ismb",&ismb[h]);
      tree[h]->SetBranchAddress("_evtnum",&evtnum[h]);
      
      tree[h]->SetBranchAddress("sectorem",&nsec[0]);
      tree[h]->SetBranchAddress("emcaletabin",etabin[0]);
      tree[h]->SetBranchAddress("emcalphibin",phibin[0]);
      tree[h]->SetBranchAddress("emcalen",calen[0]);
      tree[h]->SetBranchAddress("emcalt",calt[0]);

      tree[h]->SetBranchAddress("sectorih",&nsec[1]);
      tree[h]->SetBranchAddress("ihcaletabin",etabin[1]);
      tree[h]->SetBranchAddress("ihcalphibin",phibin[1]);
      tree[h]->SetBranchAddress("ihcalen",calen[1]);
      tree[h]->SetBranchAddress("ihcalt",calt[1]);

      tree[h]->SetBranchAddress("sectoroh",&nsec[2]);
      tree[h]->SetBranchAddress("ohcaletabin",etabin[2]);
      tree[h]->SetBranchAddress("ohcalphibin",phibin[2]);
      tree[h]->SetBranchAddress("ohcalen",calen[2]);
      tree[h]->SetBranchAddress("ohcalt",calt[2]);
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
		  for(int q=0; q<3; ++q)
		    {
		      for(int k = 0; k<nsec[q]; ++k)
			{
			  if(calen[q][k] > 0.5) ntg[q][j]++;
			  h1_calo_E[q][j]->Fill(calen[q][k]);
			  h1_etaslice_E[q][j][etabin[q][k]]->Fill(calen[q][k]);
			  h1_tower_E[q][j][etabin[q][k]][phibin[q][k]]->Fill(calen[q][k]);
			  if(calen[q][k] > 0.5)
			    {
			      h1_calo_timedist[q][j]->Fill(calt[q][k]);
			      h1_etaslice_timedist[q][j][etabin[q][k]]->Fill(calt[q][k]);
			      h1_tower_timedist[q][j][etabin[q][k]][phibin[q][k]]->Fill(calt[q][k]);		 
			    }
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

  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  for(int j=0; j<(h==0?96:24); ++j)
	    {
	      h1_etaslice_E[h][i][j]->Fit("expo","L Q S","",0.25,2);
	      h1_slopes_etaslice[h][i]->Fill(h1_etaslice_E[h][i][j]->GetFunction("expo")->GetParameter(1));
	      for(int k=0; k<(h==0?256:64); ++k)
		{
		  h1_tower_E[h][i][j][k]->Fit("expo","L Q S","",0.25,2);
		  float slope = h1_tower_E[h][i][j][k]->GetFunction("expo")->GetParameter(1);
		  h1_etaslice_slopes[h][i][j]->Fill(slope);
		  h1_calo_slopes[h][i]->Fill(slope);
		}
	    }
	}
    }
  outt->Fill();
  outfile->WriteObject(outt,outt->GetName());
  outfile->WriteObject(h1_zdist,h1_zdist->GetName());
  outfile->WriteObject(h1_mbdq,h1_mbdq->GetName());
  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  outfile->WriteObject(h1_calo_E[h][i],h1_calo_E[h][i]->GetName());
	  outfile->WriteObject(h1_calo_slopes[h][i],h1_calo_slopes[h][i]->GetName());
	  outfile->WriteObject(h1_calo_timedist[h][i],h1_calo_timedist[h][i]->GetName());
	  outfile->WriteObject(h1_slopes_etaslice[h][i],h1_slopes_etaslice[h][i]->GetName());
	  for(int j=0; j<(h==0?96:24); ++j)
	    {
	      outfile->WriteObject(h1_etaslice_E[h][i][j],h1_etaslice_E[h][i][j]->GetName());
	      outfile->WriteObject(h1_etaslice_timedist[h][i][j],h1_etaslice_timedist[h][i][j]->GetName());
	      outfile->WriteObject(h1_etaslice_E[h][i][j],h1_etaslice_E[h][i][j]->GetName());
	      outfile->WriteObject(h1_etaslice_slopes[h][i][j],h1_etaslice_slopes[h][i][j]->GetName());
	      for(int k=0; k<(h==0?256:64); ++k)
		{
		  outfile->WriteObject(h1_tower_E[h][i][j][k],h1_tower_E[h][i][j][k]->GetName());
		  outfile->WriteObject(h1_tower_timedist[h][i][j][k],h1_tower_timedist[h][i][j][k]->GetName());
		}
	    }
	}
    }
  return 0;
}
