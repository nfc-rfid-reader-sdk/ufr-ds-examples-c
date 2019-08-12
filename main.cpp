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

#define APP_VERSION	"1.2"
#include "lib/include/uFCoder.h"

void convert_str_to_key(std::string key_str, unsigned char *key, unsigned char key_length);
bool prepare_key(unsigned char *key);
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
void ChangeKey(void);
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
void GetApplicationIds(void);
void ClearRecord(void);

bool set_not_changeable = false, create_with_master = false, master_not_changeable = false;

std::string card_operation_status = "";
std::string settings[7];

unsigned char key_ext[16];
unsigned long aid;
unsigned char aid_key_nr;
unsigned char key_nr;
bool internal_key = false, master_authent_req = true;
unsigned char file_id;
unsigned char key_type_nr;

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
    //std::ifstream myfile("..\\..\\config.txt");
    std::ifstream myfile("config.txt");
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

      std::cout << std::endl << "Unable to open config,txt file." << std::endl;
      exit(0);

  }

  //get key type from first line
  int key_type_found;
  key_type_nr = 0xFF;
  key_type_found = settings[0].find("AES");
  if(key_type_found != -1)
        key_type_nr = AES_KEY_TYPE;
  else
  {
      key_type_found = settings[0].find("3K3DES");
      if(key_type_found != -1)
            key_type_nr = DES3K_KEY_TYPE;
      else
      {
          key_type_found = settings[0].find("2K3DES");
          if(key_type_found != -1)
                key_type_nr = DES2K_KEY_TYPE;
          else
          {
                key_type_found = settings[0].find("DES");
                if(key_type_found != -1)
                    key_type_nr = DES_KEY_TYPE;
          }
      }
  }

  if(key_type_nr == 0xFF)
  {
      std::cout << std::endl << "Unable to find key type in config.txt file." << std::endl;
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
bool prepare_key(unsigned char *key)
{
    int key_length;
    std::string key_text;

    key_text = settings[0];
    key_length = key_text.length();

    if(key_type_nr == AES_KEY_TYPE)
    {
        if (key_length != 32)
        {
            std::cout << "Key length must be 16 bytes" << std::endl;
            return false;
        }

        convert_str_to_key(key_text, key, 16);
    }
    else if(key_type_nr == DES_KEY_TYPE)
    {
        if (key_length != 16)
        {
            std::cout << "Key length must be 8 bytes" << std::endl;
            return false;
        }

        convert_str_to_key(key_text, key, 8);
    }
    else if(key_type_nr == DES2K_KEY_TYPE)
    {
        if (key_length != 32)
        {
            std::cout << "Key length must be 16 bytes" << std::endl;
            return false;
        }

        convert_str_to_key(key_text, key, 16);
    }
    else if(key_type_nr == DES3K_KEY_TYPE)
    {
        if (key_length != 48)
        {
            std::cout << "Key length must be 24 bytes" << std::endl;
            return false;
        }

        convert_str_to_key(key_text, key, 24);
    }
    return true;
}

//-------------------------------------------------------------------
void convert_str_to_key(std::string key_str, unsigned char *key, unsigned char key_length)
{
    char key_part[8];
	unsigned long key_nr;
	unsigned char temp[4];
	unsigned char i;

    char *temp_str = (char*)key_str.c_str();

	memset(key_part, 0, 8);
	memcpy(key_part, temp_str, 6);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		key[i] = temp[2 - i];

	memset(key_part, 0, 8);
	memcpy(key_part, &temp_str[6], 6);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		key[3 + i] = temp[2 - i];

	if(key_length == 8)
    {
        memset(key_part, 0, 8);
        memcpy(key_part, &temp_str[12], 4);
        key_nr = strtol(key_part, NULL, 16);
        memcpy(temp, (void *) &key_nr, 2);
        for (i = 0; i < 2; i++)
            key[6 + i] = temp[1 - i];
        return;
    }
	else
    {
        memset(key_part, 0, 8);
        memcpy(key_part, &temp_str[12], 6);
        key_nr = strtol(key_part, NULL, 16);
        memcpy(temp, (void *) &key_nr, 3);
        for (i = 0; i < 3; i++)
            key[6 + i] = temp[2 - i];
    }

	memset(key_part, 0, 8);
	memcpy(key_part, &temp_str[18], 6);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		key[9 + i] = temp[2 - i];

	memset(key_part, 0, 8);
	memcpy(key_part, &temp_str[24], 6);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		key[12 + i] = temp[2 - i];

	memset(key_part, 0, 8);
	memcpy(key_part, &temp_str[30], 2);
	key_nr = strtol(key_part, NULL, 16);
	key[15] = key_nr;

	if(key_length == 16)
        return;

    memset(key_part, 0, 8);
	memcpy(key_part, &temp_str[32], 6);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		key[16 + i] = temp[2 - i];

	memset(key_part, 0, 8);
	memcpy(key_part, &temp_str[38], 6);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 3; i++)
		key[19 + i] = temp[2 - i];

	memset(key_part, 0, 8);
    memcpy(key_part, &temp_str[44], 4);
	key_nr = strtol(key_part, NULL, 16);
	memcpy(temp, (void *) &key_nr, 3);
	for (i = 0; i < 2; i++)
		key[22 + i] = temp[1 - i];
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

     case CARD_CRYPTO_ERROR:
        strcat(retstr, "CARD_CRYPTO_ERROR");
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
			   "  (c) - Store key into reader\n"
			   "  (d) - Change key\n"
			   "  (e) - Change key settings\n"
			   "  (f) - Get key settings\n"
			   "  (g) - Make application\n"
			   "  (h) - Delete application\n"
			   "  (j) - Make file\n"
			   "  (k) - Delete file\n"
			   "  (l) - Write Std file or record\n"
			   "  (m) - Read Std file of records\n"
			   "  (n) - Read Value file\n"
			   "  (o) - Increase Value file\n"
			   "  (p) - Decrease Value file\n"
			   "  (r) - Clear Record file\n"
			   "  (s) - Get application IDs\n"
			   "  (t) - Change config parameters\n");
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
            ChangeKey();
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
            ClearRecord();
            break;
        case 's':
            GetApplicationIds();
            break;
        case 't':
            ChangeSettings();
            break;

		default:
			usage();
			break;
	}
	printf(" --------------------------------------------------\n");
}
//------------------------------------------------------------------------------
UFR_STATUS reader_open_ex()
{

UFR_STATUS status;

        std::string portNameStr = "";
        std::string portInterfaceStr = "";
        std::string argumentStr = "";
        uint32_t port_interface = 0;
        uint32_t reader_type = 0;

        std::cout << "Enter reader type: " << std::endl;
        scanf("%d%*c", &reader_type);
        fflush(stdin);
        std::cout << "Enter port name: " << std::endl;
        getline(std::cin, portNameStr);
		fflush(stdin);
        std::cout << "Enter port interface: " << std::endl;
        std::cin >> portInterfaceStr;
		fflush(stdin);
        std::cout << "Enter argument: " << std::endl;
        getline(std::cin, argumentStr);
		fflush(stdin);

        if(portInterfaceStr == "U") {
                port_interface = 85;
            }
        else if(portInterfaceStr == "T") {
                port_interface = 84;
            }
        else {
                port_interface = atoi(portInterfaceStr.c_str());
            }
            char port_name[portNameStr.length() + 1];
            char arg[argumentStr.length() + 1];
            memset(port_name,0, sizeof(port_name));
            memset(arg,0, sizeof(arg));

            strcpy(port_name, portNameStr.c_str());
            strcpy(arg,argumentStr.c_str());

             status = ReaderOpenEx(reader_type, port_name, port_interface, (void *)arg);

        return status;
}


