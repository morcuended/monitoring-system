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
#include <sash/TelescopeRunHeader.hh>
#include <sash/Telescope.hh>
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
#include <dbtools/simpletable_frame.hh>
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

// to launch directly from a terminal: root -l -b ExtractFF_v2.C+'("run_128027_FlatFieldCoeff_002.root")'
//void ExtractFF_v2(std::string file="./run_128027_FlatFieldCoeff_002.root")
void ExtractFFToDb_fixMe(std::string file)
{
  
	bool display = true; 

	SashFile::HandlerC *han = new SashFile::HandlerC("CT1_FlatFieldCoeff"); 

	//if (!han->ConnectFileByName("./run_128027_FlatFieldCoeff_002.root"))
	if (!han->ConnectFileByName(file))
	{	
		std::cout << "Cannot connect to the file!!!!" << std::endl;
	}

	Sash::DataSet *ds_run = han->GetDataSet("run"); 
	ds_run->GetEntry(0); //start loading the useful informations
	Sash::HESSArray *fHess = &Sash::HESSArray::GetHESSArray();

	//Setting the database connection information
	//
	simpletable::MySQLconnection *fConnection =  new simpletable::MySQLconnection("141.34.130.13", "morcuende", "", "HESS_HESS1U");
	if (!fConnection->is_connected())
			exit(1);

	simpletable::SimpleTable table("Test_Camera1U_FFCoeff",0, fConnection); //Connection to Test_Camera1U_FFCoeff table in the DB stored in fConnection

	//Getting the run number and run time
	const Sash::EventHeader *eventheader = fHess->Get<Sash::EventHeader>();
	Sash::Time CurRunStartTime = Sash::Time(fHess->Get<Sash::RunHeader>()->GetRunTime());
	Int_t runNumber = fHess->Get<Sash::RunHeader>()->GetRunNum();
	std::cout << "RunNumber: " << runNumber << " - RunTime" ;
	CurRunStartTime.Print();

	//Create Canvas and histograms that will be used in the next loop
  	TCanvas *myc = new TCanvas("myc","HiG_LoG",1000,500);
  	TCanvas *chi2Can = new TCanvas("chi2","Chi2",1000,500);
  	myc->Divide(2,1);
  	TH1D *hchi2Hi = new TH1D("hchi2Hi","chi2_pix_coeff",600,0,30);   
  	TH1D *hchi2Lo = new TH1D("hchi2Lo","chi2_pix_coeff",600,0,30);
 
  	//Defining variables
  	Double_t meanHi;	 
  	Double_t rmsHi;
  	Double_t coefHi;
  	Double_t coefRMSHi;
  	Double_t chi2Hi;
  	Double_t chi2dofHi; 

  	Double_t meanLo;	 
  	Double_t rmsLo;
  	Double_t coefLo;
  	Double_t coefRMSLo;
  	Double_t chi2Lo; 
  	Double_t chi2dofLo;

        //Creating an iterator object to iterate on the telescopes found in the runheader
	const Sash::PointerSet<Sash::Telescope> &tellist = fHess->Get<Sash::RunHeader>()->GetTelsInRun();
	for (Sash::PointerSet<Sash::Telescope>::iterator tel = tellist.begin(), tel_end = tellist.end();
		tel!=tel_end; ++tel) 
		{ 
		Int_t telId = (*tel)->GetId(); //Get the telescope number
		std::cout << "Checking CT" << telId << std::endl;
		if(telId==5)
		{
				std::cout << "I don't analyse CT5... Sorry..." << std::endl;
				fConnection->close();
				exit(1);
		}

		//Loading the information for the selected telescope

		Sash::DataSet *ds = han->GetDataSet(Form("CT%i_FlatFieldCoeff",telId)); 
		ds->GetEntry(0);

		Sash::Telescope *CT = fHess->GetTel(telId);
		const HDCalibration::TelescopeFFInfos *CT_ff = CT->Get< HDCalibration::TelescopeFFInfos > ("FFInfos");

		Utilities::SimpleStats statsHi = CT_ff->GetHiCameraMeanChargeStats();
		Utilities::SimpleStats statsLo = CT_ff->GetHiCameraMeanChargeStats();

		//Checking if the data set contains data
		if (statsHi.GetEntries()==0 && statsLo.GetEntries()==0)
		{
				std::cout << "The dataset for this telescope is empty!!!" << std::endl;
				continue;
		}

		// Getting the runnumber and the time at which the run started

		const Sash::TelescopeRunHeader *CT_runHead = CT->Get< Sash::TelescopeRunHeader > ("");
		UInt_t runNumber = CT_runHead->GetRunNumber();
		std::cout << "Run: " << runNumber << std::endl;
		Sash::Time gpsTime = CT_runHead->GetGPSTime();
		gpsTime.Print();

		CrashData::UTC utcT = gpsTime.GetUTC();
		std::cout <<utcT << std::endl;
		std::ostringstream date;

		std::cout<< utcT.GetString() << std::endl;

		//Putting condition when reading the table. If a run is already in the DB for a given telescope we skip it
		ostringstream setcond;
		setcond << "telescopeId=" << telId;
		ostringstream runcond;
		runcond << "RunNumber=" << runNumber;
		table.store_result(setcond.str(),simpletable::SCV(),simpletable::SCV(),"",0,0,false,0,"","",runcond.str());

		//Checking if some data are contained in the DB
		if (table.rows()!=0)
		{
				std::cout << "The run is already found in the database!!! Skip it!" << std::endl;
				continue;
		}

		simpletable::SimpleData data(960,13);	//Table where will be stored the parameters for each pixel

		CT_ff->DisplayFFCharge(); //needed to load the values from the files (Dunno why...)
		std::cout << "Filling the coefficients to the database" << std::endl;

		char *sep = "\t";

		// Loop over the pixels	
		  for (Sash::Pointer<Sash::Pixel> pix = CT->beginPixel(); pix != CT->endPixel(); ++pix) 
		    { //looping over all the pixel to extract the values
		    Int_t PixId = (*pix).GetConfig()->GetPixelID(); //PixId is an int that represents the pixel number
		    char *hand = "hand";
		    char *ct = "CT";
		    const HDCalibration::PixelFFInfos *pixff = CT_ff->GetMonitor(pix);
		   	
		    // Calculating the coefficients for the high gain    
		    TH1 *ffHistHi = pixff->GetHiFFPerEventDistribution(); // these are the histograms	
		    // Calculating the coefficients for the low gain
		    TH1 *ffHistLo = pixff->GetLoFFPerEventDistribution();
		    // create a vector to store quantiles values
		    Double_t q[5];
		    Double_t probs[5]={0.025,0.16,0.5,1-0.16,0.975};
		    Int_t HiEntries = ffHistHi -> GetEntries();
		    Int_t LoEntries = ffHistLo -> GetEntries();  
		    	if (HiEntries < 300 ) // if not cannot be fitted
			{
			meanHi = -1.;	 
		    	rmsHi  = -1.;
			coefHi = .0;
			coefRMSHi = .0;
		    	chi2Hi = .0; 
		        cout << "Pixel  " << PixId << "  has  "<< HiEntries <<" (Hi) and "<< LoEntries << " (Lo) entries" << endl;
		        }
			else if (LoEntries < 300)
			{
			meanLo = -1.;	 
		    	rmsLo  = -1.;
			coefLo = .0;
			coefRMSLo = .0;
		    	chi2Lo = .0; 
			cout << "Pixel " << PixId << "  has  "<< HiEntries <<" (Hi) and "<< LoEntries << " (Lo) entries" << endl;
			//fprintf(fp,"%i,%i,%i %s\n",PixId,HiEntries,LoEntries,"**No entries**");
			}
		        else
			{
			// Get the charge and coefficient of high gain channel
			myc->cd(1);
		    	ffHistHi -> Draw();
		    	ffHistHi -> Fit("gaus","Q","");	
			TF1 *fgausHi = ffHistHi->GetFunction("gaus");    	
			meanHi = pixff -> GetHiChargeStats().GetMean();
		    	rmsHi  = pixff -> GetHiChargeStats().GetRMS();
			coefHi = ffHistHi -> GetMean();
			coefRMSHi = ffHistHi -> GetRMS();
		    	chi2Hi = fgausHi -> GetChisquare(); 
		    	Int_t dofHi     = fgausHi -> GetNDF();
		 	chi2dofHi = chi2Hi/dofHi;
		 
			// Get the charge and coefficient of low gain channel
		        myc->cd(2);
		   	ffHistLo -> Draw();
		  	ffHistLo -> Fit("gaus","Q",""); //Q option in the second argument for quiet run
			TF1 *fgausLo = ffHistLo->GetFunction("gaus");   	
			meanLo = pixff->GetLoChargeStats().GetMean(); 
		    	rmsLo  = pixff->GetLoChargeStats().GetRMS();
			coefLo = ffHistLo -> GetMean();
			coefRMSLo = ffHistLo -> GetRMS();
		   	chi2Lo = fgausLo -> GetChisquare();
		    	Int_t dofLo     = fgausLo -> GetNDF();
			chi2dofLo = chi2Lo/dofLo;
			//
			cout <<"Pixel "<<PixId <<", chi2: " << chi2Hi/dofHi <<" and "<< chi2Lo/dofLo <<endl;
		    	
		    	chi2Can->cd();
			hchi2Hi->Fill(chi2dofHi);
			hchi2Lo->Fill(chi2dofLo);
			
			// Setting limits on the chi2-value (this depends on the telescope and gain channel)
		    	if (chi2Hi/dofHi>1.75 || chi2Hi/dofHi<0.55 || chi2Lo/dofLo>1.75 || chi2Lo/dofLo<0.55) // at 95% CI for CT1
				{
		     		cout <<"Pixel "<<PixId <<", chi2: " << chi2Hi/dofHi <<" and "<< chi2Lo/dofLo <<endl;
				//	cin.ignore();
				//	std::ostringstream fn;
				//	fn <<"plotId" << PixId <<".pdf";
				//	myc->SaveAs(fn.str().c_str());	
				}
			}
			// Filling the array which contains all data that will be stored into the database;
			data[PixId][0] = runNumber;
			data[PixId][1] = utcT.GetString(); 
			data[PixId][2] = PixId;
			data[PixId][3] = meanHi;
			data[PixId][4] = rmsHi;
			data[PixId][5] = meanLo;
			data[PixId][6] = rmsLo;
			data[PixId][7] = coefHi;
			data[PixId][8] = coefRMSHi;
			data[PixId][9] = coefLo;
			data[PixId][10] = coefRMSLo;
			data[PixId][11] = chi2dofHi;
			data[PixId][12] = chi2dofLo;
	 }

///////////////////////////////////////////////////////////////////////////////////////////////////
		table.read_and_put(data, simpletable::SCV(1,(int)(telId))); // Filling myTable and myTable_Set of the database  

		  //ds->~DataSet(); 
		  //CT->~Telescope();
		}
	fConnection->close();
}
