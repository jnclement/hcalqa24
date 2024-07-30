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
#include <TDatime.h>
void get_timestamps(int runnumber, unsigned int* timestamp)
{

  string datestamp;
  TSQLServer *db = TSQLServer::Connect("pgsql://sphnxdaqdbreplica:5432/daq","phnxro","");

  if (db)
    {
      printf("Server info: %s\n", db->ServerInfo());
    }
  else
    {
      printf("bad\n");
    }


  TSQLRow *row;
  TSQLResult *res;
  TString cmd = "";
  char sql[1000];

  cout << runnumber << endl;
  sprintf(sql, "select brtimestamp from run where runnumber = %d;", runnumber);

  res = db->Query(sql);

  int nrows = res->GetRowCount();
  
  int nfields = res->GetFieldCount();
  for (int i = 0; i < nrows; i++) {
    row = res->Next();
    for (int j = 0; j < nfields; j++) {
      datestamp = row->GetField(j);
    }
    delete row;
  }
  delete res;
  cout << datestamp << endl;
  TDatime das = TDatime(datestamp.c_str());
  *timestamp = das.Convert();
}

void get_scaledowns(int runnumber, int scaledowns[])
{

  TSQLServer *db = TSQLServer::Connect("pgsql://sphnxdaqdbreplica:5432/daq","phnxro","");

  if (db)
    {
      printf("Server info: %s\n", db->ServerInfo());
    }
  else
    {
      printf("bad\n");
    }


  TSQLRow *row;
  TSQLResult *res;
  TString cmd = "";
  char sql[1000];

  cout << runnumber << endl;
  for (int is = 0; is < 64; is++)
    {
      sprintf(sql, "select scaledown%02d from gl1_scaledown where runnumber = %d;", is, runnumber);

      res = db->Query(sql);

      int nrows = res->GetRowCount();

      int nfields = res->GetFieldCount();
      //cout << nrows << " " << nfields << endl;   
      for (int i = 0; i < nrows; i++) {
        row = res->Next();
        for (int j = 0; j < nfields; j++) {
          scaledowns[is] = stoi(row->GetField(j));
          //cout << is << endl;       
          if(is == 10 || is==17 || is==18 || is==19) cout  << is << ":" << scaledowns[is] << " ";
        }
        delete row;
      }


      delete res;
    }
  cout << endl;
  return 0;
}

