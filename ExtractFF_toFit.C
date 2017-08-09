// STL
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <utility>

// ROOT
#include <TFile.h>
#include <TCanvas.h>
#include <TF1.h>
//#include <TStyle.h> //DM (I add this)
//#include <TCanvas.h> //DM

// HESS
#include <sashfile/HandlerC.hh>
#include <sash/DataSet.hh>
#include <sash/MakerChain.hh>
#include <sash/Maker.hh>
#include <sash/TreeWriter.hh>
#include <summary/HVSummaryMaker2.hh>
#include <sash/Folder.hh>
#include <utilities/TNamedSaver.hh>
#include <sash/HESSArray.hh>
#include <sash/Time.hh>
#include <utilities/TextStyle.hh>
#include <sash/Chain.hh>
#include <sash/RunHeader.hh>
#include <sash/EventHeader.hh>
#include <sash/TelescopeEventHeader.hh>
#include <sash/TelescopeConfig.hh>
#include <sash/TelescopeMonitorBase.C>
#include <monitor/TelescopeHVTempMonitor.hh>
#include <dbutils/HeaderFixer.hh>
#include <dbutils/Handler.hh>

#include <sash/HESSArray.hh>
#include <sash/Pointer.hh>
#include <sash/PointerSet.hh>
#include <sash/PointerSetIterator.hh>
#include <sash/Return.hh>
#include <sash/EventHeader.hh>
#include <sash/FunMaker.hh>
#include <sash/Pixel.hh>
#include <sash/PixelConfig.hh>

#include <hdcalibration/TelescopeFFInfos.hh>
#include <hdcalibration/PixelFFInfos.hh>
#ifndef __CINT__
// We hide it from cint to avoid problems !
#include <summary/HeaderChecker.hh>
#endif

#include <summary/StatsMaker.hh>

#include <calibration/DataFixer.hh>

#include <calibrationmakers/BrokenHVMaker.hh>
#include <calibrationmakers/Utilities.hh>

#include <calibrationmakers/ConvertSampleToChargeMaker.hh>

#include <calibrationmakers/PedestalMaker2.hh>
#include <calibrationmakers/HiLoRatioMaker2.hh>
#include <calibrationmakers/ADCtoPeMaker2.hh>

#include <summary/PedestalSummaryMaker.hh>

