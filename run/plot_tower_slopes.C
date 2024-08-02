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

double get_rate(int runnumber, int ntrig)
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
  char sql[1000];
  double rate;
  //cout << runnumber << endl;
  sprintf(sql, "SELECT  rate_avg_value  FROM run_timeseries_db_summary_rate WHERE time_series_name = 'sphenix_gtm_gl1_trigger_scalar' and labels->>'gl1_trigger_id' = '%d' and labels->>'type' = 'live' and run_number = %d;", ntrig, runnumber);
      //printf("%s \n" , sql);

  res = db->Query(sql);

  int nrows = res->GetRowCount();

  int nfields = res->GetFieldCount();
  for (int i = 0; i < nrows; i++)
    {
      row = res->Next();
      for (int j = 0; j < nfields; j++)
	{
	  rate = stoi(row->GetField(j));
	}
      delete row;
    }
  delete res;
  delete db;
  return rate;
}

void get_bert(int runnumber, double rts[])
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
  char sql[1000];

  //cout << runnumber << endl;
  sprintf(sql, "SELECT extract(epoch from brtimestamp), extract(epoch from ertimestamp) FROM run WHERE runnumber = %d;", runnumber);
      //printf("%s \n" , sql);

  res = db->Query(sql);

  int nrows = res->GetRowCount();
  //cout << nrows << endl;
  int nfields = res->GetFieldCount();
  //cout << nfields << endl;
  for (int i = 0; i < nrows; i++)
    {
      row = res->Next();
      for (int j = 0; j < nfields; j++)
	{
	  rts[j] = stof(row->GetField(j));
	}
      //cout << rts[0] << " " << rts[1] << endl;
      delete row;
    }
  delete res;
  delete db;
}

float get_dicor(float rate, double rt)
{
  double ratel =  TMath::Abs(rate)/(111*78e3);
  double lambda = -1.*TMath::Log(1-ratel);
  return TMath::Abs(lambda*rt*(111*78e3));
}

