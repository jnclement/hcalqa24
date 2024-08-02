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

int plot(int rn = -1, int etabin = -1, int phibin = -1, string filebase = "summed_dat.root", string filelist="lists/sumdatlist.list")
{
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetOptStat(0);
  long int brtime = 1720843200;
  long int ertime = 1720886400;
  long int rntime = ertime-brtime;
  double rates[4][10];
  double rates10[10] = {1096139.5597489611, 1036221.461018759, 875180.1197390455,  891821.471314737, 884244.4745453877,  842631.249426887, 855117.9394749411, 849957.5479539573, 848765.9743317118, 822783.6522224195};
  double rates17[10] = {969.5414623266898, 834.2124488552087, 674.3479785558625, 678.2691532766138, 664.9496810289806, 631.7805561898182, 638.7427728929898, 631.6199123979808, 631.0232371951545, 610.7409875953092};
  double rates18[10] = {262.20818559371725, 214.37475737396235, 169.7226024345818, 170.0111400821548, 165.85673186107292, 157.59101197418235, 159.3607062311657, 157.39092938271605, 157.19426767955056, 151.95488965650557};
  double rates19[10] = {71.89399328746018, 57.78656369249358, 45.33766927554115, 45.282545914563585, 44.01830472419711, 42.07545052038432, 42.424056415774494, 41.37895501672445, 41.90388793351682, 40.30589606154157};
  for(int i=0; i<10; ++i)
    {
      rates[0][i] = rates10[i];
      rates[1][i] = rates17[i];
      rates[2][i] = rates18[i];
      rates[3][i] = rates19[i];
    }

  TH1D* h1_ntg[3][4];
  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  h1_ntg[h][i] = new TH1D(("h1_ntg"+to_string(h)+"_"+to_string(i)).c_str(),"",rntime,brtime,ertime);
	}
    }
  TH1D* h1_E[3][4];
  TH1D* h1_time[3][4];
  TH1D* h1_slope[3][4];
  TH1D* h1_etaslice_slopes[3][4];
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

	  //cout << "getting bert" << endl;
	  get_bert(runnumber, rts);
	  //cout << "got bert" << endl;
	  double rt = rts[1]-rts[0];
	  runt[runcounter] = rts[0];
	  for(int j=0; j<4; ++j)
	    {
	      double rate = rates[j][runcounter];//get_rate(runnumber,trigs[j]);
	      //cout << nevt[j] << endl;
	      cout << get_dicor(rate,rt) << endl;
	      for(int h=0; h<3; ++h)
		{
		  if(nevt[j] != 0) h1_ntg[h][j]->Fill(rts[0],ntg[h][j]);
		}
	      runevt[runcounter][j] += nevt[j];//*get_dicor(rate, rt)/(rt*rate);
	      //cout << h1_ntg[j]->Integral() << endl;
	    }
	}
      for(int h=0; h<3; ++h)
	{
	  for(int i=0; i<10; ++i)
	    {
	      for(int j=0; j<4; ++j)
		{
		  //cout << runevt[i][j] << endl;
		  if(runevt[i][j] != 0)
		    {
		      //cout << i << " " << j << endl;
		      //cout << runevt[i][j] << endl;
		      //cout << h1_ntg[j]->GetBinContent(h1_ntg[j]->FindBin(runt[i])) << endl;
		      //cout << h1_ntg[j]->GetBinContent(h1_ntg[j]->FindBin(runt[i]))/runevt[i][j] << endl;
		      h1_ntg[h][j]->SetBinContent(h1_ntg[h][j]->FindBin(runt[i]),h1_ntg[h][j]->GetBinContent(h1_ntg[h][j]->FindBin(runt[i]))/runevt[i][j]);
		      h1_ntg[h][j]->SetBinError(h1_ntg[h][j]->FindBin(runt[i]),sqrt(h1_ntg[h][j]->GetBinContent(h1_ntg[h][j]->FindBin(runt[i])))/runevt[i][j]);
		    }
		  else
		    {
		      h1_ntg[h][j]->SetBinContent(h1_ntg[h][j]->FindBin(runt[i]),0);
		      h1_ntg[h][j]->SetBinError(h1_ntg[h][j]->FindBin(runt[i]),0);
		    }
		}
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
  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  if(etabin < 0)
	    {
	      h1_slope[h][i] = (TH1D*)thefile->Get(("h1_calo_slopes"+to_string(h)+"_"+to_string(i)).c_str());
	      h1_E[h][i] = (TH1D*)thefile->Get(("h1_calo_E"+to_string(h)+"_"+to_string(i)).c_str());
	      h1_time[h][i] = (TH1D*)thefile->Get(("h1_calo_timedist"+to_string(h)+"_"+to_string(i)).c_str());
	    }
	  else if(phibin < 0)
	    {
	      h1_slope[h][i] = (TH1D*)thefile->Get(("h1_slopes_etaslice"+to_string(h)+"_"+to_string(i)).c_str());
	      h1_etaslice_slopes[h][i] = (TH1D*)thefile->Get(("h1_etaslice_slopes"+to_string(h)+"_"+to_string(i)+"_"+to_string(etabin)).c_str());
	      h1_E[h][i] = (TH1D*)thefile->Get(("h1_etaslice_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(etabin)).c_str());
	      h1_time[h][i] = (TH1D*)thefile->Get(("h1_etaslice_timedist"+to_string(h)+"_"+to_string(i)+"_"+to_string(etabin)).c_str());
	    }
	  else
	    {
	      h1_slope[h][i] = (TH1D*)thefile->Get(("h1_slopes_etaslice"+to_string(h)+"_"+to_string(i)).c_str());
	      h1_etaslice_slopes[h][i] = (TH1D*)thefile->Get(("h1_etaslice_slopes"+to_string(h)+"_"+to_string(i)+"_"+to_string(etabin)).c_str());
	      h1_E[h][i] = (TH1D*)thefile->Get(("h1_tower_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(etabin)+"_"+to_string(phibin)).c_str());
	      h1_time[h][i] = (TH1D*)thefile->Get(("h1_tower_timedist"+to_string(h)+"_"+to_string(i)+"_"+to_string(etabin)+"_"+to_string(phibin)).c_str());
	    }
	  //cout << "test1" << endl;
	  h1_slope[h][i]->Scale(1./(254));//totalevt[i]);
	  //cout << "test2" << endl;
	  if(etabin>-1) h1_etaslice_slopes[h][i]->Scale(1./(254));///totalevt[i]);
	  //cout << "test3" << endl;
	  h1_E[h][i]->Scale(1./totalevt[i]);
	  h1_time[h][i]->Scale(1./totalevt[i]);
	  //h1_ntg[i]->Scale(1./totalevt[i]);
	}
    }
  //cout << h1_E[0]->GetBinContent(50) << " " << h1_E[0]->GetBinContent(51) <<" " << h1_E[0]->GetBinContent(52) << " " << h1_E[0]->GetBinContent(53) << endl;
  int colors[4] = {kBlack, kGreen+2, kMagenta+2, kBlue+2};
  int marker[4] = {20, 21, 24, 25};
  string texts[4] = {"MBD N\&S>=1","Jet 8 GeV \& MBD N\&S>=1","Jet 10 GeV \& MBD N\&S>=1","Jet 12 GeV \& MBD N\&S>=1"};
  float max[3] = {0};
  float min[3] = {1000000000,1000000000,100000000};
  TLegend* leg = new TLegend(0.45,0.6,0.9,0.87);
  leg->SetFillStyle(0);
  leg->SetBorderSize(0);
  for(int h=0; h<((etabin==-1 || etabin<24)?3:1); ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  
	  h1_slope[h][i]->GetXaxis()->SetTitle("Tower Slope [GeV^{-1}]");
	  h1_slope[h][i]->GetYaxis()->SetTitle("Counts");
	  h1_slope[h][i]->GetYaxis()->SetTitleOffset(1.5);
	  h1_slope[h][i]->SetMarkerStyle(marker[i]);
	  h1_slope[h][i]->SetMarkerColor(colors[i]);
	  if(etabin>=0)
	    {
	  h1_etaslice_slopes[h][i]->GetXaxis()->SetTitle("Tower Slope [GeV^{-1}]");
	  h1_etaslice_slopes[h][i]->GetYaxis()->SetTitle("Counts");
	  h1_etaslice_slopes[h][i]->GetYaxis()->SetTitleOffset(1.5);
	  h1_etaslice_slopes[h][i]->SetMarkerStyle(marker[i]);
	  h1_etaslice_slopes[h][i]->SetMarkerColor(colors[i]);
	    }
	  h1_E[h][i]->GetXaxis()->SetTitle("Tower E [GeV]");
	  h1_E[h][i]->GetYaxis()->SetTitle("Counts Per Event");
	  h1_E[h][i]->GetYaxis()->SetTitleOffset(1.5);
	  h1_E[h][i]->SetMarkerStyle(marker[i]);
	  h1_E[h][i]->SetMarkerColor(colors[i]);
	  if(h1_E[h][i]->GetMaximum() > max[0]) max[0] = h1_E[h][i]->GetMaximum();
	  if(h1_E[h][i]->GetBinContent(h1_E[h][i]->FindLastBinAbove()) < min[0]) min[0] = h1_E[h][i]->GetBinContent(h1_E[h][i]->FindLastBinAbove());
	  if(h==0) leg->AddEntry(h1_E[h][i],texts[i].c_str(),"p");
	  h1_time[h][i]->GetXaxis()->SetTitle("Tower Time [sample]");
	  h1_time[h][i]->GetYaxis()->SetTitle("Counts Per Event");
	  h1_time[h][i]->SetMarkerStyle(marker[i]);
	  h1_time[h][i]->SetMarkerColor(colors[i]);
	  h1_time[h][i]->GetYaxis()->SetTitleOffset(1.5);
	  if(h1_time[h][i]->GetMaximum() > max[1]) max[1] = h1_time[h][i]->GetMaximum();
	  if(h1_time[h][i]->GetBinContent(h1_time[h][i]->FindLastBinAbove()) < min[1]) min[1] = h1_time[h][i]->GetBinContent(h1_time[h][i]->FindLastBinAbove());
	  h1_ntg[h][i]->GetXaxis()->SetTitle("Time [2024 date]");
	  h1_ntg[h][i]->GetYaxis()->SetTitle("Towers > 0.5 GeV Per Event");
	  h1_ntg[h][i]->SetMarkerStyle(marker[i]);
	  h1_ntg[h][i]->SetMarkerColor(colors[i]);
	  h1_ntg[h][i]->GetXaxis()->SetTimeDisplay(1);
	  h1_ntg[h][i]->GetXaxis()->SetTimeFormat("%d\/%m");
	  h1_ntg[h][i]->GetYaxis()->SetTitleOffset(1.5);
	  if(h1_ntg[h][i]->GetMaximum() > max[2]) max[2] = h1_ntg[h][i]->GetMaximum();
	  if(h1_ntg[h][i]->GetBinContent(h1_ntg[h][i]->FindLastBinAbove()) < min[2]) min[2] = h1_ntg[h][i]->GetBinContent(h1_ntg[h][i]->FindLastBinAbove());
	}
      
      for(int i=0; i<4; ++i)
	{
	  h1_E[h][i]->GetYaxis()->SetRangeUser(0.5*min[0],2*max[0]);
	  h1_E[h][i]->GetXaxis()->SetRangeUser(-1,15);
	  h1_time[h][i]->GetXaxis()->SetRangeUser(-8,8);
	  h1_time[h][i]->GetYaxis()->SetRangeUser(0.5*min[1],2*max[1]);
	  h1_ntg[h][i]->GetYaxis()->SetRangeUser(0.9*min[2],1.1*max[2]);
	}

      TCanvas* c = new TCanvas("","",1000,1000);
      gPad->SetLeftMargin(0.15);
      h1_ntg[h][0]->GetYaxis()->SetRangeUser(0,1);//0.9*h1_ntg[0]->GetBinContent(h1_ntg[0]->FindLastBinAbove(0)),1.1*h1_ntg[0]->GetMaximum());
      h1_ntg[h][0]->Draw("PE");
      c->SaveAs(("output/rmg/ntg"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+"_"+to_string(phibin)+"_0.pdf").c_str());
      h1_ntg[h][0]->GetYaxis()->SetRangeUser(0.9*min[2],1.1*max[2]);
      for(int i=1; i<4; ++i)
	{
	  h1_ntg[h][i]->Draw("SAME PE");
	}
      leg->Draw();
      c->SaveAs(("output/rmg/ntg"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+"_"+to_string(phibin)+".pdf").c_str());
      gPad->SetLogy();

      h1_slope[h][0]->Draw("PE");
      for(int i=1; i<4; ++i)
	{
	  h1_slope[h][i]->Draw("SAME PE");
	}
      leg->Draw();
      c->SaveAs(("output/rmg/slope"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+"_"+to_string(phibin)+".pdf").c_str());
      
      if(etabin>=0)
	{
      h1_etaslice_slopes[h][0]->Draw("PE");
      for(int i=1; i<4; ++i)
	{
	  h1_etaslice_slopes[h][i]->Draw("SAME PE");
	}
      leg->Draw();
      c->SaveAs(("output/rmg/etaslice_slopes"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+"_"+to_string(phibin)+".pdf").c_str());
	}

      h1_E[h][0]->Draw("PE");
      for(int i=1; i<4; ++i)
	{
	  h1_E[h][i]->Draw("SAME PE");
	}
      leg->Draw();
      c->SaveAs(("output/rmg/E"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+"_"+to_string(phibin)+".pdf").c_str());
      
      h1_time[h][0]->Draw("PE");
      for(int i=1; i<4; ++i)
	{
	  h1_time[h][i]->Draw("SAME PE");
	}
      leg->Draw();
      c->SaveAs(("output/rmg/time"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+"_"+to_string(phibin)+".pdf").c_str());
    }

  return 0;
}