void ExtractFF(){
  
  bool display = true; 

  SashFile::HandlerC *han = new SashFile::HandlerC("CT1_FlatFieldCoeff"); 

  if (!han->ConnectFileByName("./run_128027_FlatFieldCoeff_002.root"))
	{	
		std::cout << "Cannot connect to the file!!!!" << std::endl;
	}

	
  Sash::DataSet *ds = han->GetDataSet("CT1_FlatFieldCoeff"); 
  Sash::DataSet *ds_run = han->GetDataSet("run"); 

  ds_run->GetEntry(0); //start loading the useful informations
  ds->GetEntry(0);


  Sash::HESSArray *fHess = &Sash::HESSArray::GetHESSArray();
  Sash::Telescope *CT1 = fHess->GetTel(1);

  const HDCalibration::TelescopeFFInfos *CT1_ff = CT1->Get< HDCalibration::TelescopeFFInfos > ("FFInfos");
 
  CT1_ff->DisplayFFCharge();
//////////////////////////////////////////////////////////////////////////////////////////////
//
//	Fix me if you can!!!!
//
//
//



  char *sep = "\t";
  std::cout << "telescope/I:PixId/I:HiCoeff/F:HiCoeffRMS/F:LoCoeff/F:LoCoeffRMS/F:HiCharge/F:HiChargeRMS/F:LoCharge/F:LoChargeRMS" << std::endl;
  
  TCanvas *myc = new TCanvas("myc","HiG_LoG",1000,500);
  TCanvas *chi2Can = new TCanvas("chi2","Chi2",1000,500);
  //gStyle->SetOptFit(); //should show the fit parameters in the Canvas (but does not work!!)
 
  FILE *fp = fopen("FFcoeff.dat","w");
  fprintf(fp,"%s\n","# HiIllumination | HiIlluminationRMS | LoIllumination | LoIlluminationRMS HiCoeff | HiCoeffRMS | LoCoeff | LoCoeffRMS"); 
  myc->Divide(2,1);
  TH1D *hchi2Hi = new TH1D("hchi2Hi","chi2_pix_coeff",600,0,30);   
  TH1D *hchi2Lo = new TH1D("hchi2Lo","chi2_pix_coeff",600,0,30);

  for (Sash::Pointer<Sash::Pixel> pix = CT1->beginPixel(); pix != CT1->endPixel(); ++pix) 
    { //looping over all the pixel to extract the values
    Int_t PixId = (*pix).GetConfig()->GetPixelID(); //PixId is an int that represents the pixel number
    char *hand = "hand";
    char *ct = "CT";
    const HDCalibration::PixelFFInfos *pixff = CT1_ff->GetMonitor(pix);
   	
    // Calculating the coefficients for the high gain    
    TH1 *ffHistHi = pixff->GetHiFFPerEventDistribution(); // these are the histograms	
    // Calculating the coefficients for the low gain
    TH1 *ffHistLo = pixff->GetLoFFPerEventDistribution();
    // create a vector to store quantiles values
    Double_t q[5];
    Double_t probs[5]={0.025,0.16,0.5,1-0.16,0.975};
    Int_t HiEntries = ffHistHi -> GetEntries();
    Int_t LoEntries = ffHistLo -> GetEntries();  
    	if (ffHistHi -> GetEntries()==0 || ffHistLo -> GetEntries()==0)
	{
        cout << "Pixel  " << PixId << "  has  "<< HiEntries <<" (Hi) and "<< LoEntries << " (Lo) entries" << endl;
        //fprintf(fp,"%i,%i,%i %s\n",PixId,HiEntries,LoEntries,"**No entries**");
	}	
        else
	{
	// Get the charge and coefficient of high gain channel
	myc->cd(1);
    	ffHistHi -> Draw();
    	ffHistHi -> Fit("gaus","Q","");	
	TF1 *fgausHi = ffHistHi->GetFunction("gaus");    	
	Double_t meanHi = pixff -> GetHiChargeStats().GetMean();	 //Returns a double
    	Double_t rmsHi  = pixff -> GetHiChargeStats().GetRMS();
	Double_t coefHi = ffHistHi -> GetMean();
	Double_t coefRMSHi = ffHistHi -> GetRMS();
    	Double_t chi2Hi = fgausHi -> GetChisquare();         //Returns chi2 and dof
    	Int_t dofHi     = fgausHi -> GetNDF();
 	Double_t chi2dofHi = chi2Hi/dofHi; 
	// Get the charge and coefficient of low gain channel
        myc->cd(2);
   	ffHistLo -> Draw();
  	ffHistLo -> Fit("gaus","Q",""); //Q option in the second argument for quiet run
	TF1 *fgausLo = ffHistLo->GetFunction("gaus");   	
	Double_t meanLo = pixff->GetLoChargeStats().GetMean();         //Returns a double
    	Double_t rmsLo  = pixff->GetLoChargeStats().GetRMS();
	Double_t coefLo = ffHistLo -> GetMean();
	Double_t coefRMSLo = ffHistLo -> GetRMS();
   	Double_t chi2Lo = fgausLo -> GetChisquare();         //Returns chi2 and dof	
    	Int_t dofLo     = fgausLo -> GetNDF();
	Double_t chi2dofLo = chi2Lo/dofLo;
	fprintf(fp,"%i,%f,%f,%f,%f,%f,%f,%f,%f\n",PixId,meanHi,rmsHi,meanLo,rmsLo,coefHi,coefRMSHi,coefLo,coefRMSLo);
//
	cout <<"Pixel "<<PixId <<", chi2: " << chi2Hi/dofHi <<" and "<< chi2Lo/dofLo <<","<< meanHi<<","<< rmsHi <<endl;
    	
    	chi2Can->cd();
	hchi2Hi->Fill(chi2dofHi);
	hchi2Lo->Fill(chi2dofLo);

//        hchi2->Draw();

//	chi2Can->Update();

//	myc->Update();
	
    	if (chi2Hi/dofHi>1.75 || chi2Hi/dofHi<0.55 || chi2Lo/dofLo>1.75 || chi2Lo/dofLo<0.55) // at 95% CI
		{
     		cout <<"Pixel "<<PixId <<", chi2: " << chi2Hi/dofHi <<" and "<< chi2Lo/dofLo <<endl;
//		cin.ignore();
	//	std::ostringstream fn;
	//	fn <<"plotId" << PixId <<".pdf";
	//	myc->SaveAs(fn.str().c_str());	
		}
	}
    }
  //  hchi2   -> GetQuantiles(5,q,probs);
   // cout<<q<<endl;
    hchi2Hi -> Draw();
    hchi2Lo -> SetLineColor(kRed);
    hchi2Lo -> Draw("same");
    chi2Can -> BuildLegend();
    chi2Can -> Update();
//  	vector<double> quantiles( TH1F* h ){
  	double qHi[5],qLo[5];
  	double probs[5] = {0.025, 0.16, 0.5, 1 - 0.16, 0.975 };
 	hchi2Hi->GetQuantiles( 5, qLo, probs ); 
        hchi2Lo->GetQuantiles( 5, qHi, probs );  
	 vector<double> r(5);
   	 for (int i=0; i<5; i++)
    		{ 
    		cout << " quantile " << i << " " << qLo[i] << endl;
        	r[i] = qLo[i];
        	}
  
  fclose(fp);

}


//void h12ascii (TH1* h)
//{
//   Int_t n = h->GetNbinsX();
//   for (Int_t i=1; i<=n; i++) {
//      printf("%g %g\n",
//             h->GetBinLowEdge(i)+h->GetBinWidth(i)/2,
//             h->GetBinContent(i));
//   }
//}

// Use it this way: ro)ot [0] .x h12ascii.C(hpx); > h2ascii.dat