int plot_runbyrun()
{
  TCanvas* c = new TCanvas("","",1000,1000);
  gROOT->ProcessLine( "gErrorIgnoreLevel = 1001;");
  const int nruns = 33;//72; 
  const int nsd = 64;
  int sds[nruns][nsd] = {0};
  unsigned int timestamps[nruns];
  TFile* rootfile;
  long long unsigned int nmb = 0;
  int nJetTrig[4] = {0};
  float effevt[3] = {0};
  int whichrun = 0;
  int runmb = 0;
  int nfile = 0;
  int mbevt = 0;
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0);
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetOptTitle(0);

  int runnumbers[nruns] = {47474,47476,47480,47481,47484,47485,47491,47492,47494,47495,47503,47505,47506,47507,47513,47514,47516,47522,47524,47525,47540,47548,47552,47557,47568,47634,47636,47638,47657,47658,47659,47661,47662};// {47002,47006,47013,47017,47021,47034,47038,47042,47052,47056,47060,47068,47082,47098,47102,47114,47118,47122,47126,47130,47135,47139,47143,47156,47161,47197,47203,47216,47220,47286,47291,47298,47305,47310,47314,47325,47333,47360,47375,47379,47389,47394,47398,47439,47443,47451,47455,47459,47463,47476,47480,47484,47488,47492,47497,47505,47509,47516,47521,47525,47539,47549,47557,47567,47617,47637,47650,47654,47659,47663,47667,47724};//{47002,47005,47007,47009,47017,47019,47032,47037,47056,47060,47289,47293,47297,47298,47303,47305,47306,47308,47309,47310,47315,47316,47323,47325,47330,47334,47360,47377,47378,47379,47381,47382,47391,47393,47394,47395,47396,47397,47398,47399,47443,47445,47451,47455,47456,47457,47458,47459,47460,47464,47474,47480,47481,47484,47485,47491,47492,47494,47495,47497,47502,47503,47505,47506,47507,47508,47513,47514,47516,47522,47524,47525,47540,47548,47552,47557,47558,47568,47634,47636,47657,47658,47659,47661,47662,47666,47667,/*47698,*/47715,47716,47717,47718,47720,47722,47723,47724,47725,47726,47727,47728,47729,47730,47731,47732,47733};//,47741,47745,47746,47747,47748,47749,47751,47752,47753,47757,47758};  
  unsigned int uut = 1720509987;
  unsigned int lut = 1719629344;
  TH1D* tinty[4];
  for(int i=0; i<4; ++i)
    {
      tinty[i] = new TH1D(("tinty"+to_string(i)).c_str(),"",(uut-lut),lut,uut);
      tinty[i]->GetXaxis()->SetTimeDisplay(1);
      tinty[i]->GetXaxis()->SetNdivisions(-10);
      tinty[i]->GetXaxis()->SetTimeFormat("%m\/%d");
    }
  for(int i=0; i<nruns; ++i)
    {
      get_scaledowns(runnumbers[i],sds[i]);
      get_timestamps(runnumbers[i],&timestamps[i]);
    }
  while(nfile < nruns)
    {
      rootfile = TFile::Open(("output/sumroot/summed_"+to_string(runnumbers[whichrun])+"_dat_base.root").c_str());
      TTree* tree = (TTree*)rootfile->Get("outt");
      tree->SetBranchAddress("outnmb",&mbevt);
      tree->SetBranchAddress("nJetTrig",nJetTrig);
      for(int i=0; i<tree->GetEntries(); ++i)
        {
          tree->GetEntry(i);
          nmb += (sds[whichrun][10]==-1?0:1)*mbevt;
          runmb += (sds[whichrun][10]==-1?0:1)*mbevt;
        }
      if(sds[whichrun][17] != -1) effevt[0] += (1.*(1+sds[whichrun][10]))/(1.*(1+sds[whichrun][17]))*runmb; 
      if(sds[whichrun][18] != -1) effevt[1] += (1.*(1+sds[whichrun][10]))/(1.*(1+sds[whichrun][18]))*runmb; 
      if(sds[whichrun][19] != -1) effevt[2] += (1.*(1+sds[whichrun][10]))/(1.*(1+sds[whichrun][19]))*runmb;
      runmb = 0;
      TH1D* jetE[4];
      TH1D* jetTrigE[4];
      for(int i=0; i<3; ++i)
	{
	  jetE[i] = (TH1D*)rootfile->Get(("jetE"+to_string(1)+"_"+to_string(i)).c_str());
	  jetE[i]->GetXaxis()->SetTitle("E_{T,jet} [GeV]");

	  jetTrigE[i] = (TH1D*)rootfile->Get(("jetTrigE"+to_string(i+1)).c_str());
	  
	}
      int lowintbin = 100;
      //cout << jetE[0] << endl;
      float njet = jetE[0]->Integral(lowintbin,400);
      //cout << njet << endl;
      //for(int i=0; i<njet; ++i)
      //{
      //tinty[0]->Sumw2();jetE[i]->Scale(1./(nmb));
      tinty[0]->SetBinContent(timestamps[whichrun]-lut+1,njet/nmb);
      tinty[0]->SetBinError(timestamps[whichrun]-lut+1,sqrt(njet)/nmb);
      //}
      
      for(int i=0; i<3; ++i)
	{
	  int njet2 = jetTrigE[i]->Integral(lowintbin,400);
	  if(effevt[i] != 0)
	    {
	      tinty[i+1]->SetBinContent(timestamps[whichrun]-lut+1,njet2/effevt[i]);
	      tinty[i+1]->SetBinError(timestamps[whichrun]-lut+1,sqrt(njet2)/effevt[i]);
	    }
	}
      
      whichrun++;
      ++nfile;
      nmb = 0;
      for(int i=0; i<3; ++i)
	{
	  effevt[i] = 0;
	}
    }
  tinty[0]->GetXaxis()->SetRangeUser(lut,uut);
  
  int colors[4] = {kBlack, kMagenta+2, kBlue+2, kGreen+3};
  int mstyle[4] = {20,21,24,25};
  tinty[0]->GetXaxis()->SetTitle("Date [month/day of 2024]");
  tinty[0]->GetYaxis()->SetTitle("Total Integrated Per Event Jet #it{E}_{T}>10 GeV Yield");
  for(int i=0; i<4; ++i)
    {
      tinty[i]->SetMarkerSize(1.5);
      tinty[i]->SetMarkerColor(colors[i]);
      tinty[i]->SetMarkerStyle(mstyle[i]);
      tinty[i]->SetLineWidth(2);
    }
  TH1D* tintyrat[3];
  for(int i=0; i<3; ++i)
    {
      tintyrat[i] = new TH1D(("tintyrat"+to_string(i)).c_str(),"",(uut-lut),lut,uut);
      tintyrat[i]->GetXaxis()->SetTimeDisplay(1);
      tintyrat[i]->GetXaxis()->SetNdivisions(-10);
      tintyrat[i]->GetXaxis()->SetTimeFormat("%m\/%d");
    }
  tintyrat[0]->Divide(tinty[1],tinty[0]);
  tintyrat[1]->Divide(tinty[2],tinty[0]);
  tintyrat[2]->Divide(tinty[3],tinty[0]);
  TLegend* leg = new TLegend(0.2,0.7,0.5,0.9);
  leg->SetFillStyle(0);
  leg->SetFillColor(0);
  leg->SetTextFont(42);
  leg->SetBorderSize(0);
  leg->AddEntry(tinty[0],"MBD N/S >= 1");
  tinty[0]->GetYaxis()->SetRangeUser(0,0.1e-3);
  tinty[0]->GetXaxis()->SetLabelSize(0.02);
  tinty[0]->GetYaxis()->SetLabelSize(0.02);
  tinty[0]->GetYaxis()->SetTitleOffset(2.0);
  leg->AddEntry(tinty[1],"Jet 8 GeV \& MBD N/S >= 1");
  leg->AddEntry(tinty[2],"Jet 10 GeV \& MBD N/S >= 1");
  leg->AddEntry(tinty[3],"Jet 12 GeV \& MBD N/S >= 1");
  gPad->SetLeftMargin(0.2);
  tinty[0]->Draw("PE");
  tinty[1]->Draw("SAME PE");
  tinty[2]->Draw("SAME PE");
  tinty[3]->Draw("SAME PE");
  leg->Draw();
  gPad->SaveAs("output/rmg/tinty.pdf");
  tintyrat[0]->SetMarkerStyle(21);
  tintyrat[1]->SetMarkerStyle(24);
  tintyrat[2]->SetMarkerStyle(25);
  tintyrat[0]->SetMarkerColor(kMagenta+2);
  tintyrat[1]->SetMarkerColor(kBlue+2);
  tintyrat[2]->SetMarkerColor(kGreen+3);
  tintyrat[0]->Draw("PE");
  tintyrat[1]->Draw("SAME PE");
  tintyrat[2]->Draw("SAME PE");
  gPad->SaveAs("output/rmg/tintyrat.pdf");
  return 0;
}