int plot_tower_slopes(int rn = -1, int etabin = -1, string filebase = "summed_dat.root", string filelist="lists/sumdatlist.list")
{
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetOptStat(0);
  TH1D* h1_E[3][4][96][256];
  TH1D* h1_etaslice_E[3][4][96];
  TH1D* h1_slope[3][4];
  TH1D* h1_etaslice_slopes[3][4][96];
  TH1D* h1_slopes_etaslice[3][4];
  TFile* thefile;
  TTree* thetree;
  if(rn == -1)
    {
      int runevt[10][4] = {0};
      int runt[10];
      int trigs[4] = {10,17,18,19};
      //float rates[4] = {0};
      double rts[2];
      int runnumber;
      int ntg[3][4];
      int whichrun = 0;
      int runnumbers[10] = {48232,48233,48236,48237,48238,48239,48240,48244,48245,48246};
      int runcounter = 0;
      int nevt[4];
      thefile = TFile::Open(filebase.c_str());
      thetree = (TTree*)thefile->Get("outt");
      //cout << thetree << " " << thetree->GetEntries() << endl;
      thetree->SetBranchAddress("ntg",ntg);
      thetree->SetBranchAddress("rn",&runnumber);
      thetree->SetBranchAddress("outtrig",nevt);
      //cout << "rn -1" << endl;
      for(int i=0; i<thetree->GetEntries(); ++i)
	{
	  thetree->GetEntry(i);
	  for(int j=0; j<10; ++j)
	    {
	      if(runnumbers[j] == runnumber)
		{
		  runcounter = j;
		  break;
		}
	    }

	  for(int j=0; j<4; ++j)
	    {
	      runevt[runcounter][j] += nevt[j];//*get_dicor(rate, rt)/(rt*rate);
	    }
	}
    }
  else
    {
      thefile = TFile::Open(("output/sumroot/summed_"+to_string(rn)+"_base.root").c_str());
    }
  if(!thetree)
    {
      thetree = (TTree*)thefile->Get("outt");
    }
  int nevt[4];
  int totalevt[4] = {0};
  thetree->SetBranchAddress("outtrig",nevt);
  for(int i=0; i<thetree->GetEntries(); ++i)
    {
      thetree->GetEntry(i);
      for(int j=0; j<4; ++j)
	{
	  totalevt[j] += nevt[j];
	}
    }
  float range[3] = {-15,-25,-7};
  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  h1_slope[h][i] = new TH1D(("h1_slope"+to_string(h)+"_"+to_string(i)).c_str(),"",100,range[h],0);
	  h1_slopes_etaslice[h][i] = new TH1D(("h1_slopes_etaslice"+to_string(h)+"_"+to_string(i)).c_str(),"",100,range[h],0);
	  for(int j=0; j<(h==0?96:24); ++j)
	    {
	      h1_etaslice_E[h][i][j] = (TH1D*)thefile->Get(("h1_etaslice_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str());
	      h1_etaslice_E[h][i][j]->Scale(1./totalevt[i]);
	      h1_etaslice_E[h][i][j]->Fit("expo","Q L S","",0.5,3);
	      h1_etaslice_slopes[h][i][j] = new TH1D(("h1_etaslice_slopes"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str(),"",100,range[h],0);
	      h1_slopes_etaslice[h][i]->Fill(h1_etaslice_E[h][i][j]->GetFunction("expo")->GetParameter(1));
	      if(j!=etabin && etabin!=-1) continue;
	      for(int k=0; k<(h==0?256:64); ++k)
		{
		  h1_E[h][i][j][k] = (TH1D*)thefile->Get(("h1_tower_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)+"_"+to_string(k)).c_str());
		  h1_E[h][i][j][k]->Scale(1./totalevt[i]);
		  h1_E[h][i][j][k]->Fit("expo","Q L S","",0.5,2);
		  float slope = h1_E[h][i][j][k]->GetFunction("expo")->GetParameter(1);
		  h1_slope[h][i]->Fill(slope);
		  h1_etaslice_slopes[h][i][j]->Fill(slope);
		}
	    }
	}
    }
  //cout << h1_E[0]->GetBinContent(50) << " " << h1_E[0]->GetBinContent(51) <<" " << h1_E[0]->GetBinContent(52) << " " << h1_E[0]->GetBinContent(53) << endl;

  int colors[4] = {kBlack, kGreen+2, kMagenta+2, kBlue+2};
  int marker[4] = {20, 21, 24, 25};
  string texts[4] = {"MBD N\&S>=1","Jet 8 GeV \& MBD N\&S>=1","Jet 10 GeV \& MBD N\&S>=1","Jet 12 GeV \& MBD N\&S>=1"};
  TLegend* leg = new TLegend(0.1,0.7,0.55,0.85);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  for(int i=0; i<4; ++i)
    {
      h1_slopes_etaslice[0][i]->SetMarkerStyle(marker[i]);
      h1_slopes_etaslice[0][i]->SetMarkerColor(colors[i]);
      leg->AddEntry(h1_slopes_etaslice[0][i],texts[i].c_str(),"p");
    }
  string cal[3] = {"EMCal","IHCal","OHCal"};
  for(int h=0; h<((etabin==-1 || etabin<24)?3:1); ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->GetXaxis()->SetTitle((cal[h]+" Tower Slope [GeV^{-1}]").c_str());
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->GetYaxis()->SetTitle("Counts");
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->GetYaxis()->SetTitleOffset(1.5);
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->SetMarkerStyle(marker[i]);
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->SetMarkerColor(colors[i]);
	  if(etabin>=0)
	    {
	      h1_etaslice_slopes[h][i][etabin]->GetXaxis()->SetTitle((cal[h]+" Tower Slope [GeV^{-1}]").c_str());
	      h1_etaslice_slopes[h][i][etabin]->GetYaxis()->SetTitle("Counts");
	      h1_etaslice_slopes[h][i][etabin]->GetYaxis()->SetTitleOffset(1.5);
	      h1_etaslice_slopes[h][i][etabin]->SetMarkerStyle(marker[i]);
	      h1_etaslice_slopes[h][i][etabin]->SetMarkerColor(colors[i]);
	    }
	}
      
      TCanvas* c = new TCanvas("","",1000,1000);

      gPad->SetLogy();

      (etabin==-1?h1_slope[h][0]:h1_slopes_etaslice[h][0])->Draw("PE");
      for(int i=1; i<4; ++i)
	{
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->Draw("SAME PE");
	}
      leg->Draw();
      c->SaveAs(("output/rmg/slope"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+".pdf").c_str());
      
      if(etabin>=0)
	{
	  h1_etaslice_slopes[h][0][etabin]->Draw("PE");
	  for(int i=1; i<4; ++i)
	    {
	      h1_etaslice_slopes[h][i][etabin]->Draw("SAME PE");
	    }
	  leg->Draw();
	  c->SaveAs(("output/rmg/etaslice_slopes"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+".pdf").c_str());
	}
    }

  return 0;
}
