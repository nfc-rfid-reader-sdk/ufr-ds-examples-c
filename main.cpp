#include <iostream>
#include <string>
#include <cstring>
#include <string.h>
#include <string>
#include <sstream>
#include <time.h>
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <windows.h>
#include <conio.h>

#define APP_VERSION	"1.0"
#include "ufr-lib/include/uFCoder.h"




void convert_str_to_key(std::string key_str, unsigned char *aes_key);
bool prepare_key(unsigned char *aes_key);
bool prepare_int_key(unsigned char *aes_key);
char* get_result_str(unsigned short card_status, unsigned short exec_time);
char* switch_card_status(unsigned short card_status);
void print_settings();
void PrepareSettings(void);

void GetCardUID(void);
void GetFreeMemory(void);
void FormatCard(void);
void DEStoAES(void);
void AEStoDES(void);
void GetBaudRate(void);
void SetBaudRate(void);
void SetRandomID(void);
void StoreKeyIntoReader(void);
void InternalKeysLock(void);
void InternalKeysUnlock(void);
void GetKeySettings(void);
void ChangeKeySettings(void);
void ChangeAESKey(void);
void MakeApplication(void);
void DeleteApplication(void);
void MakeFile(void);
void DeleteFile(void);
void IncreaseValueFile(void);
void DecreaseValueFile(void);
void ReadValueFile(void);
void WriteStdFile(void);
void ReadStdFile(void);
void ChangeSettings(void);

bool set_not_changeable = false, create_with_master = false, master_not_changeable = false;

std::string card_operation_status = "";
std::string settings[7];

unsigned char aes_key_ext[16];
unsigned long aid;
unsigned char aid_key_nr;
unsigned char aes_key_nr;
bool internal_key = false, master_authent_req = false;
unsigned char file_id;

//----------------------------------
void Convert(std::string str, uint8_t *array) {

    for (unsigned int i = 0; i < str.length() / 2; i++) {

        std::string part = str.substr(i * 2, 2);

            char str1[32];
            char *ptr;
            strcpy(str1, part.c_str());

        array[i] = (uint8_t)strtol(str1, &ptr, 16);

    }

}
//-----------------------------------
void PrepareSettings()
{
    std::string line;
    std::ifstream myfile("..\\..\\config.txt");
    int i = 0;

    if (myfile.is_open())
  {
    while (getline(myfile,line))
    {
        settings[i] = line;
        std::cout << "\t" << line << std::endl;
        i++;
    }
    myfile.close();

  }else{

      std::cout << std::endl << "Unable to open settings.ini file." << std::endl;
      exit(0);

  }

  int position = settings[0].find(':');
    settings[0] = settings[0].substr(position + 2); //aes_key

    position = settings[1].find(':');
    settings[1] = settings[1].substr(position + 2); //AID

    position = settings[2].find(':');
    settings[2] = settings[2].substr(position + 2); // AID key nr

    position = settings[3].find(':');
    settings[3] = settings[3].substr(position + 2); // FileID

    position = settings[4].find(':');
    settings[4] = settings[4].substr(position + 2); // Internal key nr

}
//--------------------------------------------------------------------
bool prepare_key(unsigned char *aes_key)
{
    int aes_key_length;
    std::string key_text;

    key_text = settings[0];
    aes_key_length = key_text.length();
    if (aes_key_length != 32)
    {
        std::cout << "Key length must be 16 bytes" << std::endl;
        return false;
    }

    convert_str_to_key(key_text,aes_key);

    return true;

}

