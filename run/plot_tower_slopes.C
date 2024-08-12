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
  TH1D* h1_slope_uncert[3][4];
  TH1D* h1_etaslice_slopes[3][4][96];
  TH1D* h1_slopes_etaslice[3][4];
  TH2D* h2_slopes_eta[3][4];
  TH2D* h2_uncert_eta[3][4];
  TH2D* h2_slope_uncert[3][4];
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
  string cal[3] = {"EMCal","IHCal","OHCal"};
  TCanvas* d = new TCanvas("","",1000,1000);
  d->cd();
  gPad->SetLogy();
  gPad->SetLeftMargin(0.15);
  float lorange[3] = {-2,-5,-2};
  float range[3] = {-10,-20,-5};
  float urange[3] = {1.25,2,0.25};
  for(int h=0; h<3; ++h)
    {
      for(int i=0; i<4; ++i)
	{
	  h2_slopes_eta[h][i] = new TH2D(("h2_slopes_eta"+to_string(h)+"_"+to_string(i)).c_str(),"",(h==0?96:24),-0.5,(h==0?95.5:23.5),100,range[h],lorange[h]);
	  h2_uncert_eta[h][i] = new TH2D(("h2_uncert_eta"+to_string(h)+"_"+to_string(i)).c_str(),"",(h==0?96:24),-0.5,(h==0?95.5:23.5),100,0,urange[h]);
	  h2_slope_uncert[h][i] = new TH2D(("h2_slope_uncert"+to_string(h)+"_"+to_string(i)).c_str(),"",100,range[h],lorange[h],100,0,urange[h]);
	  h1_slope[h][i] = new TH1D(("h1_slope"+to_string(h)+"_"+to_string(i)).c_str(),"",100,range[h],lorange[h]);
	  h1_slope_uncert[h][i] = new TH1D(("h1_slope_uncert"+to_string(h)+"_"+to_string(i)).c_str(),"",100,0,urange[h]);
	  h1_slopes_etaslice[h][i] = new TH1D(("h1_slopes_etaslice"+to_string(h)+"_"+to_string(i)).c_str(),"",100,range[h],lorange[h]);
	  for(int j=0; j<(h==0?96:24); ++j)
	    {
	      h1_etaslice_E[h][i][j] = (TH1D*)thefile->Get(("h1_etaslice_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str());
	      //h1_etaslice_E[h][i][j]->Scale(1./totalevt[i]);
	      h1_etaslice_E[h][i][j]->Fit("expo","L I S Q","",0.3,2);
	      h1_etaslice_slopes[h][i][j] = new TH1D(("h1_etaslice_slopes"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)).c_str(),"",100,range[h],lorange[h]);
	      if(h1_etaslice_E[h][i][j]->GetFunction("expo")->GetParameter(1)) h1_slopes_etaslice[h][i]->Fill(h1_etaslice_E[h][i][j]->GetFunction("expo")->GetParameter(1));
	      if(j!=etabin && etabin!=-1) continue;
	      for(int k=0; k<(h==0?256:64); ++k)
		{
		  h1_E[h][i][j][k] = (TH1D*)thefile->Get(("h1_tower_E"+to_string(h)+"_"+to_string(i)+"_"+to_string(j)+"_"+to_string(k)).c_str());
		  //h1_E[h][i][j][k]->Scale(1./totalevt[i]);
		  h1_E[h][i][j][k]->Fit("expo","L I S Q","",0.3,2);
		  float slope = 0;
		  float sloperr = 0;
		  if(h1_E[h][i][j][k]->GetFunction("expo"))
		    {
		     slope = h1_E[h][i][j][k]->GetFunction("expo")->GetParameter(1);
		     sloperr = h1_E[h][i][j][k]->GetFunction("expo")->GetParError(1);
		    }
		  h2_slopes_eta[h][i]->Fill(j,slope);
		  h1_slope[h][i]->Fill(slope);
		  h2_uncert_eta[h][i]->Fill(j,sloperr);
		  h2_slope_uncert[h][i]->Fill(slope,sloperr);
		  h1_slope_uncert[h][i]->Fill(sloperr);
		  h1_etaslice_slopes[h][i][j]->Fill(slope);
		}
	    }
	}
    }
  //cout << h1_E[0]->GetBinContent(50) << " " << h1_E[0]->GetBinContent(51) <<" " << h1_E[0]->GetBinContent(52) << " " << h1_E[0]->GetBinContent(53) << endl;

  for(int h=0; h<3; ++h)
    {
      for(int j=0; j<(h==0?96:24); ++j)
	{
	  float goodslope = 0;
	  float goodamp = 0;
	  int it = -1;
	  while(goodslope < (h==0?-7:(h==1?-10:-3.2)) || goodslope > (h==0?-5.5:(h==1?-9:-3)))
	    {
	      ++it;
	      if(it>(h==0?255:63))
		{
		  it = -1;
		  break;
		}
	      if(h1_E[h][0][j][it]->GetFunction("expo"))
		{
		  goodslope = h1_E[h][0][j][it]->GetFunction("expo")->GetParameter(1); 
		  goodamp = h1_E[h][0][j][it]->GetFunction("expo")->GetParameter(0);
		}
	    }
	  if(it==-1)
	    {
	      cout << "Error in calo \ eta bin : " << h << " \ " << j << endl;
	      continue;
	    }
	  for(int k=0; k<(h==0?256:64); ++k)
	    {
	      float slope = 0;
	      float sloperr = 0;
	      if(h1_E[h][0][j][k]->GetFunction("expo"))
		{
		  slope = h1_E[h][0][j][k]->GetFunction("expo")->GetParameter(1);
		  sloperr = h1_E[h][0][j][k]->GetFunction("expo")->GetParError(1);
		}
	      if(((slope < -8 && h == 0) || ((slope < -12 || slope > -5) && h==1) || ((slope < -3.4 || slope > -2) && h==2) || sloperr>(h==0?0.7:(h==1?1.5:0.12))) && slope != 0)
		{
		  h1_E[h][0][j][k]->GetXaxis()->SetTitle((cal[h]+" Tower Energy [GeV]").c_str());
		  h1_E[h][0][j][k]->GetYaxis()->SetTitle("Per Event Counts");
		  h1_E[h][0][j][k]->GetXaxis()->SetRangeUser(0,2.5);
		  //h1_E[h][0][j][k]->GetYaxis()->SetRangeUser(0.5*h1_E[h][0][j][k]->GetBinContent(h1_E[h][0][j][k]->FindLastBinAbove()),2*h1_E[h][0][j][k]->GetBinContent(2));
		  h1_E[h][0][j][k]->GetYaxis()->SetTitleOffset(1.5);
		  h1_E[h][0][j][k]->SetMarkerStyle(20);
		  h1_E[h][0][j][it]->GetFunction("expo")->SetLineColor(kGreen);
		  h1_E[h][0][j][it]->SetMarkerColor(kGreen);
		  h1_E[h][0][j][it]->SetMarkerStyle(20);
		  TLegend* leg = new TLegend(0.45,0.6,0.9,0.75);
		  leg->SetFillStyle(0);
		  leg->SetBorderSize(0);
		  leg->AddEntry(h1_E[h][0][j][it],("Good Tower \& Fit (#eta,#phi) = ("+to_string(j)+","+to_string(it)+")").c_str(),"p");
		  leg->AddEntry(h1_E[h][0][j][k],("Outlier Tower \& Fit (#eta,#phi) = ("+to_string(j)+","+to_string(k)+")").c_str(),"p");
		  h1_E[h][0][j][k]->Draw("P E");
		  h1_E[h][0][j][it]->Draw("SAME P E");
		  
		  leg->Draw();
		  sphenixtext();
		  float amp = h1_E[h][0][j][k]->GetFunction("expo")->GetParameter(0);
		  //amp = TMath::Exp(amp);
		  drawText(("Outlier Fit = #left(e^{"+to_string(amp).substr(0,to_string(amp).find(".")+3)+"}#right)e^{"+to_string(slope).substr(0,to_string(slope).find(".")+3)+"x}").c_str(), 0.87, 0.85, 1, kBlack, 0.025);
		  drawText(("Good Fit = #left(e^{"+to_string(goodamp).substr(0,to_string(goodamp).find(".")+3)+"}#right)e^{"+to_string(goodslope).substr(0,to_string(goodslope).find(".")+3)+"x}").c_str(), 0.87, 0.8, 1, kBlack, 0.025);
		  d->SaveAs(("output/rmg/outlierslope"+to_string(h)+"_"+to_string(j)+"_"+to_string(k)+".png").c_str());
		}
	    }
	}
    }

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
  TCanvas* c = new TCanvas("","",1000,1000);
  for(int h=0; h<((etabin==-1 || etabin<24)?3:1); ++h)
    {

      for(int i=0; i<4; ++i)
	{
	  //formathist(h2_uncert_eta[h][i],,"Tower Slope Uncertainty",20,kBlack);
	  h2_slope_uncert[h][i]->GetXaxis()->SetTitle("Tower Slope [GeV^{-1}]");
	  h2_slope_uncert[h][i]->GetYaxis()->SetTitle("Tower Slope Fit Uncertainty");
	  h2_uncert_eta[h][i]->GetXaxis()->SetTitle((cal[h]+" #eta Bin").c_str());
	  h2_uncert_eta[h][i]->GetYaxis()->SetTitle("Tower Slope Fit Uncertainty");
	  formathist(h1_slope_uncert[h][i],"Tower Slope Fit Uncertainty","Counts",20,kBlack);
	  h2_slopes_eta[h][i]->GetXaxis()->SetTitle((cal[h]+" #eta Bin").c_str());
	  h2_slopes_eta[h][i]->GetYaxis()->SetTitle("Tower Slope [GeV^{-1}]");
	  h2_slopes_eta[h][i]->GetYaxis()->SetTitleOffset(1.5);
	  h2_slopes_eta[h][i]->GetZaxis()->SetTitle("Counts");
	  h2_slopes_eta[h][i]->GetZaxis()->SetTitleOffset(1.5);
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
      
      
      c->cd();
      gPad->SetLogy();

      h1_slope_uncert[h][0]->Draw("PE");
      c->SaveAs(("output/rmg/slope_uncert"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+".png").c_str());

      (etabin==-1?h1_slope[h][0]:h1_slopes_etaslice[h][0])->Draw("PE");
      /*
      for(int i=1; i<4; ++i)
	{
	  (etabin==-1?h1_slope[h][i]:h1_slopes_etaslice[h][i])->Draw("SAME PE");
	}
      leg->Draw();
      */
      c->SaveAs(("output/rmg/slope"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+".png").c_str());
      
      if(etabin>=0)
	{
	  h1_etaslice_slopes[h][0][etabin]->Draw("PE");
	  /*
	  for(int i=1; i<4; ++i)
	    {
	      h1_etaslice_slopes[h][i][etabin]->Draw("SAME PE");
	    }
	  leg->Draw();
	  */
	  c->SaveAs(("output/rmg/etaslice_slopes"+to_string(h)+"_"+to_string(rn)+"_"+to_string(etabin)+".png").c_str());
	}
    }
  gPad->SetLogy(0);
  gPad->SetLeftMargin(0.15);
  gPad->SetRightMargin(0.15);
  
  for(int h=0; h<3; ++h)
    {
      h2_slope_uncert[h][0]->GetZaxis()->SetTitle("Counts");
      h2_slope_uncert[h][0]->Draw("COLZ");
      c->SaveAs(("output/rmg/h2_slope_uncert"+to_string(h)+".png").c_str());
      h2_uncert_eta[h][0]->GetZaxis()->SetTitle("Counts");
      h2_uncert_eta[h][0]->Draw("COLZ");
      c->SaveAs(("output/rmg/h2_uncert_eta"+to_string(h)+".png").c_str());
      h2_slopes_eta[h][0]->Draw("COLZ");
      c->SaveAs(("output/rmg/h2_slopes_eta"+to_string(h)+".png").c_str());
    }

  return 0;
}