int main()
{
	char key;
    int mode = 0;
	UFR_STATUS status;
    std::cout << "Select reader opening mode:" << std::endl;
    std::cout <<" (1) - Simple Reader Open" << std::endl;
    std::cout <<" (2) - Advanced Reader Open" << std::endl;
    std::cin >> mode;
    if (mode == 1){
        status = ReaderOpen();

    } else if (mode == 2) {
        status = reader_open_ex();

    } else {
        std::cout << "Invalid input. Press any key to quit the application..." << std::endl;
        getchar();
        return 0;
}
	if (status != UFR_OK)
	{
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

	unsigned char data[11];
	unsigned char data_length;

	unsigned long aid;
	unsigned char aid_key_nr;
	unsigned char key_ext[24];
	unsigned char key_nr = 0;

	memset(data, 0, 11);

    aid = strtol(settings[1].c_str(),NULL,16);

    aid_key_nr = stoul(settings[2], nullptr, 10);

    if (internal_key == false)
    {
        if (!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4], nullptr, 10);
    }

    if (internal_key == true)
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_GetDesfireUid_aes(key_nr, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_GetDesfireUid_des(key_nr, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_GetDesfireUid_2k3des(key_nr, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
        else
            status = uFR_int_GetDesfireUid_3k3des(key_nr, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
    }
    else
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_GetDesfireUid_aes_PK(key_ext, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_GetDesfireUid_des_PK(key_ext, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_GetDesfireUid_2k3des_PK(key_ext, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
        else
            status = uFR_int_GetDesfireUid_3k3des_PK(key_ext, aid, aid_key_nr, data, &data_length, &card_status, &exec_time);
    }

    if (status)
    {
        std::cout << "uFR_int_GetDesfireUID(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_nr = 0;
	unsigned char key_ext[24];


    if (internal_key == false)
    {
        if (!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4], nullptr, 10);
    }

   if (internal_key == true)
	{
		if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireFormatCard_aes(key_nr, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireFormatCard_des(key_nr, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireFormatCard_2k3des(key_nr, &card_status, &exec_time);
        else
            status = uFR_int_DesfireFormatCard_3k3des(key_nr, &card_status, &exec_time);
	}
	else
	{
		if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireFormatCard_aes_PK(key_ext, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireFormatCard_des_PK(key_ext, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireFormatCard_2k3des_PK(key_ext, &card_status, &exec_time);
        else
            status = uFR_int_DesfireFormatCard_3k3des_PK(key_ext, &card_status, &exec_time);
	}

	if (status)
    {
        std::cout << "Command was not received || Confirmation was not received" << std::endl;
        std::cout << "uFR_int_DesfireFormatCard(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_nr = 0;
	unsigned char key_ext[24];

	if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4], nullptr, 10);
    }

    if (internal_key == true)
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireSetConfiguration_aes(key_nr, 1, 0, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireSetConfiguration_des(key_nr, 1, 0, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireSetConfiguration_2k3des(key_nr, 1, 0, &card_status, &exec_time);
        else
            status = uFR_int_DesfireSetConfiguration_3k3des(key_nr, 1, 0, &card_status, &exec_time);
    }
	else
	{
		if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireSetConfiguration_aes_PK(key_ext, 1, 0, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireSetConfiguration_des_PK(key_ext, 1, 0, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireSetConfiguration_2k3des_PK(key_ext, 1, 0, &card_status, &exec_time);
        else
            status = uFR_int_DesfireSetConfiguration_3k3des_PK(key_ext, 1, 0, &card_status, &exec_time);
	}

	if (status)
    {
        std::cout << "Communication error" << std::endl;
        std::cout <<"uFR_int_DesfireSetConfiguration(): " << UFR_Status2String(status) << std::endl;
        return;
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
    std::cin >> tx_speed;

    std::cout << "Enter value for setting receive rate (rx speed)" << std::endl;
    std::cout << "0 - 106 kbps" << std::endl;
    std::cout << "1 - 212 kbps" << std::endl;
    std::cout << "2 - 424 kbps" << std::endl;
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

    unsigned char key[24];

    int choice;
    unsigned char key_type = AES_KEY_TYPE;

    std::cout << " Enter key type " << std::endl;
    std::cout << " 1 - DES (8 bytes)" << std::endl;
    std::cout << " 2 - 2K3DES (16 bytes)" << std::endl;
    std::cout << " 3 - 3K3DES (24 bytes)" << std::endl;
    std::cout << " 4 - AES (16 bytes)" << std::endl;

    scanf("%d", &choice);

    switch(choice)
    {
    case 1:
        key_type = DES_KEY_TYPE;
        break;
    case 2:
        key_type = DES2K_KEY_TYPE;
        break;
    case 3:
        key_type = DES3K_KEY_TYPE;
        break;
    case 4:
        key_type = AES_KEY_TYPE;
        break;
    }

	std::string key_str;

    switch(key_type)
    {
    case DES_KEY_TYPE:
        std::cout << "Enter DES key (8 bytes): " << std::endl;
        break;
    case DES2K_KEY_TYPE:
        std::cout << "Enter 2K3DES key (16 bytes): " << std::endl;
        break;
    case DES3K_KEY_TYPE:
        std::cout << "Two key fields will be occupied !!!" << std::endl;
        std::cout << "Enter 3K3DES key (24 bytes): " << std::endl;
        break;
    case AES_KEY_TYPE:
        std::cout << "Enter AES key (16 bytes): " << std::endl;
        break;
    }

    std::cin >> key_str;

    if(key_type == AES_KEY_TYPE || key_type == DES2K_KEY_TYPE)
    {
        if (key_str.length() != 32)
        {
            std::cout << "Key must be 16 bytes long" << std::endl;
            return;
        }
        else
        {
            Convert(key_str, key);
        }
    }
    else if(key_type == DES_KEY_TYPE)
    {
        if (key_str.length() != 16)
        {
            std::cout << "Key must be 8 bytes long" << std::endl;
            return;
        }
        else
        {
            Convert(key_str, key);
        }
    }
    else
    {
        if (key_str.length() != 48)
        {
            std::cout << "Key must be 24 bytes long" << std::endl;
            return;
        }
        else
        {
            Convert(key_str, key);
        }
    }

    std::cout << "Input reader internal key number (0-15):" << std::endl;

    scanf("%d", &key_no);

    status = uFR_int_DesfireWriteKey(key_no, key, key_type);

    if (status)
    {
        std::cout << "Error: " << std::endl;
        if (status == UFR_KEYS_LOCKED)
        {
            std::cout << "Internal keys are locked" << std::endl;
        }
        std::cout << "Function status: " << UFR_Status2String(status) << std::endl;
    }
    else
        std::cout << "Operation completed. Status is " << UFR_Status2String(status) << std::endl;
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0, set_temp = 0;
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
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }

    if (internal_key == true)
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireChangeKeySettings_aes(key_nr, aid, setting, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireChangeKeySettings_des(key_nr, aid, setting, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireChangeKeySettings_2k3des(key_nr, aid, setting, &card_status, &exec_time);
        else
            status = uFR_int_DesfireChangeKeySettings_3k3des(key_nr, aid, setting, &card_status, &exec_time);
    }
    else
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireChangeKeySettings_aes_PK(key_ext, aid, setting, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireChangeKeySettings_des_PK(key_ext, aid, setting, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireChangeKeySettings_2k3des_PK(key_ext, aid, setting, &card_status, &exec_time);
        else
            status = uFR_int_DesfireChangeKeySettings_3k3des_PK(key_ext, aid, setting, &card_status, &exec_time);
    }

    if (status)
    {
        std::cout << std::endl << "Communication error" << std::endl;
        std::cout << "uFR_int_DesfireChangeKeySettings(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;


	aid = strtol(settings[1].c_str(),NULL,16);

    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }

    if(internal_key == true)
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireGetKeySettings_aes(key_nr, aid, &setting, &max_key_no, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireGetKeySettings_des(key_nr, aid, &setting, &max_key_no, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireGetKeySettings_2k3des(key_nr, aid, &setting, &max_key_no, &card_status, &exec_time);
        else
            status = uFR_int_DesfireGetKeySettings_3k3des(key_nr, aid, &setting, &max_key_no, &card_status, &exec_time);
	}
	else
	{
		if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireGetKeySettings_aes_PK(key_ext, aid, &setting, &max_key_no, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireGetKeySettings_des_PK(key_ext, aid, &setting, &max_key_no, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireGetKeySettings_2k3des_PK(key_ext, aid, &setting, &max_key_no, &card_status, &exec_time);
        else
            status = uFR_int_DesfireGetKeySettings_3k3des_PK(key_ext, aid, &setting, &max_key_no, &card_status, &exec_time);
	}

	if(status)
    {
        std::cout << "Communication error" <<std::endl;
        std::cout << "uFR_int_DesfireGetKeySettings(): " << UFR_Status2String(status) << std::endl;
        return;
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

void ChangeKey()
{
    UFR_STATUS status;

    unsigned short card_status;
	unsigned short exec_time;

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char aid_key_nr_auth;
	int aid_key_nr;
	unsigned char new_key[24];
	unsigned char old_key[24];
	unsigned char new_key_type = 0;
	unsigned char new_key_nr = 0;
	unsigned char old_key_nr = 0;
	std::string old_key_str = "", new_key_str = "";
	int key_int;

	aid = strtol(settings[1].c_str(),NULL,16);

	if(aid == 0x000000)
    {
        //Master PICC key
        std::cout << "MASTER KEY CHANGE !!!" << std::endl;
        aid_key_nr_auth = 0;
        aid_key_nr = 0;

        std::cout << " Enter new key type " << std::endl;
        std::cout << " 1 - DES (8 bytes)" << std::endl;
        std::cout << " 2 - 2K3DES (16 bytes)" << std::endl;
        std::cout << " 3 - 3K3DES (24 bytes)" << std::endl;
        std::cout << " 4 - AES (16 bytes)" << std::endl;

        int choice;
        scanf("%d",&choice);

        switch(choice)
        {
        case 1:
            new_key_type = DES_KEY_TYPE;
            break;
        case 2:
            new_key_type = DES2K_KEY_TYPE;
            break;
        case 3:
            new_key_type = DES3K_KEY_TYPE;
            break;
        case 4:
            new_key_type = AES_KEY_TYPE;
            break;
        }

        if(internal_key)
        {
            printf("Input new internal key number: ");
            scanf("%d", &key_int);
            new_key_nr = key_int;
        }
        else
        {
            if(new_key_type == DES_KEY_TYPE)
                std::cout << "Input new DES key (8 bytes): " << std::endl;
            else if(new_key_type == DES2K_KEY_TYPE)
                std::cout << "Input new 2K3DES key (16 bytes): " << std::endl;
            else if(new_key_type == DES3K_KEY_TYPE)
                std::cout << "Input new 3K3DES key (24 bytes): " << std::endl;
            else if(new_key_type == AES_KEY_TYPE)
                std::cout << "Input new AES key (16 bytes): " << std::endl;

            std::cin >> new_key_str;

            if(new_key_type == AES_KEY_TYPE || new_key_type == DES2K_KEY_TYPE)
            {
                if (new_key_str.length() != 32)
                {
                    std::cout << "Key must be 16 bytes long" << std::endl;
                    return;
                }
                else
                {
                    Convert(new_key_str, new_key);
                }
            }
            else if(new_key_type == DES_KEY_TYPE)
            {
                if (new_key_str.length() != 16)
                {
                    std::cout << "Key must be 8 bytes long" << std::endl;
                    return;
                }
                else
                {
                    Convert(new_key_str, new_key);
                }
            }
            else
            {
                if (new_key_str.length() != 48)
                {
                    std::cout << "Key must be 24 bytes long" << std::endl;
                    return;
                }
                else
                {
                    Convert(new_key_str, new_key);
                }
            }


        }
    }
	else
    {
        //Application key
        aid_key_nr_auth = stoul(settings[2],nullptr,10);

        printf("Input key number to change: ");
        scanf("%d", &aid_key_nr);

        if(aid_key_nr != aid_key_nr_auth)
        {
            //different key enter old key
            if(internal_key)
            {
                printf("Input old internal key number: ");
                scanf("%d", &key_int);
                old_key_nr = key_int;
            }
            else
            {
                if(key_type_nr == DES_KEY_TYPE)
                    std::cout << "Input old DES key (8 bytes): " << std::endl;
                else if(key_type_nr == DES2K_KEY_TYPE)
                    std::cout << "Input old 2K3DES key (16 bytes): " << std::endl;
                else if(key_type_nr == DES3K_KEY_TYPE)
                    std::cout << "Input old 3K3DES key (24 bytes): " << std::endl;
                else if(key_type_nr == AES_KEY_TYPE)
                    std::cout << "Input old AES key (16 bytes): " << std::endl;

                std::cin >> old_key_str;

                if(key_type_nr == AES_KEY_TYPE || key_type_nr == DES2K_KEY_TYPE)
                {
                    if (old_key_str.length() != 32)
                    {
                        std::cout << "Key must be 16 bytes long" << std::endl;
                        return;
                    }
                    else
                    {
                        Convert(old_key_str, old_key);
                    }
                }
                else if(key_type_nr == DES_KEY_TYPE)
                {
                    if (old_key_str.length() != 16)
                    {
                        std::cout << "Key must be 8 bytes long" << std::endl;
                        return;
                    }
                    else
                    {
                        Convert(old_key_str, old_key);
                    }
                }
                else
                {
                    if (old_key_str.length() != 48)
                    {
                        std::cout << "Key must be 24 bytes long" << std::endl;
                        return;
                    }
                    else
                    {
                        Convert(old_key_str, old_key);
                    }
                }
            }

        }

        if(key_type_nr == DES_KEY_TYPE || key_type_nr == DES2K_KEY_TYPE)
        {
            std::cout << " Enter new key type " << std::endl;
            std::cout << " 1 - DES (8 bytes)" << std::endl;
            std::cout << " 2 - 2K3DES (16 bytes)" << std::endl;

            int choice;
            scanf("%d",&choice);

            switch(choice)
            {
            case 1:
                new_key_type = DES_KEY_TYPE;
                break;
            case 2:
                new_key_type = DES2K_KEY_TYPE;
                break;
            }
        }
        else
            new_key_type = key_type_nr;

        if(internal_key)
        {
            printf("Input new internal key number: ");
            scanf("%d", &key_int);
            new_key_nr = key_int;
        }
        else
        {
            if(new_key_type == DES_KEY_TYPE)
                std::cout << "Input new DES key (8 bytes): " << std::endl;
            else if(new_key_type == DES2K_KEY_TYPE)
                std::cout << "Input new 2K3DES key (16 bytes): " << std::endl;
            else if(new_key_type == DES3K_KEY_TYPE)
                std::cout << "Input new 3K3DES key (24 bytes): " << std::endl;
            else if(new_key_type == AES_KEY_TYPE)
                std::cout << "Input new AES key (16 bytes): " << std::endl;

            std::cin >> new_key_str;

            if(new_key_type == AES_KEY_TYPE || new_key_type == DES2K_KEY_TYPE)
            {
                if (new_key_str.length() != 32)
                {
                    std::cout << "Key must be 16 bytes long" << std::endl;
                    return;
                }
                else
                {
                    Convert(new_key_str, new_key);
                }
            }
            else if(new_key_type == DES_KEY_TYPE)
            {
                if (new_key_str.length() != 16)
                {
                    std::cout << "Key must be 8 bytes long" << std::endl;
                    return;
                }
                else
                {
                    Convert(new_key_str, new_key);
                }
            }
            else
            {
                if (new_key_str.length() != 48)
                {
                    std::cout << "Key must be 24 bytes long" << std::endl;
                    return;
                }
                else
                {
                    Convert(new_key_str, new_key);
                }
            }
        }
    }

    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }

    if (internal_key == true)
    {
        if(key_type_nr == AES_KEY_TYPE)
        {
            if(new_key_type == AES_KEY_TYPE)
                status = uFR_int_DesfireChangeAesKey_aes(key_nr, aid, aid_key_nr_auth, new_key_nr, aid_key_nr, old_key_nr, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey(key_nr, AES_KEY_TYPE, new_key_nr, new_key_type, &card_status, &exec_time);
            }
        }
        else if(key_type_nr == DES_KEY_TYPE)
        {
            if(new_key_type == DES_KEY_TYPE)
                status = uFR_int_DesfireChangeDesKey_des(key_nr, aid, aid_key_nr_auth, new_key_nr, aid_key_nr, old_key_nr, &card_status, &exec_time);
            else if(new_key_type == DES2K_KEY_TYPE)
                status = uFR_int_DesfireChange2K3DesKey_des(key_nr, aid, aid_key_nr_auth, new_key_nr, aid_key_nr, old_key_nr, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey(key_nr, DES_KEY_TYPE, new_key_nr, new_key_type, &card_status, &exec_time);
            }
        }
        else if(key_type_nr == DES2K_KEY_TYPE)
        {
            if(new_key_type == DES2K_KEY_TYPE)
                status = uFR_int_DesfireChange2K3DesKey_2k3des(key_nr, aid, aid_key_nr_auth, new_key_nr, aid_key_nr, old_key_nr, &card_status, &exec_time);
            else if(new_key_type == DES_KEY_TYPE)
                status = uFR_int_DesfireChangeDesKey_2k3des(key_nr, aid, aid_key_nr_auth, new_key_nr, aid_key_nr, old_key_nr, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey(key_nr, DES2K_KEY_TYPE, new_key_nr, new_key_type, &card_status, &exec_time);
            }
        }
        else
        {
            if(new_key_type == DES3K_KEY_TYPE)
                status = uFR_int_DesfireChange3K3DesKey_3k3des(key_nr, aid, aid_key_nr_auth, new_key_nr, aid_key_nr, old_key_nr, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey(key_nr, DES3K_KEY_TYPE, new_key_nr, new_key_type, &card_status, &exec_time);
            }
        }

    }
	else
	{
		if(key_type_nr == AES_KEY_TYPE)
        {
            if(new_key_type == AES_KEY_TYPE)
                status = uFR_int_DesfireChangeAesKey_aes_PK(key_ext, aid, aid_key_nr_auth, new_key, aid_key_nr, old_key, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey_PK(key_ext, AES_KEY_TYPE, new_key, new_key_type, &card_status, &exec_time);
            }
        }
        else if(key_type_nr == DES_KEY_TYPE)
        {
            if(new_key_type == DES_KEY_TYPE)
                status = uFR_int_DesfireChangeDesKey_des_PK(key_ext, aid, aid_key_nr_auth, new_key, aid_key_nr, old_key, &card_status, &exec_time);
            else if(new_key_type == DES2K_KEY_TYPE)
                status = uFR_int_DesfireChange2K3DesKey_des_PK(key_ext, aid, aid_key_nr_auth, new_key, aid_key_nr, old_key, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey_PK(key_ext, DES_KEY_TYPE, new_key, new_key_type, &card_status, &exec_time);
            }
        }

        else if(key_type_nr == DES2K_KEY_TYPE)
        {
            if(new_key_type == DES2K_KEY_TYPE)
                status = uFR_int_DesfireChange2K3DesKey_2k3des_PK(key_ext, aid, aid_key_nr_auth, new_key, aid_key_nr, old_key, &card_status, &exec_time);
            else if(new_key_type == DES_KEY_TYPE)
                status = uFR_int_DesfireChangeDesKey_2k3des_PK(key_ext, aid, aid_key_nr_auth, new_key, aid_key_nr, old_key, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey_PK(key_ext, DES2K_KEY_TYPE, new_key, new_key_type, &card_status, &exec_time);
            }
        }
        else
        {
            if(new_key_type == DES3K_KEY_TYPE)
                status = uFR_int_DesfireChange3K3DesKey_3k3des_PK(key_ext, aid, aid_key_nr_auth, new_key, aid_key_nr, old_key, &card_status, &exec_time);
            else
            {
                //master key
                status = uFR_int_DesfireChangeMasterKey_PK(key_ext, DES3K_KEY_TYPE, new_key, new_key_type, &card_status, &exec_time);
            }
        }
	}

	if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireChangeAesKey(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned char application_type = AES_KEY_TYPE;

	int choice = 0;

	std::cout << " Choose application key type:" << std::endl;
	std::cout << " 1 - DES" << std::endl;
	std::cout << " 2 - 3K3DES" << std::endl;
	std::cout << " 3 - AES" << std::endl;

	scanf("%d",&choice);

	switch(choice)
	{
    case 1:
        application_type = DES_KEY_TYPE;
        break;
    case 2:
        application_type = DES3K_KEY_TYPE;
        break;
    case 3:
        application_type = AES_KEY_TYPE;
        break;
	}

    printf("Input  AID tnumber (3 bytes hex): ");
    std::cin >> str_aid;

    aid = strtol(str_aid.c_str(), NULL,16);

    std::cout << std::endl;
    printf("Input maximal key number: (1 - 14)");
    scanf("%d", &max_key_no);

    std::cout << " Choose application master key settings:" << std::endl;
    std::cout << " 0 - No settings" << std::endl;
    std::cout << " 1 - Settings not changeable anymore" << std::endl;
    std::cout << " 2 - Create or delete file with master key authentication" << std::endl;
    std::cout << " 3 - Master key not changeable anymore" << std::endl;
    std::cout << " 4 - Settings not changeable anymore and create or delete file with master key" << std::endl;
    std::cout << " 5 - Settings and master key not changeable anymore" << std::endl;
    std::cout << " 6 - Create and delete file with master key and master key is not changeable anymore" << std::endl;
    std::cout << " 7 - Settings not changeable anymore, create or delete file with master key,";
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
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4], nullptr, 10);
    }

    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            if(key_type_nr == AES_KEY_TYPE)
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_aes(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_aes(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_aes(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
            }
            else if(key_type_nr == DES_KEY_TYPE)
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
            }
            else if(key_type_nr == DES2K_KEY_TYPE)
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_2k3des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_2k3des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_2k3des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
            }
            else
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_3k3des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_3k3des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_3k3des(key_nr, aid, setting, max_key_no, &card_status, &exec_time);
            }
        }
        else
        {
            if(key_type_nr == AES_KEY_TYPE)
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_aes_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_aes_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_aes_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
            }
            else if(key_type_nr == DES_KEY_TYPE)
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
            }
            else if(key_type_nr == DES2K_KEY_TYPE)
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_2k3des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_2k3des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_2k3des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
            }
            else
            {
                if(application_type == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateDesApplication_3k3des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else if(application_type == DES3K_KEY_TYPE)
                    status = uFR_int_DesfireCreate3k3desApplication_3k3des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateAesApplication_3k3des_PK(key_ext, aid, setting, max_key_no, &card_status, &exec_time);
            }
        }
    }
    else
    {
        if(application_type == DES_KEY_TYPE)
            status = uFR_int_DesfireCreateDesApplication_no_auth(aid, setting, max_key_no, &card_status, &exec_time);
        else if(application_type == DES3K_KEY_TYPE)
            status = uFR_int_DesfireCreate3k3desApplication_no_auth(aid, setting, max_key_no, &card_status, &exec_time);
        else
            status = uFR_int_DesfireCreateAesApplication_no_auth(aid, setting, max_key_no, &card_status, &exec_time);
    }

    if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireCreateApplication(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0;

    std::string str_aid = "";

    printf("Input AID to delete (3 bytes hex): ");
    std::cin >> str_aid;

    aid = strtol(str_aid.c_str(), NULL,16);

    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    } else
    {
        key_nr = stoul(settings[4], nullptr,10);
    }


    if (internal_key == true)
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireDeleteApplication_aes(key_nr, aid, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireDeleteApplication_des(key_nr, aid, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireDeleteApplication_2k3des(key_nr, aid, &card_status, &exec_time);
        else
            status = uFR_int_DesfireDeleteApplication_3k3des(key_nr, aid, &card_status, &exec_time);
    }
    else
    {
        if(key_type_nr == AES_KEY_TYPE)
            status = uFR_int_DesfireDeleteApplication_aes_PK(key_ext, aid, &card_status, &exec_time);
        else if(key_type_nr == DES_KEY_TYPE)
            status = uFR_int_DesfireDeleteApplication_des_PK(key_ext, aid, &card_status, &exec_time);
        else if(key_type_nr == DES2K_KEY_TYPE)
            status = uFR_int_DesfireDeleteApplication_2k3des_PK(key_ext, aid, &card_status, &exec_time);
        else
            status = uFR_int_DesfireDeleteApplication_3k3des_PK(key_ext, aid, &card_status, &exec_time);
    }

    if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireDeleteApplication(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
    int lower_limit = 0, upper_limit = 0, value_value = 0;
    unsigned char limited_credit_enabled = 0;
    int limited_credit_choice = 0, free_get_choice = 0;
    int comm_choice = 0, file_choice = 0;

    int record_size, max_rec_nr;

    aid = strtol(settings[1].c_str(), NULL,16);

    printf("Input File ID: ");
    scanf("%d", &file_id);


    printf("Choose communication mode:\n 1 - PLAIN.\n 2 - MACKED.\n 3 - ENCIPHERED.\n");
    scanf("%d", &comm_choice);

    printf("Choose file type:\n 1 - Standard data file\n 2 - Value file\n 3 - Linear record file\n 4 - Cyclic record file\n");
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
        if(!prepare_key(key_ext))
        {
            return;
        }
    } else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }

    if (file_choice == 1)
    {
        printf("Enter size of the file you wish to create: ");
        scanf("%d", &file_size);
    }
    else if (file_choice == 2)
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
    else if(file_choice == 3 || file_choice == 4)
    {
        printf("Enter size of record: ");
        scanf("%d", &record_size);

        printf("Enter maximal number of records: ");
        scanf("%d", &max_rec_nr);
    }

    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            if (file_choice == 1)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateStdDataFile_aes(key_nr, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateStdDataFile_des(key_nr, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateStdDataFile_2k3des(key_nr, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateStdDataFile_3k3des(key_nr, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
            }
            else if(file_choice == 2)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateValueFile_aes(key_nr, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateValueFile_des(key_nr, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateValueFile_2k3des(key_nr, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateValueFile_3k3des(key_nr, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
            }
            else if(file_choice == 3)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateLinearRecordFile_aes(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateLinearRecordFile_des(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateLinearRecordFile_2k3des(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateLinearRecordFile_3k3des(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
            }
            else
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateCyclicRecordFile_aes(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateCyclicRecordFile_des(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateCyclicRecordFile_2k3des(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateCyclicRecordFile_3k3des(key_nr, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
            }
        }
        else
        {
            if (file_choice == 1)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateStdDataFile_aes_PK(key_ext, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateStdDataFile_des_PK(key_ext, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateStdDataFile_2k3des_PK(key_ext, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateStdDataFile_3k3des_PK(key_ext, aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
            }
            else if (file_choice == 2)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateValueFile_aes_PK(key_ext, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateValueFile_des_PK(key_ext, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateValueFile_2k3des_PK(key_ext, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateValueFile_3k3des_PK(key_ext, aid, file_id,
                             lower_limit, upper_limit, value_value, limited_credit_enabled,
                             read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                             communication_settings, &card_status, &exec_time);
            }
            else if (file_choice == 3)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateLinearRecordFile_aes_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateLinearRecordFile_des_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateLinearRecordFile_2k3des_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateLinearRecordFile_3k3des_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
            }
            else
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireCreateCyclicRecordFile_aes_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireCreateCyclicRecordFile_des_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireCreateCyclicRecordFile_2k3des_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireCreateCyclicRecordFile_3k3des_PK(key_ext, aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
            }
        }
    }
    else
    {
        if (file_choice == 1)
            status = uFR_int_DesfireCreateStdDataFile_no_auth(aid, file_id, file_size,read_key_nr,write_key_nr, read_write_key_nr, change_key_nr, communication_settings, &card_status, &exec_time);
        else if(file_choice == 2)
            status = uFR_int_DesfireCreateValueFile_no_auth(aid, file_id,
                         lower_limit, upper_limit, value_value, limited_credit_enabled,
                         read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                         communication_settings, &card_status, &exec_time);
        else if(file_choice == 3)
            status = uFR_int_DesfireCreateLinearRecordFile_no_auth(aid, file_id, record_size, max_rec_nr,
                                                    read_key_nr, write_key_nr, read_write_key_nr, change_key_nr,
                                                    communication_settings, &card_status, &exec_time);
        else
            status = uFR_int_DesfireCreateCyclicRecordFile_no_auth(aid, file_id, record_size, max_rec_nr,
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;

	printf("Enter file ID to delete: \n");
	scanf("%d", &file_no);

	if (internal_key == false)
    {
        if(!prepare_key(key_ext))
           {
               return;
           }
    } else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }

    aid = strtol(settings[1].c_str(), NULL, 16);


    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireDeleteFile_aes(key_nr, aid, file_no, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireDeleteFile_des(key_nr, aid, file_no, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireDeleteFile_2k3des(key_nr, aid, file_no, &card_status, &exec_time);
            else
                status = uFR_int_DesfireDeleteFile_3k3des(key_nr, aid, file_no, &card_status, &exec_time);
		}
		else
		{
			if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireDeleteFile_aes_PK(key_ext, aid, file_no, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireDeleteFile_des_PK(key_ext, aid, file_no, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireDeleteFile_2k3des_PK(key_ext, aid, file_no, &card_status, &exec_time);
            else
                status = uFR_int_DesfireDeleteFile_3k3des_PK(key_ext, aid, file_no, &card_status, &exec_time);
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
        return;
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
	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
    int file_value = 0, comm_choice = 0;


    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    } else
    {
        key_nr = stoul(settings[4],nullptr,10);
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
            if(key_type_nr == AES_KEY_TYPE)
                status =  uFR_int_DesfireReadValueFile_aes(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status =  uFR_int_DesfireReadValueFile_des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status =  uFR_int_DesfireReadValueFile_2k3des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
            else
                status =  uFR_int_DesfireReadValueFile_3k3des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
		}
		else
		{
			if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireReadValueFile_aes_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireReadValueFile_des_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireReadValueFile_2k3des_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, &file_value, &card_status, &exec_time);
            else
                status = uFR_int_DesfireReadValueFile_3k3des_PK(key_ext, aid, aid_key_nr, file_id,
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
        return;
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
	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
    int file_value = 0, comm_choice = 0;


    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    } else
    {
        key_nr = stoul(settings[4],nullptr,10);
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
            if(key_type_nr == AES_KEY_TYPE)
                status =  uFR_int_DesfireIncreaseValueFile_aes(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status =  uFR_int_DesfireIncreaseValueFile_des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status =  uFR_int_DesfireIncreaseValueFile_2k3des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else
                status =  uFR_int_DesfireIncreaseValueFile_3k3des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
		}
		else
		{
			if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireIncreaseValueFile_aes_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireIncreaseValueFile_des_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireIncreaseValueFile_2k3des_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else
                status = uFR_int_DesfireIncreaseValueFile_3k3des_PK(key_ext, aid, aid_key_nr, file_id,
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
        return;
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
	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
    int file_value = 0, comm_choice = 0;


    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    } else
    {
        key_nr = stoul(settings[4],nullptr,10);
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
            if(key_type_nr == AES_KEY_TYPE)
                status =  uFR_int_DesfireDecreaseValueFile_aes(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status =  uFR_int_DesfireDecreaseValueFile_des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status =  uFR_int_DesfireDecreaseValueFile_2k3des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else
                status =  uFR_int_DesfireDecreaseValueFile_3k3des(key_nr, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
		}
		else
		{
			if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireDecreaseValueFile_aes_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireDecreaseValueFile_des_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireDecreaseValueFile_2k3des_PK(key_ext, aid, aid_key_nr, file_id,
                              communication_settings, file_value, &card_status, &exec_time);
            else
                status = uFR_int_DesfireDecreaseValueFile_3k3des_PK(key_ext, aid, aid_key_nr, file_id,
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
        return;
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


	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;
	int file_type;


    FILE *stream;

    stream = fopen("write.txt", "rb");

    int curpos, length;

    curpos = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);

	printf("\nChoose file type:\n 1 - Standard data file\n 2 - Record file\n");
	scanf("%d", &file_type);

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
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4], nullptr, 10);
    }


    fread(file_data,file_length,1,stream);
    fclose(stream);

    if(master_authent_req == true)
    {
        if (internal_key == true)
        {
            if(file_type == 1)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireWriteStdDataFile_aes(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireWriteStdDataFile_des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireWriteStdDataFile_2k3des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireWriteStdDataFile_3k3des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
            }
            else
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireWriteRecord_aes(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireWriteRecord_des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireWriteRecord_2k3des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireWriteRecord_3k3des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
            }
		}
		else
		{
			if(file_type == 1)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireWriteStdDataFile_aes_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireWriteStdDataFile_des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireWriteStdDataFile_2k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireWriteStdDataFile_3k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
            }
            else
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireWriteRecord_aes_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireWriteRecord_des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireWriteRecord_2k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireWriteRecord_3k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings,file_data, &card_status, &exec_time);
            }
		}
    }
    else
    {
        if(file_type == 1)
            status = uFR_int_DesfireWriteStdDataFile_no_auth(aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
        else
            status = uFR_int_DesfireWriteRecord_no_auth(aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
    }

    if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireWriteStdDataFile(): " << UFR_Status2String(status) << std::endl;
        return;
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

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char file_id, aid_key_nr;

	int comm_choice;
	int file_type;
	int record_len, rec_nr;

	stream = fopen("read.txt", "wb");

	printf("\nChoose file type:\n 1 - Standard data file\n 2 - Record file\n");
	scanf("%d", &file_type);

	if(file_type == 1)
    {
        printf("Input file length to read: ");
        scanf("%d", &file_length);
    }
    else
    {
        printf("Enter record size: ");
        scanf("%d", &record_len);

        printf("Enter number of records: ");
        scanf("%d", &rec_nr);
    }


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
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }


    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            if(file_type == 1)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireReadStdDataFile_aes(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireReadStdDataFile_des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireReadStdDataFile_2k3des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireReadStdDataFile_3k3des(key_nr, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
            }
            else
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireReadRecords_aes(key_nr, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireReadRecords_des(key_nr, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireReadRecords_2k3des(key_nr, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireReadRecords_3k3des(key_nr, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
            }
		}
		else
		{
			if(file_type == 1)
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireReadStdDataFile_aes_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireReadStdDataFile_des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireReadStdDataFile_2k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireReadStdDataFile_3k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
            }
			else
            {
                if(key_type_nr == AES_KEY_TYPE)
                    status = uFR_int_DesfireReadRecords_aes_PK(key_ext, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES_KEY_TYPE)
                    status = uFR_int_DesfireReadRecords_des_PK(key_ext, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
                else if(key_type_nr == DES2K_KEY_TYPE)
                    status = uFR_int_DesfireReadRecords_2k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
                else
                    status = uFR_int_DesfireReadRecords_3k3des_PK(key_ext, aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
            }
		}
	}
	else
	{
		if(file_type == 1)
            status = uFR_int_DesfireReadStdDataFile_no_auth(aid, aid_key_nr, file_id, 0, file_length, communication_settings, file_data, &card_status, &exec_time);
        else
            status = uFR_int_DesfireReadRecords_no_auth(aid, aid_key_nr, file_id, 0, rec_nr, record_len, communication_settings, file_data, &card_status, &exec_time);
	}

	if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireReadStdDataFile(): " << UFR_Status2String(status) << std::endl;
        fclose(stream);
        return;
    }

    if(card_status == CARD_OPERATION_OK)
    {
        if(file_type == 2)
            file_length = record_len * rec_nr;
        fwrite(file_data, file_length, 1, stream);
        fclose(stream);
    }
    else
        fclose(stream);


    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

}

void print_settings()
{
    printf("1 - Change key\n");
    printf("2 - Change AID\n");
    printf("3 - Change AID key number\n");
    printf("4 - Change File ID\n");
    printf("5 - Change internal key number\n");
    printf("esc - Exit to main menu\n");
}

void ChangeSettings()
{
    printf("Current config:\n");
    if(key_type_nr == DES_KEY_TYPE)
        printf("\tDES key: ");
    else if(key_type_nr == DES2K_KEY_TYPE)
        printf("\t2K3DES key: ");
    else if(key_type_nr == DES3K_KEY_TYPE)
        printf("\t3K3DES key: ");
    else
        printf("\tAES key: ");
    printf("%s\n", settings[0].c_str());
    printf("\tAID: %s\n", settings[1].c_str());
    printf("\tAID key number auth: %s\n", settings[2].c_str());
    printf("\tFile ID: %s\n", settings[3].c_str());
    printf("\tInternal key nr: %s\n", settings[4].c_str());

    printf("1 - Change key\n");
    printf("2 - Change AID\n");
    printf("3 - Change AID key number\n");
    printf("4 - Change File ID\n");
    printf("5 - Change internal key number\n");
    printf("esc - Exit to main menu\n");


    char key;
    std::string new_key = "";
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
                std::cout << " Enter key type " << std::endl;
                std::cout << " 1 - DES (8 bytes)" << std::endl;
                std::cout << " 2 - 2K3DES (16 bytes)" << std::endl;
                std::cout << " 3 - 3K3DES (24 bytes)" << std::endl;
                std::cout << " 4 - AES (16 bytes)" << std::endl;

                int choice;
                scanf("%d",&choice);

                switch(choice)
                {
                case 1:
                    std::cout << "Input new DES key (8 bytes):" << std::endl;
                    std::cin >> new_key;
                    if (new_key.length() != 16)
                    {
                        std::cout << "DES key must be 8 bytes long" << std::endl;
                        return;
                    }
                    key_type_nr = DES_KEY_TYPE;
                    break;
                case 2:
                    std::cout << "Input new 2K3DES key (16 bytes):" << std::endl;
                    std::cin >> new_key;
                    if (new_key.length() != 32)
                    {
                        std::cout << "2K3DES key must be 16 bytes long" << std::endl;
                        return;
                    }
                    key_type_nr = DES2K_KEY_TYPE;
                    break;
                case 3:
                    std::cout << "Input new 3K3DES key (24 bytes):" << std::endl;
                    std::cin >> new_key;
                    if (new_key.length() != 48)
                    {
                        std::cout << "3K3DES key must be 24 bytes long" << std::endl;
                        return;
                    }
                    key_type_nr = DES3K_KEY_TYPE;
                    break;
                case 4:
                    std::cout << "Input new AES key (16 bytes):" << std::endl;
                    std::cin >> new_key;
                    if (new_key.length() != 32)
                    {
                        std::cout << "aes key must be 16 bytes long" << std::endl;
                        return;
                    }
                    key_type_nr = AES_KEY_TYPE;
                    break;
                default:
                    std::cout << "wrong choice" << std::endl;
                    return;
                    break;
                }

                settings[0] = new_key;
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

   std::ofstream myfile("config.txt");

   if (myfile.is_open())
   {
       std::string key_type_str;
       switch(key_type_nr)
       {
       case DES_KEY_TYPE:
            key_type_str = "DES key: ";
            break;
       case DES2K_KEY_TYPE:
            key_type_str = "2K3DES key: ";
            break;
       case DES3K_KEY_TYPE:
            key_type_str = "3K3DES key: ";
            break;
       case AES_KEY_TYPE:
            key_type_str = "AES key: ";
            break;
       }

       myfile << key_type_str << settings[0] << std::endl;
       myfile << "AID 3 bytes hex: " << settings[1] << std::endl;
       myfile << "AID key number for auth: " << settings[2] << std::endl;
       myfile << "File ID: " << settings[3] << std::endl;
       myfile << "Internal key number: " << settings[4] << std::endl;
   } else
   {
       std::cout << "Couldn't write new settings to config.txt";
   }
}

void GetApplicationIds(void)
{
    UFR_STATUS status;
	unsigned short card_status;
	unsigned short exec_time;

	unsigned char key_ext[24];
	unsigned char key_nr = 0;

	uint32_t app_ids[100];
	unsigned char app_ids_nr;

	memset(app_ids, 0, 100);

	if (internal_key == false)
    {
        if(!prepare_key(key_ext))
       {
           return;
       }
    }
    else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }

    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireGetApplicationIds_aes(key_nr, app_ids, &app_ids_nr, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireGetApplicationIds_des(key_nr, app_ids, &app_ids_nr, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireGetApplicationIds_2k3des(key_nr, app_ids, &app_ids_nr, &card_status, &exec_time);
            else
                status = uFR_int_DesfireGetApplicationIds_3k3des(key_nr, app_ids, &app_ids_nr, &card_status, &exec_time);
        }
        else
        {
            if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireGetApplicationIds_aes_PK(key_ext, app_ids, &app_ids_nr, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireGetApplicationIds_des_PK(key_ext, app_ids, &app_ids_nr, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireGetApplicationIds_2k3des_PK(key_ext, app_ids, &app_ids_nr, &card_status, &exec_time);
            else
                status = uFR_int_DesfireGetApplicationIds_3k3des_PK(key_ext, app_ids, &app_ids_nr, &card_status, &exec_time);
        }
    }
    else
        status = uFR_int_DesfireGetApplicationIds_no_auth(app_ids, &app_ids_nr, &card_status, &exec_time);

    if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireGetApplicationIds(): " << UFR_Status2String(status) << std::endl;
        return;
    }

    if(card_status == CARD_OPERATION_OK)
    {
        printf("\nFound %d application IDs: \n", app_ids_nr);
        unsigned char i;
        for(i = 0; i < app_ids_nr; i++)
            printf("%0X\n", app_ids[i]);
    }
    else
       std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

}

void ClearRecord(void)
{
    UFR_STATUS status;
	unsigned short card_status = 0;
	unsigned short exec_time;

	unsigned char key_ext[24];
	unsigned char key_nr = 0;
	unsigned long aid;
	unsigned char file_id;

	aid = strtol(settings[1].c_str(),NULL,16);

    file_id = stoul(settings[3], nullptr, 10);

    if (internal_key == false)
    {
        if(!prepare_key(key_ext))
        {
            return;
        }
    }
    else
    {
        key_nr = stoul(settings[4],nullptr,10);
    }


    if (master_authent_req == true)
    {
        if (internal_key == true)
        {
            if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireClearRecordFile_aes(key_nr, aid, file_id, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireClearRecordFile_des(key_nr, aid, file_id, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireClearRecordFile_2k3des(key_nr, aid, file_id, &card_status, &exec_time);
            else
                status = uFR_int_DesfireClearRecordFile_3k3des(key_nr, aid, file_id, &card_status, &exec_time);
        }
        else
        {
            if(key_type_nr == AES_KEY_TYPE)
                status = uFR_int_DesfireClearRecordFile_aes_PK(key_ext, aid, file_id, &card_status, &exec_time);
            else if(key_type_nr == DES_KEY_TYPE)
                status = uFR_int_DesfireClearRecordFile_des_PK(key_ext, aid, file_id, &card_status, &exec_time);
            else if(key_type_nr == DES2K_KEY_TYPE)
                status = uFR_int_DesfireClearRecordFile_2k3des_PK(key_ext, aid, file_id, &card_status, &exec_time);
            else
                status = uFR_int_DesfireClearRecordFile_3k3des_PK(key_ext, aid, file_id, &card_status, &exec_time);
        }
    }
    else
       status = uFR_int_DesfireClearRecordFile_no_auth(aid, file_id, &card_status, &exec_time);

    if (status)
    {
        std::cout << std::endl << "Communication error";
        std::cout << std::endl << "uFR_int_DesfireClearRecordFile(): " << UFR_Status2String(status) << std::endl;
        return;
    }

    std::cout << "Operation completed\n";

    std::cout << "Function status is: " << UFR_Status2String(status) << std::endl;

    std::cout << "Card status is: " << switch_card_status(card_status) << std::endl;

    std::cout << "Execution time: " << exec_time << " ms" << std::endl;

    if(card_status == CARD_OPERATION_OK)
        std::cout << "All records deleted" << std::endl;


}
