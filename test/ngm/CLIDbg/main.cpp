#include <iostream>
#include <unordered_map>
#include "CLIManager.h"
#include "SampleDevice.h"
#include "HardwareManager.h"

int main(int argc, char **argv)
{
	try {
		HardwareManager *hwMgr = HardwareManager::Instance();

		DevicePtr device = SampleDevice::create(/*"msp430"*/"dev", "msp430Config.ini", "msp430Params.xml");
		hwMgr->AddDevice(device);

		CLIManager* CLIMgr = CLIManager::Instance(hwMgr);

		CLIMgr->Init();

		while (CLIMgr->isUP()) {
			CLIMgr->ReceiveCommand();
			try {
				CLIMgr->Execute();
			}
			catch (std::exception& ex) {
				log::info() << "Exception: " << ex.what() << std::endl;
			}
		}
	}
	catch (std::exception &ex) {
		log::info() << "Exception: " << ex.what() << std::endl;
	}

	return 0;
}

	////try {
	////	HardwareManager* hwMgr = HardwareManager::Instance();

	////	char c = 'c';
	////	short sh = -5;
	////	int i = -250;
	////	long l = -100000;

	////	unsigned char uc = 'u';		// byte
	////	unsigned short ush = 5;			// word
	////	unsigned int ui = 250;			// dword
	////	unsigned long ul = 100000;		// qword

	////	float f = 5.5;
	////	double d = 34.555555;
	////	
	////	bool b = true;
	////	std::string s = "Xruk!";
	////	
	////	AnyValue AVc(c);
	////	ParameterPtr Pc(new Parameter("Pc", SectionType::Parameter, AVc, AVc));

	////	AnyValue AVsh(sh);
	////	ParameterPtr Psh(new Parameter("Psh", SectionType::Parameter, AVsh, AVsh));

	////	AnyValue AVi(i);
	////	ParameterPtr Pi(new Parameter("Pi", SectionType::Parameter, AVi, AVi));

	////	AnyValue AVl(l);
	////	ParameterPtr Pl(new Parameter("Pl", SectionType::Parameter, AVl, AVl));

	////	AnyValue AVuc(uc);
	////	ParameterPtr Puc(new Parameter("Puc", SectionType::Parameter, AVuc, AVuc));

	////	AnyValue AVush(ush);
	////	ParameterPtr Push(new Parameter("Push", SectionType::Parameter, AVush, AVush));

	////	AnyValue AVui(ui);
	////	ParameterPtr Pui(new Parameter("Pui", SectionType::Parameter, AVui, AVui));

	////	AnyValue AVul(ul);
	////	ParameterPtr Pul(new Parameter("Pul", SectionType::Parameter, AVul, AVul));
	////	
	////	AnyValue AVf(f);
	////	ParameterPtr Pf(new Parameter("Pf", SectionType::Parameter, AVf, AVf));

	////	AnyValue AVd(d);
	////	ParameterPtr Pd(new Parameter("Pd", SectionType::Parameter, AVd, AVd));

	////	AnyValue AVb(b);
	////	ParameterPtr Pb(new Parameter("Pb", SectionType::Parameter, AVb, AVb));

	////	AnyValue AVs(s);
	////	ParameterPtr Ps(new Parameter("Ps", SectionType::Parameter, AVs, AVs));

	////	std::unordered_map<std::string, ParameterPtr> params;
	////	params.insert(std::pair<std::string, ParameterPtr>("Pc", Pc));
	////	params.insert(std::pair<std::string, ParameterPtr>("Psh", Psh));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pi", Pi));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pl", Pl));
	////	params.insert(std::pair<std::string, ParameterPtr>("Puc", Puc));
	////	params.insert(std::pair<std::string, ParameterPtr>("Push", Push));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pui", Pui));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pul", Pul));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pf", Pf));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pd", Pd));
	////	params.insert(std::pair<std::string, ParameterPtr>("Pb", Pb));
	////	params.insert(std::pair<std::string, ParameterPtr>("Ps", Ps));

	////	DevicePtr device(new Device("dev", params));
	////	hwMgr->AddDevice(device);

	////	CLIManager* CLIMgr = CLIManager::Instance(hwMgr);

	////	CLIMgr->Init();

	////	while (CLIMgr->isUP()) {
	////		CLIMgr->ReceiveCommand();
	////		try {
	////			CLIMgr->Execute();
	////		}
	////		catch (std::exception& ex) {
	////			log::info() << "Exception: " << ex.what() << std::endl;
	////		}
	////	}

	////}
	////catch (std::exception& ex) {
	////	log::info() << "Exception: " << ex.what() << std::endl;
	////}