//-------------------------------------------------------------------
void convert_str_to_key(std::string key_str, unsigned char *aes_key)
{
    char aes_key_part[8];
	unsigned long key_nr;
	unsigned char temp[4];
	unsigned char i;

       	char *aes_str = (char*)key_str.c_str();

	memset(aes_key_part, 0, 8);
	memcpy(aes_key_part, aes_str, 6);
	key_nr = strtol(aes_key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		aes_key[i] = temp[2 - i];

	memset(aes_key_part, 0, 8);
	memcpy(aes_key_part, &aes_str[6], 6);
	key_nr = strtol(aes_key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		aes_key[3 + i] = temp[2 - i];

	memset(aes_key_part, 0, 8);
	memcpy(aes_key_part, &aes_str[12], 6);
	key_nr = strtol(aes_key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		aes_key[6 + i] = temp[2 - i];

	memset(aes_key_part, 0, 8);
	memcpy(aes_key_part, &aes_str[18], 6);
	key_nr = strtol(aes_key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		aes_key[9 + i] = temp[2 - i];

	memcpy(aes_key_part, &aes_str[24], 6);
	key_nr = strtol(aes_key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		aes_key[12 + i] = temp[2 - i];

	memset(aes_key_part, 0, 8);
	memcpy(aes_key_part, &aes_str[30], 2);
	key_nr = strtol(aes_key_part, NULL, 16);
	aes_key[15] = key_nr;

}
//---
char* switch_card_status(unsigned short card_status)
{
	static char retstr[4096] = "";

	strcpy(retstr,"");

	switch (card_status)
	{
	case READER_ERROR:
		strcat(retstr, "READER_ERROR");
		break;

	case NO_CARD_DETECTED:
		strcat(retstr, "NO_CARD_DETECTED");
		break;

	case CARD_OPERATION_OK:
		strcat(retstr, "CARD_OPERATION_OK");
		break;

	case DESFIRE_CARD_APPLICATION_NOT_FOUND:
		strcat(retstr, "DESFIRE_CARD_APPLICATION_NOT_FOUND");
		break;

	case WRONG_KEY_TYPE:
		strcat(retstr, "WRONG_KEY_TYPE");
		break;

	case KEY_AUTH_ERROR:
		strcat(retstr, "KEY_AUTH_ERROR");
		break;

	case DESFIRE_CARD_NO_SUCH_KEY:
		strcat(retstr, "DESFIRE_CARD_NO_SUCH_KEY");
		break;

	case DESFIRE_CARD_DUPLICATE_ERROR:
		strcat(retstr, "DESFIRE_CARD_DUPLICATE_ERROR");
		break;

	case DESFIRE_CARD_PERMISSION_DENIED:
	    strcat(retstr, "DESFIRE_CARD_PERMISSION_DENIED");
		break;
	case DESFIRE_CARD_AUTHENTICATION_ERROR:
		strcat(retstr, "DESFIRE_CARD_AUTHENTICATION_ERROR");
		break;

	case DESFIRE_CARD_BOUNDARY_ERROR:
		strcat(retstr, "DESFIRE_CARD_BOUNDARY_ERROR");
		break;

	case DESFIRE_CARD_OUT_OF_EEPROM_ERROR:
		strcat(retstr, "DESFIRE_CARD_OUT_OF_EEPROM_ERROR");
		break;

	case DESFIRE_CARD_NO_CHANGES:
		strcat(retstr, "DESFIRE_CARD_NO_CHANGES");
		break;

	case DESFIRE_CARD_ILLEGAL_COMMAND_CODE:
		strcat(retstr, "DESFIRE_CARD_ILLEGAL_COMMAND_CODE");
		break;

	case DESFIRE_CARD_INTEGRITY_ERROR:
		strcat(retstr, "DESFIRE_CARD_INTEGRITY_ERROR");
		break;

	case DESFIRE_CARD_LENGTH_ERROR:
		strcat(retstr, "DESFIRE_CARD_LENGTH_ERROR");
		break;

	case DESFIRE_CARD_PARAMETER_ERROR:
		strcat(retstr, "DESFIRE_CARD_PARAMETER_ERROR");
		break;

	case DESFIRE_CARD_APPL_INTEGRITY_ERROR:
		strcat(retstr, "DESFIRE_CARD_APPL_INTEGRITY_ERROR");
		break;

	case DESFIRE_CARD_ADDITIONAL_FRAME:
		strcat(retstr, "DESFIRE_CARD_ADDITIONAL_FRAME");
		break;

	case DESFIRE_CARD_PICC_INTEGRITY_ERROR:
		strcat(retstr, "DESFIRE_CARD_PICC_INTEGRITY_ERROR");
		break;

	case DESFIRE_CARD_COMMAND_ABORTED:
		strcat(retstr, "DESFIRE_CARD_COMMAND_ABORTED");
		break;

	case DESFIRE_CARD_PICC_DISABLED_ERROR:
		strcat(retstr, "DESFIRE_CARD_PICC_DISABLED_ERROR");
		break;

	case DESFIRE_CARD_COUNT_ERROR:
		strcat(retstr, "DESFIRE_CARD_COUNT_ERROR");
		break;

	case DESFIRE_CARD_EEPROM_ERROR_DES:
		strcat(retstr, "DESFIRE_CARD_EEPROM_ERROR_DES");
		break;

	case DESFIRE_CARD_FILE_NOT_FOUND:
		strcat(retstr, "DESFIRE_CARD_FILE_NOT_FOUND");
		break;

	case DESFIRE_CARD_FILE_INTEGRITY_ERROR:
		strcat(retstr, "DESFIRE_CARD_FILE_INTEGRITY_ERROR");
		break;

     case COMMIT_TRANSACTION_NO_REPLY:
          strcat(retstr, "COMMIT_TRANSACTION_NO_REPLY");
		break;

     case COMMIT_TRANSACTION_ERROR:
          strcat(retstr, "COMMIT_TRANSACTION_ERROR");
		break;

	default:
		strcat(retstr, "Unknown status");
		break;
	}
		return retstr;
}

//------------------------------------------------------------------------------
char* get_result_str(unsigned short card_status, unsigned short exec_time)
{
	static char retstr[4096] = "";
	const char eol[] = "\r\n";
	const char line_separtor[] = "=============";
	char tmpstr[4096];
	bool prn_time;
//	retstr[0] = 0;
	strcpy(retstr, line_separtor);
	strcat(retstr, eol);

	switch (card_status)
	{
	case READER_ERROR:
		strcat(retstr, "Reader error");
		prn_time = false;
		break;

	case NO_CARD_DETECTED:
		strcat(retstr, "Card did not detected");
		prn_time = false;
		break;

	case CARD_OPERATION_OK:
		strcat(retstr, "OK");
		prn_time = true;
		break;

	case DESFIRE_CARD_APPLICATION_NOT_FOUND:
		strcat(retstr, "AID not found");
		prn_time = true;
		break;

	case WRONG_KEY_TYPE:
		strcat(retstr, "Wrong type of key (is not AES)");
		prn_time = true;
		break;

	case KEY_AUTH_ERROR:
		strcat(retstr, "Wrong key");
		prn_time = true;
		break;

	case DESFIRE_CARD_NO_SUCH_KEY:
		strcat(retstr, "Invalid key number");
		prn_time = true;
		break;

	case DESFIRE_CARD_DUPLICATE_ERROR:
		strcat(retstr, "This AID or File ID already exist");
		prn_time = true;
		break;

	case DESFIRE_CARD_PERMISSION_DENIED:
	case DESFIRE_CARD_AUTHENTICATION_ERROR:
		strcat(retstr, "Permission denied");
		prn_time = true;
		break;

	case DESFIRE_CARD_BOUNDARY_ERROR:
		strcat(retstr, "File boundary error");
		prn_time = false;
		break;

	case DESFIRE_CARD_OUT_OF_EEPROM_ERROR:
		strcat(retstr, "Insuficient of card memory");
		prn_time = false;
		break;

	case DESFIRE_CARD_NO_CHANGES:
		strcat(retstr, "No changes");
		prn_time = false;
		break;

	case DESFIRE_CARD_ILLEGAL_COMMAND_CODE:
		strcat(retstr, "Not supported command");
		prn_time = false;
		break;

	case DESFIRE_CARD_INTEGRITY_ERROR:
		strcat(retstr, "Not valid crypto data");
		prn_time = false;
		break;

	case DESFIRE_CARD_LENGTH_ERROR:
		strcat(retstr, "Length of command error");
		prn_time = false;
		break;

	case DESFIRE_CARD_PARAMETER_ERROR:
		strcat(retstr, "Command parameter error");
		prn_time = false;
		break;

	case DESFIRE_CARD_APPL_INTEGRITY_ERROR:
		strcat(retstr, "Application structure is not valid");
		prn_time = false;
		break;

	case DESFIRE_CARD_ADDITIONAL_FRAME:
		strcat(retstr, "Additional frame expected");
		prn_time = false;
		break;

	case DESFIRE_CARD_PICC_INTEGRITY_ERROR:
		strcat(retstr, "Card error");
		prn_time = false;
		break;

	case DESFIRE_CARD_COMMAND_ABORTED:
		strcat(retstr, "Command not fully completed");
		prn_time = false;
		break;

	case DESFIRE_CARD_PICC_DISABLED_ERROR:
		strcat(retstr, "Card disabled");
		prn_time = false;
		break;

	case DESFIRE_CARD_COUNT_ERROR:
		strcat(retstr, "Maximal number of application is reached");
		prn_time = false;
		break;

	case DESFIRE_CARD_EEPROM_ERROR_DES:
		strcat(retstr, "Card internal EEPROM error");
		prn_time = false;
		break;

	case DESFIRE_CARD_FILE_NOT_FOUND:
		strcat(retstr, "File ID not found");
		prn_time = false;
		break;

	case DESFIRE_CARD_FILE_INTEGRITY_ERROR:
		strcat(retstr, "File integrity error");
		prn_time = false;
		break;

     case COMMIT_TRANSACTION_NO_REPLY:
          strcat(retstr, "Response to the commit transaction was not received");
		prn_time = false;
		break;

     case COMMIT_TRANSACTION_ERROR:
          strcat(retstr, "Error occured during the commit transaction");
		prn_time = false;
		break;

	default:
		strcat(retstr, "Unknown status");

		prn_time = false;

		break;
	}

	strcat(retstr, eol);

	// patch
	prn_time = true;
	if (prn_time)
	{
		sprintf(tmpstr, "Execution time of operation = %d ms", (int) exec_time);
		strcat(retstr, tmpstr);
		strcat(retstr, eol);
	}

	return retstr;
}
//------------------------------------------------------------------------------

void usage(void)
{
		printf(" +------------------------------------------------+\n"
			   " |              uFR Desfire example               |\n"
			   " |                 version " APP_VERSION "                    |\n"
			   " +------------------------------------------------+\n"
			   "                              For exit, hit escape.\n");
		printf(" --------------------------------------------------\n");
		printf("  (0) - Change authentication mode\n"
			   "  (1) - Master key authentication\n"
			   "  (2) - Get card UID\n"
			   "  (3) - Format card\n"
			   "  (4) - DES to AES\n"
			   "  (5) - AES to DES\n"
			   "  (6) - Get free memory\n"
			   "  (7) - Set random ID\n"
			   "  (8) - Internal key lock\n"
			   "  (9) - Internal key unlock\n"
			   "  (a) - Set baud rate\n"
			   "  (b) - Get baud rate\n"
			   "  (c) - Store AES key into reader\n"
			   "  (d) - Change AES key\n"
			   "  (e) - Change key settings\n"
			   "  (f) - Get key settings\n"
			   "  (g) - Make application\n"
			   "  (h) - Delete application\n"
			   "  (j) - Make file\n"
			   "  (k) - Delete file\n"
			   "  (l) - Write Std file\n"
			   "  (m) - Read Std file\n"
			   "  (n) - Read Value file\n"
			   "  (o) - Increase Value file\n"
			   "  (p) - Decrease Value file\n"
			   "  (r) - Change config parameters\n");
			   printf(" --------------------------------------------------\n");
}
//------------------------------------------------------------------------------

void menu(char key)
{
	switch (key)
	{
        case '0':

            internal_key = !internal_key;

            if (internal_key == false)
            {
                std::cout << " Authentication mode is set to PROVIDED KEY\n";

            } else
            {
                std::cout << " Authentication mode is set to INTERNAL KEY\n";
            }
            break;

        case '1':
            master_authent_req = !master_authent_req;

            if (master_authent_req == false)
            {
                std::cout << " Master key authentication is not required\n";

            } else
            {
                std::cout << " Master key authentication is now required\n";
            }
            break;

        case '2':
            GetCardUID();
            break;
        case '3':
            FormatCard();
            break;
        case '4':
            DEStoAES();
            break;
        case '5':
            AEStoDES();
            break;
        case '6':
            GetFreeMemory();
            break;
        case '7':
            SetRandomID();
            break;
        case '8':
            InternalKeysLock();
            break;
        case '9':
            InternalKeysUnlock();
            break;
        case 'a':
            SetBaudRate();
            break;
        case 'b':
            GetBaudRate();
            break;
        case 'c':
            StoreKeyIntoReader();
            break;
        case 'd':
            ChangeAESKey();
            break;
        case 'e':
            ChangeKeySettings();
            break;
        case 'f':
            GetKeySettings();
            break;
        case 'g':
            MakeApplication();
            break;
        case 'h':
            DeleteApplication();
            break;
        case 'j':
            MakeFile();
            break;
        case 'k':
            DeleteFile();
            break;
        case 'l':
            WriteStdFile();
            break;
        case 'm':
            ReadStdFile();
            break;
        case 'n':
            ReadValueFile();
            break;
        case 'o':
            IncreaseValueFile();
            break;
        case 'p':
            DecreaseValueFile();
            break;
        case 'r':
            ChangeSettings();
            break;

		default:
			usage();
			break;
	}
	printf(" --------------------------------------------------\n");
}
//------------------------------------------------------------------------------

int main()
{
	char key;

	UFR_STATUS status;

    status = ReaderOpen();

	if (status != UFR_OK)
	{
		printf("Error while opening device, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}
	status = ReaderReset();
	if (status != UFR_OK)
	{
		ReaderClose();
		printf("Error while opening device, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}


	Sleep(500);

	printf(" --------------------------------------------------\n");
	printf("        uFR NFC reader successfully opened.\n");
	printf(" --------------------------------------------------\n");

	    PrepareSettings();
	    printf(" --------------------------------------------------\n");
	    usage();

	do
	{
		while (!_kbhit())
		{

		}

		key = _getch();
		menu(key);
	}
	while (key != '\x1b');

	ReaderClose();

	return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------

void GetCardUID(void)
{
     UFR_STATUS status;

    unsigned short card_status;
	unsigned short exec_time;

	unsigned char data[192];
	unsigned char data_length;

	unsigned long aid;
	unsigned char aid_key_nr;
	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;


    aid = strtol(settings[1].c_str(),NULL,16);

    aid_key_nr = stoul(settings[2], nullptr, 10);

    if (internal_key == false)
    {
        if (!prepare_key(aes_key_ext))
        {

        }
    } else
    {
        aes_key_nr = stoul(settings[4], nullptr, 10);
    }

    if (internal_key == true)
    {
        status = uFR_int_GetDesfireUid(aes_key_nr, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);

    } else
    {
        status = uFR_int_GetDesfireUid_PK(aes_key_ext, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
    }
    if (status)
    {
        std::cout << "uFR_int_GetDesfireUID(): " << UFR_Status2String(status) << std::endl;
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    std::cout << "CARD UID = ";
        for (int a = 0; a < 7; a++)
        {
        printf("%02X",data[a]);
        }

    std::cout << std::endl;

}
//------------------------------------------------------------------------------

void GetFreeMemory(void)
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	uint32_t mem_size;

	status = uFR_int_DesfireFreeMem(&mem_size,&card_status,&exec_time);

    if (status)
    {
        std::cout << "\nCommunication error";
        std::cout << "\nuFR_int_DesfireFreeMem(): ";
        std::cout <<  UFR_Status2String(status);
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    if (card_status == 3001)
    {
        std::cout << "Free memory: " + std::to_string(mem_size) + " bytes" << std::endl;
    }
}
//------------------------------------------------------------------------------

void FormatCard()
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned char aes_key_nr = 0;
	unsigned char aes_key_ext[16];


    if (internal_key == false)
    {
        if (!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4], nullptr, 10);
    }

   if (internal_key == true)
	{
		status = uFR_int_DesfireFormatCard(aes_key_nr, &card_status, &exec_time);
	}
	else
	{
		status = uFR_int_DesfireFormatCard_PK(aes_key_ext, &card_status, &exec_time);
	}

	if (status)
    {
        std::cout << "Command was not received || Confirmation was not received" << std::endl;
        std::cout << "uFR_int_DesfireFormatCard(): " << UFR_Status2String(status) << std::endl;
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    if (card_status == 3001)
    {
        std::cout << "Card is formatted" << std::endl;
    }
}
//------------------------------------------------------------------------------

void DEStoAES()
{
    UFR_STATUS status;
    unsigned char i, res;

    res = 0;

    for (i = 0; i < 10; i++)
    {
        status = DES_to_AES_key_type();
            if(!status)
            {
                std::cout << "Operation succesfull" << std::endl;
                std::cout << "Key type changed to AES" << std::endl;
                std::cout << "New AES key is 00000000000000000000000000000000" << std::endl;
                res = 1;
                ReaderUISignal(1,3);
                break;
            }
    }

   if (res == 0)
   {
    std::cout << "Error: " << std::endl;
    std::cout << "Function status: " << UFR_Status2String(status) << std::endl;
   }

}
//------------------------------------------------------------------------------

void AEStoDES()
{
    UFR_STATUS status;
    unsigned char i, res;

    res = 0;

    for (i = 0; i < 10; i++)
    {
        status = DES_to_AES_key_type();
            if(!status)
            {
                std::cout << "Operation succesful" << std::endl;
                std::cout << "Key type changed to DES" << std::endl;
                std::cout << "New AES key is 0000000000000000" << std::endl;
                res = 1;
                ReaderUISignal(1,3);
                break;
            }
    }

   if (res == 0)
   {
    std::cout << "Error: " << std::endl;
    std::cout << "Function status: " << UFR_Status2String(status) << std::endl;
   }

}

void SetRandomID()
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned char aes_key_nr = 0;
	unsigned char aes_key_ext[16];

	if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {

        }
    } else
    {
        aes_key_nr = stoul(settings[4], nullptr, 10);
    }

    if (internal_key == true)
    {
        status = uFR_int_DesfireSetConfiguration(aes_key_nr, 1, 0, &card_status, &exec_time);
	}else
	{
		status = uFR_int_DesfireSetConfiguration_PK(aes_key_ext, 1, 0, &card_status, &exec_time);
	}

	if (status)
    {
        std::cout << "Communication error" << std::endl;
        std::cout <<"uFR_int_DesfireSetConfiguration(): " << UFR_Status2String(status) << std::endl;
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    if (card_status == 3001)
    {
        std::cout << "\nRandom UID is set\n";
    }
}

void GetBaudRate()
{
    unsigned char tx_speed, rx_speed;
    GetSpeedParameters(&tx_speed,&rx_speed);

    if (tx_speed == 0)
    {
        std::cout << "TX baud rate = 106 kbps;" << std::endl;
    } else if (tx_speed == 1)
    {
        std::cout << "TX baud rate = 212 kbps;" << std::endl;

    }
    else if (tx_speed == 2)
    {
        std::cout << "TX baud rate = 424 kbps;" << std::endl;

    }else if (tx_speed == 3)
    {
        std::cout << "TX baud rate = 848 kbps;" << std::endl;
    }

    if (rx_speed == 0)
    {
        std::cout << "RX baud rate = 106 kbps;" << std::endl;
    } else if (rx_speed == 1)
    {
        std::cout << "RX baud rate = 212 kbps;" << std::endl;

    }
    else if (rx_speed == 2)
    {
        std::cout << "RX baud rate = 424 kbps;" << std::endl;

    }else if (rx_speed == 3)
    {
        std::cout << "RX baud rate = 848 kbps;" << std::endl;
    }
}
//------------------------------------------------------------------------------

void SetBaudRate()
{
    UFR_STATUS status;
    unsigned char tx_speed, rx_speed;

    std::cout << "Enter value for setting transmit rate (tx speed)" << std::endl;
    std::cout << "0 - 106 kbps" << std::endl;
    std::cout << "1 - 212 kbps" << std::endl;
    std::cout << "2 - 424 kbps" << std::endl;
    std::cout << "3 - 848 kbps" << std::endl;
    std::cin >> tx_speed;

    std::cout << "Enter value for setting receive rate (rx speed)" << std::endl;
    std::cout << "0 - 106 kbps" << std::endl;
    std::cout << "1 - 212 kbps" << std::endl;
    std::cout << "2 - 424 kbps" << std::endl;
    std::cout << "3 - 848 kbps" << std::endl;
    std::cin >> rx_speed;



   status = SetSpeedPermanently(tx_speed,rx_speed);

    if (status)
    {
        std::cout << "Communication error" << std::endl;
        std::cout << "SetSpeedPermanently(): " << UFR_Status2String(status) << std::endl;
    } else
    {
        std::cout << "Operation completed. Status is: " << UFR_Status2String(status) << std::endl;
    }

}
//------------------------------------------------------------------------------

void StoreKeyIntoReader()
{

    UFR_STATUS status;

    int key_no = 0;

    unsigned char aes_key[16];

	std::string key;

    std::cout << "Input AES key number (0-15):" << std::endl;

    scanf("%d", &key_no);

    std::cout << "Enter key you want to enter: " << std::endl;

    std::cin >> key;



    if (key.length() != 32)
    {
        std::cout << "Key must be 16 bytes long" << std::endl;

    } else
    {
        Convert(key,aes_key);

       status = uFR_int_DesfireWriteAesKey(key_no,aes_key);

        if (status)
        {
             std::cout << "Error: " << std::endl;
            if (status == UFR_KEYS_LOCKED)
            {
                std::cout << "Internal keys are locked" << std::endl;
            }
            std::cout << "Function status: " << UFR_Status2String(status) << std::endl;
        } else
        {
            std::cout << "Operation completed. Status is " << UFR_Status2String(status) << std::endl;

        }

    }
}
//------------------------------------------------------------------------------

void InternalKeysLock()
{
    UFR_STATUS status;
    std::string pass_text;

    std::cout << "Input password (8 characters): " << std::endl;
    std::cin >> pass_text;

    if (pass_text.length() != 8)
    {
        std::cout << "Password must be 8 characters long" << std::endl;
        return;
    }

    char *pass_array = new char[pass_text.length()+1];
    strcpy(pass_array,pass_text.c_str());


    status = ReaderKeysLock((const uint8_t*)pass_array);


    if (status == 0)
    {
        std::cout << "Operation completed. Status is " << UFR_Status2String(status) <<std::endl;
    } else
    {

        std::cout << "Error :" << std::endl;

        std::cout << "Function status: " << UFR_Status2String(status) << std::endl;
    }

}
//------------------------------------------------------------------------------

void InternalKeysUnlock()
{
   UFR_STATUS status;
    std::string pass_text;

    std::cout << "Input password: " << std::endl;
    std::cin >> pass_text;

    if (pass_text.length() != 8)
    {
        std::cout << "Password must be 8 characters long" << std::endl;
        return;
    }

    char *pass_array = new char[pass_text.length()+1];
    strcpy(pass_array,pass_text.c_str());


    status = ReaderKeysUnlock((const uint8_t*)pass_array);

    if (status == 0)
    {

        std::cout << "Operation completed. Status is: " << UFR_Status2String(status) <<std::endl;
    } else
    {

        std::cout << "Error :" << std::endl;

        std::cout << "Function status: " << UFR_Status2String(status) << std::endl;
    }

}
//------------------------------------------------------------------------------

void ChangeKeySettings()
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned char setting;

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0, set_temp = 0;
	unsigned long aid;

    int choice = 0;

            std::cout << " Choose key settings:" << std::endl;
            std::cout << " 0 - No settings" << std::endl;
            std::cout << " 1 - Settings not changeable anymore" << std::endl;
            std::cout << " 2 - Create or delete application with master key authentication" << std::endl;
            std::cout << " 3 - Master key not changeable anymore" << std::endl;
            std::cout << " 4 - Settings not changeable anymore and create or delete application with master key" << std::endl;
            std::cout << " 5 - Settings and master key not changeable anymore" << std::endl;
            std::cout << " 6 - Create and delete application with master key and master key is not changeable anymore" << std::endl;
            std::cout << " 7 - Settings not changeable anymore, create or delete application with master key,";
            std::cout << " master key is not changeable anymore" << std::endl;

            scanf("%d",&choice);

            if(choice == 1)
            {
                set_temp |= 0x04;

            }else if(choice == 2)
            {
                set_temp |= 0x02;
            }
            else if(choice == 3)
            {
                set_temp |= 0x01;

            }else if(choice == 4)
            {
                set_temp |= 0x04;
                set_temp |= 0x02;

            }else if(choice == 5)
            {
                set_temp |= 0x04;
                set_temp |= 0x01;

            }else if(choice == 6)
            {
                set_temp |= 0x02;
                set_temp |= 0x01;
            }else if(choice == 7)
            {
                set_temp |= 0x04;
                set_temp |= 0x02;
                set_temp |= 0x01;
            }

            switch (set_temp)
            {
                case 0:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_CHANGE_KEY_CHANGE;
                    break;
                case 1:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_CHANGE_KEY_NOT_CHANGE;
                    break;
                case 2:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_CHANGE_KEY_CHANGE;
                    break;
                case 3:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_CHANGE_KEY_NOT_CHANGE;
                    break;
                case 4:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_NOT_CHANGE_KEY_CHANGE;
                    break;
                case 5:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_NOT_CHANGE_KEY_NOT_CHANGE;
                    break;
                case 6:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_NOT_CHANGE_KEY_CHANGE;
                    break;
                case 7:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_NOT_CHANGE_KEY_NOT_CHANGE;
                    break;
            }

            aid = strtol(settings[1].c_str(),NULL,16);

            if (internal_key == false)
            {
                if(!prepare_key(aes_key_ext))
                {
                    return;
                }
            } else
            {
                aes_key_nr = stoul(settings[4],nullptr,10);
            }

            if (internal_key == true)
            {
                status = uFR_int_DesfireChangeKeySettings(aes_key_nr, aid, setting, &card_status, &exec_time);
            }
            else
            {
                status = uFR_int_DesfireChangeKeySettings_PK(aes_key_ext, aid, setting, &card_status, &exec_time);
            }

            if (status)
            {
                std::cout << std::endl << "Communication error" << std::endl;
                std::cout << "uFR_int_DesfireChangeKeySettings(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;
}
//------------------------------------------------------------------------------

void GetKeySettings()
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned char setting, set_temp = 0;
	unsigned char max_key_no;

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;


	aid = strtol(settings[1].c_str(),NULL,16);

    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    if(internal_key == true)
    {
        status = uFR_int_DesfireGetKeySettings(aes_key_nr, aid, &setting, &max_key_no, &card_status, &exec_time);
	}
	else
	{
		status = uFR_int_DesfireGetKeySettings_PK(aes_key_ext, aid, &setting, &max_key_no, &card_status, &exec_time);
	}

	if(status)
    {
        std::cout << "Communication error" <<std::endl;
        std::cout << "uFR_int_DesfireGetKeySettings(): " << UFR_Status2String(status) << std::endl;
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    if (card_status == 3001)
    {

        std::cout << "Maximal number of keys into application: " << std::to_string(max_key_no) << std::endl;

        setting &= 0x0F;

		switch (setting)
		{
		case DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_CHANGE_KEY_CHANGE:
			set_temp = 0;
			break;
		case DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_CHANGE_KEY_NOT_CHANGE:
			set_temp = 1;
			break;
		case DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_CHANGE_KEY_CHANGE:
			set_temp = 2;
			break;
		case DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_CHANGE_KEY_NOT_CHANGE:
			set_temp = 3;
			break;
		case DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_NOT_CHANGE_KEY_CHANGE:
			set_temp = 4;
			break;
		case DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_NOT_CHANGE_KEY_NOT_CHANGE:
			set_temp = 5;
			break;
		case DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_NOT_CHANGE_KEY_CHANGE:
			set_temp = 6;
			break;
		case DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_NOT_CHANGE_KEY_NOT_CHANGE:
			set_temp = 7;
			break;
		}

        if (set_temp & 0x04)
			set_not_changeable = true;
		else
			set_not_changeable = false;

		if (set_temp & 0x02)
			create_with_master = true;
		else
			create_with_master = false;

		if (set_temp & 0x01)
			master_not_changeable = true;
		else
			master_not_changeable = false;

            if (set_not_changeable == true && create_with_master == true && master_not_changeable == true)
            {
                std::cout << "7 - Settings not changable anymore, create or delete application with master key, master key is not changeable anymore" << std::endl;
            }
            else if (set_not_changeable == false && create_with_master == true && master_not_changeable == true)
            {
                std::cout << "6 - Create and delete application with master key and master key is not changable anymore" << std::endl;
            }
            else if (set_not_changeable == true && create_with_master == false && master_not_changeable == true)
            {
                std::cout << "5 - Settings and master key not changable anymore" << std::endl;
            }
            else if (set_not_changeable == true && create_with_master == true && master_not_changeable == false)
            {
                std::cout << "4 - Settings not changeable anymore and create or delete application with master key" << std::endl;
            }
            else if (set_not_changeable == false && create_with_master == false && master_not_changeable == true)
            {
                std::cout << "3 - Master key not changeable anymore" << std::endl;
            }
            else if (set_not_changeable == false && create_with_master == true && master_not_changeable == false)
            {
                std::cout << "2 - Create or delete application with master key authentication" << std::endl;
            }
            else if (set_not_changeable == true && create_with_master == false && master_not_changeable == false)
            {
                std::cout << "1 - Settings not changeable anymore" << std::endl;
            }
            else if (set_not_changeable == false && create_with_master == false && master_not_changeable == false)
            {
                std::cout << "0 - No settings set." << std::endl;
            }
    }
}

//------------------------------------------------------------------------------

void ChangeAESKey()
{

    UFR_STATUS status;

    unsigned short card_status;
	unsigned short exec_time;

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;
	unsigned char aid_key_nr_auth;
	int aid_key_nr;
	unsigned char new_aes_key[16];
	unsigned char old_aes_key[16];

	std::string old_key = "", new_key = "";

	 std::cout << "Input old AES key (16 bytes): " << std::endl;

    std::cin >> old_key;

    if (old_key.length() != 32)
    {
        std::cout << "Key must be 16 bytes long" << std::endl;

    } else
    {
        Convert(old_key,old_aes_key);
    }

    std::cout << "Input new new key (16 bytes): " << std::endl;

    std::cin >> new_key;

    if (new_key.length() != 32)
    {
        std::cout << "Key must be 16 bytes long" << std::endl;

    } else
    {
        Convert(new_key,new_aes_key);
    }


    aid = strtol(settings[1].c_str(),NULL,16);

    aid_key_nr_auth = stoul(settings[2],nullptr,10);

    printf("Input key number to change: ");
    scanf("%d", &aid_key_nr);


    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    if (internal_key == true)
    {
        status = uFR_int_DesfireChangeAesKey(aes_key_nr, aid, aid_key_nr_auth, new_aes_key, aid_key_nr, old_aes_key, &card_status, &exec_time);
	}
	else
	{
		status = uFR_int_DesfireChangeAesKey_PK(aes_key_ext, aid, aid_key_nr_auth, new_aes_key, aid_key_nr, old_aes_key, &card_status, &exec_time);
	}

	if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireChangeAesKey(): " << UFR_Status2String(status) << std::endl;
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

}
//------------------------------------------------------------------------------

void MakeApplication()
{

    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned char setting, set_temp = 0;
	unsigned long aid;
	std::string str_aid = "";
	int max_key_no;

	unsigned char aes_key_ext[16];
	unsigned char  aes_key_nr = 0;

	int choice = 0;


            printf("Input  AID tnumber (3 bytes hex): ");
            std::cin >> str_aid;

            aid = strtol(str_aid.c_str(), NULL,16);

            std::cout << std::endl;
            printf("Input maximal key number: ");
            scanf("%d", &max_key_no);

            std::cout << " Choose application settings:" << std::endl;
            std::cout << " 0 - No settings" << std::endl;
            std::cout << " 1 - Settings not changeable anymore" << std::endl;
            std::cout << " 2 - Create or delete application with master key authentication" << std::endl;
            std::cout << " 3 - Master key not changeable anymore" << std::endl;
            std::cout << " 4 - Settings not changeable anymore and create or delete application with master key" << std::endl;
            std::cout << " 5 - Settings and master key not changeable anymore" << std::endl;
            std::cout << " 6 - Create and delete application with master key and master key is not changeable anymore" << std::endl;
            std::cout << " 7 - Settings not changeable anymore, create or delete application with master key,";
            std::cout << " master key is not changeable anymore" << std::endl;

            scanf("%d",&choice);

            if(choice == 1)
            {
                set_temp |= 0x04;

            }else if(choice == 2)
            {
                set_temp |= 0x02;
            }
            else if(choice == 3)
            {
                set_temp |= 0x01;

            }else if(choice == 4)
            {
                set_temp |= 0x04;
                set_temp |= 0x02;

            }else if(choice == 5)
            {
                set_temp |= 0x04;
                set_temp |= 0x01;

            }else if(choice == 6)
            {
                set_temp |= 0x02;
                set_temp |= 0x01;
            }else if(choice == 7)
            {
                set_temp |= 0x04;
                set_temp |= 0x02;
                set_temp |= 0x01;
            }

            switch (set_temp)
            {
                case 0:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_CHANGE_KEY_CHANGE;
                    break;
                case 1:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_CHANGE_KEY_NOT_CHANGE;
                    break;
                case 2:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_CHANGE_KEY_CHANGE;
                    break;
                case 3:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_CHANGE_KEY_NOT_CHANGE;
                    break;
                case 4:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_NOT_CHANGE_KEY_CHANGE;
                    break;
                case 5:
                    setting = DESFIRE_KEY_SET_CREATE_WITHOUT_AUTH_SET_NOT_CHANGE_KEY_NOT_CHANGE;
                    break;
                case 6:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_NOT_CHANGE_KEY_CHANGE;
                    break;
                case 7:
                    setting = DESFIRE_KEY_SET_CREATE_WITH_AUTH_SET_NOT_CHANGE_KEY_NOT_CHANGE;
                    break;
            }

            if (internal_key == false)
            {
                if(!prepare_key(aes_key_ext))
                {
                    return;
                }
            } else
            {
                aes_key_nr = stoul(settings[4], nullptr, 10);
            }

            if (master_authent_req == true)
            {
                if (internal_key == true)
                {
                status = uFR_int_DesfireCreateAesApplication(aes_key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                }
                else
                {
                    status = uFR_int_DesfireCreateAesApplication_PK(aes_key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                }
            }
            else
            {
                status = uFR_int_DesfireCreateAesApplication_no_auth(aid, setting, max_key_no, &card_status, &exec_time);
            }

            if (status)
        {
            std::cout << std::endl << "Communication error";
            std::cout << std::endl << "uFR_int_DesfireCreateApplication(): " << UFR_Status2String(status) << std::endl;
        }

        card_operation_status = get_result_str(card_status,exec_time);
        std::cout << card_operation_status;

        if (card_status == 3001)
        {
            std::cout << std::endl << "Application created" << std::endl;
        }
}
//------------------------------------------------------------------------------

void DeleteApplication()
{
	UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	//unsigned long aid;

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;

    std::string str_aid = "";

            printf("Input AID to delete (3 bytes hex): ");
            std::cin >> str_aid;

            aid = strtol(str_aid.c_str(), NULL,16);

            if (internal_key == false)
            {
                if(!prepare_key(aes_key_ext))
                {
                    return;
                }
            } else
            {
                aes_key_nr = stoul(settings[4], nullptr,10);
            }


            if (internal_key == true)
            {
                status = uFR_int_DesfireDeleteApplication(aes_key_nr, aid, &card_status, &exec_time);
            }
            else
            {
                status = uFR_int_DesfireDeleteApplication_PK(aes_key_ext, aid, &card_status, &exec_time);
            }

            if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireDeleteApplication(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;
}
//------------------------------------------------------------------------------

void MakeFile(){
	UFR_STATUS status = UFR_OK;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned long aid;
	int file_size;

	unsigned char communication_settings;
	int file_id;
	int read_key_nr, write_key_nr, read_write_key_nr, change_key_nr;

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
    int lower_limit = 0, upper_limit = 0, value_value = 0;
    unsigned char limited_credit_enabled = 0;
    int limited_credit_choice = 0, free_get_choice = 0;
    int comm_choice = 0, file_choice = 0;

    aid = strtol(settings[1].c_str(), NULL,16);

    printf("Input File ID: ");
    scanf("%d", &file_id);


    printf("Choose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);

    printf("Choose file type:\n 1 - Standard data file\n 2 - Value file\n");
    scanf("%d", &file_choice);

    printf("Enter Read key number: ");
    scanf("%d", &read_key_nr);

    printf("Enter Write key number: ");
    scanf("%d", &write_key_nr);

    printf("Enter Read/Write key number: ");
    scanf("%d", &read_write_key_nr);

    printf("Enter Change key number: ");
    scanf("%d", &change_key_nr);

    switch (comm_choice)
	{
	case 1:
		communication_settings = 0;
		break;
	case 2:
		communication_settings = 1;
		break;
	case 3:
		communication_settings = 3;
		break;
	default:
		communication_settings = 0;
		break;
	}

	if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    if (file_choice == 1)
    {
    printf("Enter size of the file you wish to create: ");
    scanf("%d", &file_size);
    } else if (file_choice == 2)
    {
        printf("Enter lower limit of your Value file: ");
        scanf("%d", &lower_limit);


        printf("Enter upper limit of your Value file: ");
        scanf("%d", &upper_limit);


        printf("Enter value of your Value file: ");
        scanf("%d",&value_value);


        printf("Do you wish to enable Limited credit?\n 1 - Yes\n 2 - No\n");
        scanf("%d",&limited_credit_choice);

        if (limited_credit_choice == 1)
                limited_credit_enabled = 1;
        else if (limited_credit_choice == 2)
                limited_credit_enabled = 0;

        printf("Do you wish to enable Free get value?\n 1 - Yes\n 2 - No\n");
        scanf("%d",&free_get_choice);
        if (free_get_choice == 1)
            limited_credit_enabled |= 0x02;
        else if (free_get_choice == 2) {} //nothing happens

    }




if (master_authent_req == true)
{
    if (internal_key == true)
    {
        if (file_choice == 1)
        {
            status = uFR_int_DesfireCreateStdDataFile(aes_key_nr, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
        }else if (file_choice == 2)
        {
            status = uFR_int_DesfireCreateValueFile(aes_key_nr, aid, file_id,
                         lower_limit, upper_limit, value_value, limited_credit_enabled,
                         read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                         communication_settings, &card_status, &exec_time);

        }

    }else
        {
            if (file_choice == 1)
        {
            status = uFR_int_DesfireCreateStdDataFile_PK(aes_key_ext, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
        }else if (file_choice == 2)
        {
            status = uFR_int_DesfireCreateValueFile_PK(aes_key_ext, aid, file_id,
                         lower_limit, upper_limit, value_value, limited_credit_enabled,
                         read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                         communication_settings, &card_status, &exec_time);
        }

        }


    } else
    {
        if (file_choice == 1)
        {
            status = uFR_int_DesfireCreateStdDataFile_no_auth(aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
        }else
            status = uFR_int_DesfireCreateValueFile_no_auth(aid, file_id,
                         lower_limit, upper_limit, value_value, limited_credit_enabled,
                         read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                         communication_settings, &card_status, &exec_time);
    }

    if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireCreateFile(): " << UFR_Status2String(status) << std::endl;
            }

            card_operation_status = get_result_str(card_status,exec_time);
            std::cout << card_operation_status << std::endl;

            if (card_status == 3001)
            {
                std::cout << std::endl << "File created" << std::endl;
            }
}
void DeleteFile()
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	int file_no;

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;

	printf("Enter file ID to delete: \n");
	scanf("%d", &file_no);

	if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
           {
               return;
           }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    aid = strtol(settings[1].c_str(), NULL, 16);


    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            status = uFR_int_DesfireDeleteFile(aes_key_nr, aid, file_no, &card_status, &exec_time);
		}
		else
		{
			status = uFR_int_DesfireDeleteFile_PK(aes_key_ext, aid, file_no, &card_status, &exec_time);
		}
	}
	else
	{
		status = uFR_int_DesfireDeleteFile_no_auth(aid, file_no, &card_status, &exec_time);
	}

         if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireDeleteFile(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;
}
//------------------------------------------------------------------------------

void ReadValueFile()
{
    UFR_STATUS status;
	unsigned short card_status = 0;
	unsigned short exec_time;

	unsigned char communication_settings;
	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
    int file_value = 0, comm_choice = 0;


    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    aid = strtol(settings[1].c_str(), NULL, 16);

     aid_key_nr = stoul(settings[2], nullptr, 10);

     file_id = stoul(settings[3], nullptr, 10);

    printf("\nChoose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);
    switch (comm_choice)
	{
	case 1:
		communication_settings = 0;
		break;
	case 2:
		communication_settings = 1;
		break;
	case 3:
		communication_settings = 3;
		break;
	default:
		communication_settings = 0;
		break;
	}


	if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            status =  uFR_int_DesfireReadValueFile(aes_key_nr, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
		}
		else
		{
			status = 	uFR_int_DesfireReadValueFile_PK(aes_key_ext, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
		}
	}
	else
    {
        status =  uFR_int_DesfireReadValueFile_no_auth(aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
    }

    if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireReadValueFile(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    std::cout << "Value: " << std::to_string(file_value) << std::endl;
}
//------------------------------------------------------------------------------

void IncreaseValueFile()
{
    UFR_STATUS status;
	unsigned short card_status = 0;
	unsigned short exec_time;

	unsigned char communication_settings;
	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
    int file_value = 0, comm_choice = 0;


    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    aid = strtol(settings[1].c_str(), NULL, 16);

     aid_key_nr = stoul(settings[2], nullptr, 10);

     file_id = stoul(settings[3], nullptr, 10);

    printf("\nChoose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);
    switch (comm_choice)
	{
	case 1:
		communication_settings = 0;
		break;
	case 2:
		communication_settings = 1;
		break;
	case 3:
		communication_settings = 3;
		break;
	default:
		communication_settings = 0;
		break;
	}

	printf("\nValue for increasing: \n");
    scanf("%d", &file_value);


	if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            status =  uFR_int_DesfireIncreaseValueFile(aes_key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
		}
		else
		{
			status = 	uFR_int_DesfireIncreaseValueFile_PK(aes_key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
		}
	}
	else
    {
        status =  uFR_int_DesfireIncreaseValueFile_no_auth(aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
    }

    if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireReadValueFile(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    std::cout << "Value increased by: " << std::to_string(file_value) << std::endl;

}
//------------------------------------------------------------------------------

void DecreaseValueFile()
{
    UFR_STATUS status;
	unsigned short card_status = 0;
	unsigned short exec_time;

	unsigned char communication_settings;
	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
    int file_value = 0, comm_choice = 0;


    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }

    aid = strtol(settings[1].c_str(), NULL, 16);

     aid_key_nr = stoul(settings[2], nullptr, 10);

     file_id = stoul(settings[3], nullptr, 10);

    printf("\nChoose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);
    switch (comm_choice)
	{
	case 1:
		communication_settings = 0;
		break;
	case 2:
		communication_settings = 1;
		break;
	case 3:
		communication_settings = 3;
		break;
	default:
		communication_settings = 0;
		break;
	}

	printf("\nValue for decreasing: \n");
    scanf("%d", &file_value);


	if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            status =  uFR_int_DesfireDecreaseValueFile(aes_key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
		}
		else
		{
			status = 	uFR_int_DesfireDecreaseValueFile_PK(aes_key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
		}
	}
	else
    {
        status =  uFR_int_DesfireDecreaseValueFile_no_auth(aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
    }

    if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireDecreaseValueFile(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    std::cout << "Value decreased by: " << std::to_string(file_value) << std::endl;

}

//------------------------------------------------------------------------------

void WriteStdFile()
{
    UFR_STATUS status;
	unsigned short card_status = 0;
	unsigned short exec_time;

	unsigned char file_data[10000];
	int file_length;
	unsigned char communication_settings;
	int comm_choice = 0;


	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;


    FILE *stream;

    stream = fopen("write.txt", "rb");

    int curpos, length;

    curpos = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);

	printf("\nChoose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);
    switch (comm_choice)
	{
	case 1:
		communication_settings = 0;
		break;
	case 2:
		communication_settings = 1;
		break;
	case 3:
		communication_settings = 3;
		break;
	default:
		communication_settings = 0;
		break;
	}

	aid = strtol(settings[1].c_str(),NULL,16);

    aid_key_nr = stoul(settings[2], nullptr, 10);

    file_id = stoul(settings[3], nullptr, 10);

    file_length = length;

    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4], nullptr, 10);
    }


    fread(file_data,file_length,1,stream);
    fclose(stream);

    if(master_authent_req == true)
    {
        if (internal_key == true)
        {
            status = uFR_int_DesfireWriteStdDataFile(aes_key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
		}
		else
		{
			status = uFR_int_DesfireWriteStdDataFile_PK(aes_key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
		}
    }
    else
    {
        status = uFR_int_DesfireWriteStdDataFile_no_auth(aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
    }

    if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireWriteStdDataFile(): " << UFR_Status2String(status) << std::endl;
            }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

}
//------------------------------------------------------------------------------

void ReadStdFile()
{
    UFR_STATUS status;
	unsigned short card_status = 0;
	unsigned short exec_time;

	int file_length;
	unsigned char communication_settings;
	FILE *stream;
	unsigned char file_data[10000];

	unsigned char aes_key_ext[16];
	unsigned char aes_key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;

	int comm_choice;

	stream = fopen("read.txt", "wb");

    printf("Input file length to read: ");
    scanf("%d", &file_length);

	printf("\nChoose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);
    switch (comm_choice)
	{
	case 1:
		communication_settings = 0;
		break;
	case 2:
		communication_settings = 1;
		break;
	case 3:
		communication_settings = 3;
		break;
	default:
		communication_settings = 0;
		break;
	}

	aid = strtol(settings[1].c_str(),NULL,16);

    aid_key_nr = stoul(settings[2], nullptr, 10);

    file_id = stoul(settings[3], nullptr, 10);



    if (internal_key == false)
    {
        if(!prepare_key(aes_key_ext))
        {
            return;
        }
    } else
    {
        aes_key_nr = stoul(settings[4],nullptr,10);
    }


    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            status = uFR_int_DesfireReadStdDataFile(aes_key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
		}
		else
		{
			status = uFR_int_DesfireReadStdDataFile_PK(aes_key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
		}
	}
	else
	{
		status = uFR_int_DesfireReadStdDataFile_no_auth(aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
	}

	if (status)
            {
                std::cout << std::endl << "Communication error";
                std::cout << std::endl << "uFR_int_DesfireReadStdDataFile(): " << UFR_Status2String(status) << std::endl;
            }

    fwrite(file_data, file_length, 1, stream);
    fclose(stream);

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

}

void print_settings()
{
    printf("1 - Change AES key\n");
    printf("2 - Change AID\n");
    printf("3 - Change AID key number\n");
    printf("4 - Change File ID\n");
    printf("5 - Change internal key number\n");
    printf("esc - Exit to main menu\n");
}



void ChangeSettings()
{

    printf("Current config:\n");
    printf("\tAES key: %s\n", settings[0].c_str());
    printf("\tAID: %s\n", settings[1].c_str());
    printf("\tAID key number auth: %s\n", settings[2].c_str());
    printf("\tFile ID: %s\n", settings[3].c_str());
    printf("\tInternal key nr: %s\n", settings[4].c_str());

    printf("1 - Change AES key\n");
    printf("2 - Change AID\n");
    printf("3 - Change AID key number\n");
    printf("4 - Change File ID\n");
    printf("5 - Change internal key number\n");
    printf("esc - Exit to main menu\n");


    char key;
    std::string new_aes_key = "";
    std::string new_aid = "";
    std::string new_aid_key_nr = "";
    std::string new_file_id = "";
    std::string new_internal_key_nr = "";

    do
	{
		while (!_kbhit())
		{

		}

		key = _getch();

		switch(key)
            {
            case '1':

                std::cout << "Input new AES key (16 bytes):" << std::endl;
                std::cin >> new_aes_key;
                if (new_aes_key.length() != 32)
                {
                    std::cout << "aes key must be 16 bytes long" << std::endl;
                    return;
                }
                settings[0] = new_aes_key;
                print_settings();
                break;

            case '2':
                std::cout << "Input new AID (3 bytes hex):" << std::endl;
                std::cin >> new_aid;
                if(new_aid.length() != 6)
                {
                    std::cout << "AID must be 3 hex bytes long" << std::endl;
                    return;
                }
                settings[1] = new_aid;
                print_settings();
                break;

            case '3':
                std::cout << "Input new AID key number (0-13):" << std::endl;
                std::cin >> new_aid_key_nr;
                if (stoul(new_aid_key_nr,nullptr,10) > 13 || stoul(new_aid_key_nr,nullptr,10) < 0)
                {
                    std::cout << "AID key number must be between 0 and 13" << std::endl;
                    return;
                }
                settings[2] = new_aid_key_nr;
                print_settings();
                break;

            case '4':
                std::cout << "Input new File ID  (0-31):" << std::endl;
                std::cin >> new_file_id;
                if (stoul(new_file_id,nullptr,10) > 31 || stoul(new_file_id,nullptr,10) < 0)
                {
                    std::cout << "File ID must be between 0 and 31" << std::endl;
                    return;
                }
                settings[3] = new_file_id;
                print_settings();
                break;

            case '5':
                std::cout << "Input new internal key number (0-15)" << std::endl;
                std::cin >> new_internal_key_nr;
                if (stoul(new_internal_key_nr,nullptr,10) > 15 || stoul(new_internal_key_nr,nullptr,10) < 0)
                {
                    std::cout << "Internal key number must be between 0 and 15" << std::endl;
                    return;
                }
                settings[4] = new_internal_key_nr;
                print_settings();
                break;

            default:
            print_settings();
                break;
            }
	} while (key != '\x1b');

   std::ofstream myfile("..\\..\\config.txt");

   if (myfile.is_open())
   {
       myfile << "AES key: " << settings[0] << std::endl;
       myfile << "AID 3 bytes hex: " << settings[1] << std::endl;
       myfile << "AID key number for auth: " << settings[2] << std::endl;
       myfile << "File ID: " << settings[3] << std::endl;
       myfile << "Internal key number: " << settings[4] << std::endl;
   } else
   {
       std::cout << "Couldn't write new settings to config.txt";
   }
}
