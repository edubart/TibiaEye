#ifndef CONSTTIBIA_H
#define CONSTTIBIA_H

#define NETWORKMESSAGE_MAXSIZE 16384

//TODO: change how store memory addresses
namespace ConstTibia {
	static const uint8 VersionsNum = 1;
	static const uint16 AvailableVersionsIds[VersionsNum] = {840};
	static const char AvailableVersions[VersionsNum][13] = {"Version 8.40"};

	static const char * const OTServPrivateRSA_P = "1429962396241639952007017738289889555079540334546615321"
												   "7470516082934737582776038882967213386204600674145392845"
												   "853859217990626450972452084065728686565928113";
	static const char * const OTServPrivateRSA_Q = "7630979195970404721891201847792002125535401292779123937"
												   "2074475745966927885136471792353355293072513505707284073"
												   "73705564708871762033017096809910315212884101";
	static const char * const OTServPrivateRSA_D = "4673033022358411862216018001503683214873298680851934467"
												   "5210555262940258739805766860224610646919605860206328024"
												   "3267033616301098884178392419595075722472848070352355696"
												   "1917379229278690784579190495510360165282251912190836718"
												   "7885509270025388641700821735345222087940578381210879116"
												   "823013776808975766851829020659073";

	static const char * const OTServPublicRSA = "10912013296739942927886096050899554152823750290279812912346"
												"875793726629149257644633073969600111060390723088861007265581"
												"882535850342905759282762943641310856602909362821263595383668"
												"656267584972062078627943109021801768106152175505671082387647"
												"644426055814717970711967428398241915211810375907603061668397"
												"8566631413";

	static const char * const CipPublicRSA = "12471045942682794300437644989798558216780170796069703716404"
											 "490486294856938085042139690459768695387702239460423942818549"
											 "828416906858180227761208102796672433631944853781144171907648"
											 "434092285492927351730866137072710538289911899940380804584644"
											 "464728449912316487903510362700466852100532836741525993991528"
											 "4902061793";

#ifdef WIN32 // Windows Addresses
	static const uint32 VersionAddress[VersionsNum] = {0x6376FD};
	static const uint32 ConnectionAddress[VersionsNum] = {0x788370}; // 0 = Offline, 8 = Online

	static const uint32 HostAddress[VersionsNum][4] = {{0x77FC48,0x77FCAC,112,10}}; // ip, port, size, quantity
	static const uint32 RsaAddress[VersionsNum] = {0x5AB610};
#else // Linux Addresses
	static const uint32 VersionAddress[VersionsNum] = {0x855f0b3};
	static const uint32 ConnectionAddress[VersionsNum] = {0x0};

	static const uint32 HostAddress[VersionsNum][4] = {{0x86b8c00,0x86b8c64,112,10}};
	static const uint32 RsaAddress[VersionsNum] = {0x84C1040};
#endif
}

enum SpeakClasses {
	SPEAK_SAY				= 0x01,	//normal talk
	SPEAK_WHISPER			= 0x02,	//whispering - #w text
	SPEAK_YELL				= 0x03,	//yelling - #y text
	SPEAK_PRIVATE_PN		= 0x04, //Player-to-NPC speaking(NPCs channel)
	SPEAK_PRIVATE_NP		= 0x05, //NPC-to-Player speaking
	SPEAK_PRIVATE			= 0x06, //Players speaking privately to players
	SPEAK_CHANNEL_Y     	= 0x07,	//Yellow message in chat
	SPEAK_CHANNEL_W         = 0x08, //White message in chat
	SPEAK_RVR_CHANNEL		= 0x09, //Reporting rule violation - Ctrl+R
	SPEAK_RVR_ANSWER		= 0x0A, //Answering report
	SPEAK_RVR_CONTINUE		= 0x0B, //Answering the answer of the report
	SPEAK_BROADCAST     	= 0x0C,	//Broadcast a message - #b
	SPEAK_CHANNEL_R1    	= 0x0D,	//Talk red on chat - #c
	SPEAK_PRIVATE_RED   	= 0x0E,	//Red private - @name@ text
	SPEAK_CHANNEL_O     	= 0x0F,	//Talk orange on text
	//SPEAK_                = 0x10, //?
	SPEAK_CHANNEL_R2    	= 0x11,	//Talk red anonymously on chat - #d
	//SPEAK_                = 0x12, //?
	SPEAK_MONSTER_SAY   	= 0x13,	//Talk orange
	SPEAK_MONSTER_YELL  	= 0x14,	//Yell orange
};


#endif // CONSTTIBIA_H
