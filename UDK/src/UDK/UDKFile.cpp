#include "UDKFile.h"
#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(ArrayOfNode);

int FDtoBlockSize(int FD)
{
	int block_size = 0;
#if defined(__linux__)
	ioctl(FD, BLKSSZGET, &block_size);
#elif defined (__WXMAC__)
	ioctl(FD, DKIOCGETBLOCKSIZE, &block_size);
#elif defined (BSD)
	ioctl(FD, DIOCGSECTORSIZE, &block_size);
#elif defined (__WXMSW__)
	struct stat* sbufptr = new struct stat;
	fstat(FD, sbufptr);
	if (sbufptr->st_mode == 0)
	{
		//Enable block size detection code on windows targets,
		DWORD dwResult;
		DISK_GEOMETRY driveInfo;
		DeviceIoControl((void*)_get_osfhandle(FD), IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof(driveInfo), &dwResult, NULL);
		block_size = driveInfo.BytesPerSector;
	}
#endif
	return block_size;
}

uint64_t FDtoBlockCount(int FD)
{
	uint64_t block_count = 0;
#if defined(__linux__)
	ioctl(FD, BLKGETSIZE64, &block_count);
	block_count /= FDtoBlockSize(FD);
#elif defined (__WXMAC__)
	ioctl(FD, DKIOCGETBLOCKCOUNT, &block_count);
#elif defined (BSD)
	ioctl(FD, DIOCGMEDIASIZE, &block_count);
	block_count /= FDtoBlockSize(FD);
#elif defined (__WXMSW__)
	DWORD dwResult;
	DISK_GEOMETRY driveInfo;
	DeviceIoControl((void*)_get_osfhandle(FD), IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof(driveInfo), &dwResult, NULL);
	block_count = driveInfo.TracksPerCylinder * driveInfo.SectorsPerTrack * driveInfo.Cylinders.QuadPart;
#endif
	return block_count;
}

UDKFile::UDKFile(wxFileName& myfilename, FileAccessMode FAM, unsigned ForceBlockRW)
{
	m_FileAccessMode = FAM;
	m_TheFile = myfilename;
	m_FileLock = false;
	m_BlockRWSize = ForceBlockRW;
	m_ProcessID = -1;
	m_Getptr = m_Putptr = 0;
	if (myfilename.GetFullPath().Lower().StartsWith(wxT("-buf")))
	{
		memset(m_InternalFileBuffer.GetWriteBuf(1024), 0, 1024);
		m_InternalFileBuffer.UngetWriteBuf(1024);
		m_FileType = UDK_Buffer;
	}
	else
	{
		OSDependedOpen(myfilename, FAM, ForceBlockRW);
	}
}

#ifdef __WXMSW__
HANDLE GetDDK(PCWSTR a);

bool IsWinDevice(wxFileName myfilename)
{
	if (myfilename.GetFullPath().StartsWith(wxT(".:"))
		or myfilename.GetFullPath().StartsWith(wxT("\\Device\\Harddisk")))
		return true;
	return false;
}
bool UDKFile::OSDependedOpen(wxFileName& myfilename, FileAccessMode FAM, unsigned ForceBlockRW)
{
	//Windows special device opening
	std::cout << "WinOSDepOpen" << std::endl;
	//Handling Memory Process Debugging Here
	if (myfilename.GetFullPath().Lower().StartsWith(wxT("-pid=")))
	{
		long int a;
		myfilename.GetFullPath().Mid(5).ToLong(&a);
		m_ProcessID = a;
		m_RAMProcess = true;
		//HANDLE WINAPI OpenProcess(  _In_ DWORD dwDesiredAccess,  _In_ BOOL  bInheritHandle,  _In_ DWORD dwProcessId );
		m_HDevice = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_ProcessID);
		if (m_HDevice == NULL)
		{
			wxMessageBox(wxString::Format(_("Process ID:%d cannot be open."), m_ProcessID), _("Error"), wxOK | wxICON_ERROR);
			m_ProcessID = -1;
			return false;
		}
		//waitpid(ProcessID, NULL, WUNTRACED);
		m_BlockRWSize = 4;
		//BlockRWCount=0x800000000000LL/4;
		FAM = ReadOnly;
		return true;
	}
	else if (IsWinDevice(myfilename))
	{
		//wxFileName converts "\\.\E:" to ".:\E:"  so we need to fix this
		if (myfilename.GetFullPath().StartsWith(wxT(".:")))
		{
			m_devnm = wxString(wxT("\\\\.")) + myfilename.GetFullPath().AfterFirst(':');
		}
		else
		{
			m_devnm = myfilename.GetFullPath();
			//devnm=wxT("\\Device\\HarddiskVolume1");
		}

		DWORD dwResult;

		std::wcout << "WinDevice" << m_devnm << std::endl;
		if (myfilename.GetFullPath().StartsWith("\\Device")) {
			//hDevice=GetDDK(devnm);
			//std::cout << hDevice << std::endl;
			int nDosLinkCreated = m_Wdd.FakeDosNameForDevice(m_devnm.wchar_str(), m_szDosDevice, m_szCFDevice, FALSE);
			m_devnm = m_szCFDevice;
		}

		if (FAM == ReadOnly) 
		{
			m_HDevice = CreateFile(m_devnm.c_str(), GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_NO_BUFFERING | FILE_ATTRIBUTE_READONLY | FILE_FLAG_RANDOM_ACCESS,
				NULL);
			//Check if drive is mounted
			//#ifdef FSCTL_IS_VOLUME_MOUNTED //not available on Win???
			if (!DeviceIoControl(m_HDevice, FSCTL_IS_VOLUME_MOUNTED, NULL, 0, NULL, 0, &dwResult, NULL)
				&& (!m_devnm.StartsWith("\\\\.\\PhysicalDrive")) //PhysicalDrive can not checked since it's not logical volume to mount.
				) {
				DWORD err = GetLastError();
				std::cout << err << std::endl;
				wxMessageBox(_("Device is not mounted"), _("Error"), wxOK | wxICON_ERROR);
				return false;
			}
			//#endif //FSCTL_IS_VOLUME_MOUNTED
		}
		else
		{
			//	hDevice = CreateFile( devnm, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			m_HDevice = CreateFile(m_devnm.c_str(), GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS,
				NULL);

			//			 //lock volume
			//			if (!DeviceIoControl (hDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwResult, NULL))
			//				wxMessageBox( wxString::Format( wxT("Error %d attempting to lock volume: %s"), GetLastError (), devnm) );
			//
			//			//Dismount
			//			if (!DeviceIoControl (hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &dwResult, NULL))
			//				wxMessageBox( wxString::Format( wxT("Error %d attempting to dismount volume: %s"), GetLastError (), devnm) );
		}

		if (m_HDevice == INVALID_HANDLE_VALUE) // this may happen if another program is already reading from disk
		{
			std::cout << "Device cannot open due invalid handle : " << m_HDevice << std::endl;
			CloseHandle(m_HDevice);
			return false;
		}

		DISK_GEOMETRY driveInfo;

		DeviceIoControl(m_HDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof(driveInfo), &dwResult, NULL);
		m_BlockRWSize = driveInfo.BytesPerSector;
		m_BlockRWCount = driveInfo.TracksPerCylinder * driveInfo.SectorsPerTrack * driveInfo.Cylinders.QuadPart;

		int fd = _open_osfhandle(reinterpret_cast<intptr_t>(m_HDevice), 0);
#ifdef _DEBUG_
		std::cout << "Win Device Info:\n" << "Bytes per sector = " << BlockRWSize << "\nTotal number of bytes = " << BlockRWCount << std::endl;
#endif

		wxFile::Attach(fd);
		return true;
	}
	if (!myfilename.IsFileReadable()) {
		wxMessageBox(wxString(_("File is not readable by permissions.")) + wxT("\n") + _("Please change file permissons or run this program with Windows UAC privileges."), _("Error"), wxOK | wxICON_ERROR);
		return false;
	}

	return UDKFileOpen(myfilename, FAM, ForceBlockRW);
}

bool WindowsHDD::FakeDosNameForDevice(
	const WCHAR* lpszDiskFile, WCHAR* lpszDosDevice, WCHAR* lpszCFDevice,
	BOOL bNameOnly)
{
	if (wcsncmp(lpszDiskFile, L"\\\\", 2) == 0)
	{
		wcscpy(lpszCFDevice, lpszDiskFile);
		return 1;
	}

	BOOL bDosLinkCreated = TRUE;
	_snwprintf(lpszDosDevice, MAX_PATH, L"wxhxed%lu", GetCurrentProcessId());

	if (bNameOnly == FALSE)
		bDosLinkCreated = DefineDosDeviceW(DDD_RAW_TARGET_PATH, lpszDosDevice, lpszDiskFile);

	if (bDosLinkCreated == FALSE)
		return 1;
	else
		_snwprintf(lpszCFDevice, MAX_PATH, L"\\\\.\\%s", lpszDosDevice);

	return 0;
}


bool WindowsHDD::RemoveFakeDosName(const WCHAR* lpszDiskFile, const WCHAR* lpszDosDevice)
{
	BOOL bDosLinkRemoved = DefineDosDeviceW(DDD_RAW_TARGET_PATH | DDD_EXACT_MATCH_ON_REMOVE |
		DDD_REMOVE_DEFINITION, lpszDosDevice, lpszDiskFile);
	if (bDosLinkRemoved == FALSE)
		return 1;
	return 0;
}

void WindowsHDD::list_device(const WCHAR* format_str, const WCHAR* szTmp, int n)
{
	int nDosLinkCreated;
	HANDLE dev = INVALID_HANDLE_VALUE;
	DWORD dwResult;
	BOOL bResult;
	PARTITION_INFORMATION diskInfo;
	DISK_GEOMETRY driveInfo;
	WCHAR szDosDevice[MAX_PATH], szCFDevice[MAX_PATH];
	static LONGLONG deviceSize = 0;
	wchar_t size[100] = { 0 }, partTypeStr[1024] = { 0 }, * partType = partTypeStr;

	BOOL drivePresent = FALSE;
	BOOL removable = FALSE;

	drivePresent = TRUE;

	nDosLinkCreated = FakeDosNameForDevice(szTmp, szDosDevice, szCFDevice, FALSE);

	dev = CreateFileW(szCFDevice, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

	bResult = DeviceIoControl(dev, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0, &diskInfo, sizeof(diskInfo), &dwResult, NULL);

	// Test if device is removable
	if (// n == 0 &&
		DeviceIoControl(dev, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &driveInfo, sizeof(driveInfo), &dwResult, NULL))
		removable = driveInfo.MediaType == RemovableMedia;

	RemoveFakeDosName(szTmp, szDosDevice);
	CloseHandle(dev);

	if (!bResult)
		return;
	char* ascii = new char[wcslen(szTmp) + 1];
	memset(ascii, 0, wcslen(szTmp) + 1);
	wcstombs(ascii, szTmp, wcslen(szTmp));
	printf("Device Found: %s \r\n", ascii);
	devicenames.push_back(ascii);
}

void WindowsHDD::list_devices()
{
	const WCHAR* format_str = L"%-30s %9S %-9s %-20S\n";
	WCHAR szTmp[MAX_PATH];
	int i;
	for (i = 0; i < 64; i++)
	{
		_snwprintf(szTmp, sizeof(szTmp), L"\\\\.\\PhysicalDrive%d", i);
		list_device(format_str, szTmp, 0);
	}

	list_device(format_str, L"\\Device\\Ramdisk", 0);

	//	for (i = 0; i < 64; i++) {
	//		for (int n = 0; n <= 32; n++) {
	//			_snwprintf(szTmp, sizeof(szTmp), L"\\Device\\Harddisk%d\\Partition%d", i, n);
	//			list_device(format_str, szTmp, n);
	//			}
	//		}

	//	for (i = 0; i < 8; i++) {
	//		_snwprintf(szTmp, sizeof(szTmp), L"\\Device\\Floppy%d", i);
	//		list_device(format_str, szTmp, 0);
	//		}

	//	for (i = 0; i < 26; i++) {
	//		_snwprintf(szTmp, sizeof(szTmp), L"\\\\.\\%c:", 'A' + i);
	//		list_device(format_str, szTmp, 0);
	//		}

		//Add logical Drives here directly.
	uint32_t drives = GetLogicalDrives();
	for (int i = 2; i < 32; i++)
	{	
		//i=2 drops A: and B: flopies if available
		if ((drives >> i) & 0x01)
		{
			//printf("%c: ", 'A' + i);
			_snwprintf(szTmp, sizeof(szTmp), L"\\\\.\\%c:", 'A' + i);
			char* ascii = new char[wcslen(szTmp) + 1];
			memset(ascii, 0, wcslen(szTmp) + 1);
			wcstombs(ascii, szTmp, wcslen(szTmp));
			printf("Device Found: %s \r\n", ascii);
			//list_device(format_str, szTmp, 0);
			if (GetDriveTypeA(ascii + 4) != 4)	//ascii+4 for strip out Z: , !=4 checks if its Network Drive
				devicenames.push_back(ascii);
		}
	}

	///What about https://msdn.microsoft.com/en-us/library/windows/desktop/aa364425(v=vs.85).aspx
	///FindFirstVolume? FindNextVolume? FindCloseVolume?
	//HANDLE WINAPI FindFirstVolume(  _Out_ LPTSTR lpszVolumeName,  _In_  DWORD  cchBufferLength );
//	CHAR volname[256];
//	HANDLE dvar = FindFirstVolume( volname, 256 );
//	printf( "\r\nFindFirstVolume is: %s \r\n", volname );
//	while( FindNextVolume( dvar, volname, 256 ) )
//		printf( "FindNextVolume is: %s \r\n", volname );
//
//	int d = GetLogicalDriveStrings(  256, volname );
//	for(int i=0; i<d ; i++){
//		if(volname[i]==0)
//			printf(" ");
//		else
//			printf("%c", volname[i]);
//		}
//	printf("\r\n");
//	uint32_t drives=GetLogicalDrives();
//	for(int i=0; i<32 ; i++){
//		if((drives>>i) & 0x01 )
//			printf("%c: ", 'A' + i);
//		}
}

vector<string> WindowsHDD::getdevicenamevector()
{
	list_devices();
	return devicenames;
}
#elif defined( __WXGTK__ )

bool FAL::OSDependedOpen(wxFileName& myfilename, FileAccessMode FAM, unsigned ForceBlockRW) {
	struct stat fileStat;
	bool DoFileExists = (stat((const char*)myfilename.GetFullPath().fn_str(), &fileStat) >= 0);

	//Handling Memory Process Debugging Here
	if (myfilename.GetFullPath().Lower().StartsWith(wxT("-pid="))) {
		long int a;
		myfilename.GetFullPath().Mid(5).ToLong(&a);
		ProcessID = a;
		RAMProcess = true;
		if ((ptrace(PTRACE_ATTACH, ProcessID, NULL, 0)) < 0) {
			wxMessageBox(wxString::Format(_("Process ID:%d cannot be open."), ProcessID), _("Error"), wxOK | wxICON_ERROR);
			ProcessID = -1;
			return false;
		}
		waitpid(ProcessID, NULL, WUNTRACED);
		BlockRWSize = 4;
		BlockRWCount = 0x800000000000LL / 4;
		FAM = ReadOnly;
		FileType = FAL_Process;
		return true;
	}

	else if (!DoFileExists) {
		wxMessageBox(wxString(_("File does not exists at path:")) + wxT("\n") + myfilename.GetFullPath(), _("Error"), wxOK | wxICON_ERROR);
		return false;
	}

	//Owning file
	else if (!myfilename.IsFileReadable() && DoFileExists) { // "and myfilename.FileExist()" not used because it's for just plain files, not for block ones.
		if (wxCANCEL == wxMessageBox(wxString(_("File is not readable by permissions.")) + wxT("\n") +
			_("Please change file permissons or run this program with root privileges.") + wxT("\n") +
			_("You can also try to own the file temporarily (requires root's password.)") + wxT("\n") + wxT("\n") +
			_("Do you want to own the file?"), _("Warning"), wxOK | wxCANCEL | wxICON_WARNING))

			return false;

		wxArrayString output, errors;

		//Save old owner to update at file close...
		wxExecute(wxT("stat -c %U ") + myfilename.GetFullPath(), output, errors, wxEXEC_SYNC);
		if (output.Count() > 0 && errors.Count() == 0)
			oldOwner = output[0];//this return root generally :D
		else {
			wxMessageBox(_("Unknown error on \"stat -c %U") + myfilename.GetFullPath() + wxT("\""), _("Error"), wxOK | wxCANCEL | wxICON_ERROR);
			return false;
		}
		//Changing owner of file...
		//I think it's better than changing permissions directly. Doesn't it?
		//Will restore owner on file close.
		wxString cmd, spacer = wxT(" ");
		if (wxFile::Exists(wxT("/usr/bin/pkexec"))) {
			cmd = wxT("pkexec --user root chown \"");
			spacer = wxT("\" \"");
		}
		else if (wxFile::Exists(wxT("/usr/bin/gnomesu")))
			cmd = wxT("gnomesu -u root -c \"chown ");
		else if (wxFile::Exists(wxT("/usr/bin/gksu")))
			cmd = wxT("gksu -u root \"chown ");
		else if (wxFile::Exists(wxT("/usr/bin/gksudo")))
			cmd = wxT("gksudo -u root \"chown ");
		else {
			wxMessageBox(_("For using this function, please install \"pkexec\", \"gnomesu\" or \"gksu\" tools first."), _("Error"), wxOK | wxCANCEL | wxICON_ERROR);
			return false;
		}
		cmd += wxGetUserId() + spacer + myfilename.GetFullPath() + wxT("\"");
#ifdef _DEBUG_
		std::cout << "Changing permission of " << myfilename.GetFullPath().ToAscii() << std::endl;
		std::cout << cmd.ToAscii() << std::endl;
#endif
		//wxExecute( cmd , output, errors, wxEXEC_SYNC);
		wxShell(cmd);
	}
	FileType = FAL_File;
	return FALOpen(myfilename, FAM, ForceBlockRW);
}

#elif defined( __WXOSX__ )
bool FAL::OSDependedOpen(wxFileName& myfilename, FileAccessMode FAM, unsigned ForceBlockRW) {
	//Handling Memory Process Debugging Here
	if (myfilename.GetFullPath().Lower().StartsWith(wxT("-pid="))) {
		long int a;
		myfilename.GetFullPath().Mid(5).ToLong(&a);
		ProcessID = a;
		RAMProcess = true;
		if ((ptrace(PTRACE_ATTACH, ProcessID, NULL, NULL)) < 0) {
			wxMessageBox(wxString::Format(_("Process ID:%d cannot be open."), ProcessID), _("Error"), wxOK | wxICON_ERROR);
			ProcessID = -1;
			return false;
		}
		waitpid(ProcessID, NULL, WUNTRACED);
		BlockRWSize = 4;
		FAM == ReadOnly;
		return true;
	}

	if (!myfilename.IsFileReadable()) {
		wxMessageBox(wxString(_("File is not readable by permissions.")) + wxT("\n") + _("Please change file permissons or run this program with root privileges"), _("Error"), wxOK | wxICON_ERROR);
		return false;
	}
	return FALOpen(myfilename, FAM, ForceBlockRW);
}
#endif

bool IsBlockDev(int FD)
{
	struct stat* sbufptr = new struct stat;
	fstat(FD, sbufptr);
#ifdef __WXMSW__
	return sbufptr->st_mode == 0;	//Enable block size detection code on windows targets,
#elif defined (BSD)
	//return S_ISBLK( sbufptr->st_mode ); //this not working on BSD
	return S_ISCHR(sbufptr->st_mode); //well, this works...
	//return !S_ISREG( sbufptr->st_mode ); //if not an ordinary file, it might be block...
#else
	return S_ISBLK(sbufptr->st_mode);
#endif
}

// Complete function
bool UDKFile::UDKFileOpen(wxFileName& myfilename, FileAccessMode FAM, unsigned ForceBlockRW)
{
#ifdef _DEBUG_
	std::cout << "FAL:FALOpen( " << myfilename.GetFullPath().ToAscii() << " )" << std::endl;
#endif // _DEBUG_
	if (myfilename.IsFileReadable())
	{//FileExists()){
		if (FAM == ReadOnly)
			Open(myfilename.GetFullPath(), wxFile::read);
		else
			Open(myfilename.GetFullPath(), wxFile::read_write);

		if (!IsOpened()) {
			m_FileAccessMode = AccessInvalid;
			wxMessageBox(_("File cannot open."), _("Error"), wxOK | wxICON_ERROR);
			return false;
		}

		if (IsBlockDev(wxFile::fd()))
		{
			m_BlockRWSize = FDtoBlockSize(wxFile::fd());
			m_BlockRWCount = FDtoBlockCount(wxFile::fd());
		}
		else if (ForceBlockRW)
		{
			m_BlockRWCount = wxFile::Length() / ForceBlockRW;
		}

		return true;
	}
	else if (m_FileType == UDK_Buffer)
	{
		return true;
	}
	else
	{
		return false;
	}
}

long UDKFile::Read(char* buffer, int size)
{
	return Read(reinterpret_cast<unsigned char*>(buffer), size);
}

long UDKFile::Read(unsigned char* buffer, int size)
{
	//Why did I calculate j here? To find active patch indice...
	int j = 0;
	for (unsigned i = 0; i < m_DiffArray.GetCount(); i++)
		if (m_DiffArray[i]->flag_undo && !m_DiffArray[i]->flag_commit)	// Allready committed to disk, nothing to do here
			break;
		else
			j = i + 1;

	long ret = ReadR(buffer, size, m_Getptr, &m_DiffArray, j);

	//Encryption layer
	ApplyXOR(buffer, ret, m_Getptr);

	//for next read
	m_Getptr += ret;

	return ret;
}

///State of the Art Read Function
//Only recursion could handle such a complex operation. Comes my mind while I am trying to sleep.
long UDKFile::ReadR(unsigned char* buffer, unsigned size, uint64_t from, ArrayOfNode* PatchArray, int PatchIndice)
{
#ifdef _DEBUG_FILE_
	std::cout << "ReadR from:" << std::dec << from << "\t size:" << std::setw(3) << size << "\t PtchIndice" << PatchIndice << std::endl;
#endif // _DEBUG_FILE_
	///Getting Data from bellow layer.
	if (PatchIndice == 0)	//Deepest layer
	{
		//Block Read/Write mechanism.
		//if( 0 )//for debugging

#ifdef __linux__
		//Linux file read speed up hack for Block devices.
		//This macro disables blockRW code.
		//Block read code just allowed for memory devices under linux.
		//Because you can read arbitrary locations from block devices at linux.
		//Kernel handle the job...
		//This hack increase reading speed from 164MB to 196MB on my SSD with using read 4MB buffer due use of memcmp.
		//(Max disk rw is 230MB/s)
		if (ProcessID >= 0)
#else
		if (m_BlockRWSize > 0)
#endif
		{
			///NOTE:This function just read +1 more sectors and copies to buffer via memcpy, thus inefficient, at least for SSD's.
			///TODO:Need to read 1 sector at start to bfs, than read to buffer directly and read one more sector to bfr.
			///Than we could copy readed sectors via memcpy which supposed to increase copy speed some percent.

			//Start & end sector and shift calculation.
			uint64_t StartSector = from / m_BlockRWSize;
			unsigned StartShift = from - (from / m_BlockRWSize) * m_BlockRWSize;
			uint64_t EndSector = (from + size) / m_BlockRWSize;

			if (EndSector > m_BlockRWCount - 1)
				EndSector = m_BlockRWCount - 1;

			int rd_size = (EndSector - StartSector + 1) * m_BlockRWSize; //+1 for read least one sector
			char* bfr = new char[rd_size];
			int rd = 0;

			//If reading from a process memory
			if (m_ProcessID >= 0)
			{
				long word = 0;
#ifdef __WXMSW__
				SIZE_T written = 0;
#endif
				char* addr;
				//unsigned long *ptr = (unsigned long *) buffer;
				while (rd < rd_size) {
					addr = reinterpret_cast<char*>(StartSector * m_BlockRWSize + rd);
#ifdef __WXMSW__
					ReadProcessMemory(m_HDevice, addr, &word, sizeof(word), &written);
#else
					word = ptrace(PTRACE_PEEKTEXT, ProcessID, addr, 0);
#endif
					memcpy(bfr + rd, &word, 4);
					rd += 4;
				}

			}
			//Reading from a file
			else
			{
				wxFile::Seek(StartSector * m_BlockRWSize);
				rd = wxFile::Read(bfr, rd_size);
			}
			//Here, we adjust shifting by copying bfr to buffer. Inefficient but easy to programe.
			memcpy(buffer, bfr + StartShift, wxMin(wxMin(rd, rd_size - StartShift), size)); //wxMin protects file ends.
			delete[] bfr;
			return wxMin(wxMin(rd, rd_size - StartShift), size);
		}
		else if (m_FileType == UDK_Buffer)
		{
			memcpy(buffer, reinterpret_cast<char*>(m_InternalFileBuffer.GetData()) + from, size);
			return (from + size > m_InternalFileBuffer.GetDataLen() ? m_InternalFileBuffer.GetDataLen() - from : size);
		}
		wxFile::Seek(from); //Since this is the Deepest layer
		return wxFile::Read(buffer, size); //Ends recursion. here
	}

	int readsize = 0;
	//than process at current layer
	if (PatchIndice != 0) {
		DiffNode* patch = PatchArray->Item(PatchIndice - 1); //PatchIndice-1 means the last patch item! Don't confuse at upper code.
		if (patch->flag_inject && patch->size < 0) {//Deletion patch
			readsize = DeletionPatcher(from, buffer, size, PatchArray, PatchIndice);
		}
		else if (patch->flag_inject) {	//Injection patch
			readsize = InjectionPatcher(from, buffer, size, PatchArray, PatchIndice);
		}
		else {
			readsize = ReadR(buffer, size, from, PatchArray, PatchIndice - 1);//PatchIndice-1 => Makes Patch = 0 for 1 patch. Read from file.
			if (size != static_cast<unsigned int>(readsize)) //If there is free chars
				readsize = (Length(PatchIndice - 1) - from > size) ? (size) : (Length(PatchIndice - 1) - from);	//check for buffer overflow
			ModificationPatcher(from, buffer, size, patch);
		}
	}
	//	if(readsize < 0)
	//		return -1;
	if (static_cast<int64_t>(from + readsize) > Length(PatchIndice)) {
		//Injection fills all buffer as requested. So we need truncate it for avoid random memory on file end.
		readsize = Length(PatchIndice) - from;
	}
#ifdef _DEBUG_FILE_
	std::cout << "Read Size:" << std::setw(3) << readsize << std::endl;
#endif // _DEBUG_FILE_
	return readsize;
}

wxFileOffset UDKFile::Length(int PatchIndice)
{
	if (m_ProcessID >= 0)
		return 0x800000000000LL;

	if (m_BlockRWSize > 0)
		return m_BlockRWSize * m_BlockRWCount;

#ifdef __WXGTK__
	if (the_file.GetFullPath() == wxT("/dev/mem")) {
		return 512 * MB;
	}
#endif
#ifdef __WXMAC__
	if (the_file.GetFullPath().StartsWith(wxT("/dev/disk"))) {
		int block_size = 0;
		int64_t block_count = 0;
		if (ioctl(fd(), DKIOCGETBLOCKSIZE, &block_size) || ioctl(fd(), DKIOCGETBLOCKCOUNT, &block_count))
			return -1;
		else
			return block_size * block_count;
	}
#endif

	if (!IsOpened())
		return -1;

	wxFileOffset max_size;
	if (m_FileType == UDK_Buffer)
		max_size = m_InternalFileBuffer.GetDataLen();
	else
		max_size = wxFile::Length();

#ifdef __WXGTK__
	///WorkAround for wxFile::Length() zero size bug
	if (max_size == 0) { //This could be GIANT file like /proc/kcore = 128 TB -10MB +12KB
		struct stat st;
		stat(the_file.GetFullPath().To8BitData(), &st);
		uint64_t sz = st.st_size;
#ifdef _DEBUG_FILE_
		printf("File Size by STD C is : %llu \r\n", sz);
#endif // _DEBUG_FILE_
		max_size = sz;
	}
#endif // __wxGTK__

	if (PatchIndice == -1)
		PatchIndice = m_DiffArray.GetCount();
	for (int i = 0; i < PatchIndice; i++)
		if (m_DiffArray[i]->flag_undo && !m_DiffArray[i]->flag_commit)
			continue;
		else if (m_DiffArray[i]->flag_inject)
			max_size += m_DiffArray[i]->size;

	return max_size;
}

long UDKFile::InjectionPatcher(uint64_t current_location, unsigned char* data, int size, ArrayOfNode* PatchArray, int PatchIndice)
{
#ifdef _DEBUG_FILE_
	std::cout << "InjectionP:" << std::dec << current_location << "\t size:" << std::setw(3) << size << "\t PtchIndice" << PatchIndice << std::endl;
#endif // _DEBUG_FILE_
	/******* MANUAL of Code Understanding *******
	* current_location                    = [   *
	* current_location + size             = ]   *
	* Injections start_offset             = (   *
	* Injections start_offset + it's size = )   *
	* data                                = ... *
	* first partition                     = xxx *
	********************************************/
	if (PatchArray->Item(PatchIndice - 1)->size > 0 && PatchArray->Item(PatchIndice - 1)->flag_inject) {
		DiffNode* Inject_Node = PatchArray->Item(PatchIndice - 1);	//For easy debugging
		///If injection starts behind, so we needed to read bytes from before to show correct location.
		///State ...().,,,[...]..... -> ...(...).[,,,].
		if (Inject_Node->end_offset() <= current_location) {
			return ReadR(data, size, current_location - Inject_Node->size, PatchArray, PatchIndice - 1);
		}
		///State ...(),,,[....]..... -> ...(..[xx),,],..
		else if (Inject_Node->start_offset <= current_location
			&& Inject_Node->end_offset() > current_location
			&& Inject_Node->end_offset() < current_location + size
			) {
			int movement = current_location - Inject_Node->start_offset;   // This bytes discarded from injection
			int first_part = Inject_Node->end_offset() - current_location; // First part size
			memcpy(data, Inject_Node->new_data + movement, first_part);  // Copy first part to buffer
			int read_size = ReadR(data + first_part, size - first_part, Inject_Node->start_offset, PatchArray, PatchIndice - 1); //Than copy second part.
			return wxMin(read_size + first_part, size);
		}

		///State ...(),,,[...]..... -> ...(...[...].),,,
		else if (Inject_Node->start_offset <= current_location
			&& Inject_Node->end_offset() >= current_location + size
			) {
			int movement = current_location - Inject_Node->start_offset;	// This bytes discarded from injection
			memcpy(data, Inject_Node->new_data + movement, size);			// Copy first part to buffer
			return size;
		}
		///State ...[...(),,,]... -> ...[xx(...)...],,,...
		else if (Inject_Node->start_offset > current_location
			&& Inject_Node->start_offset < current_location + size
			&& Inject_Node->end_offset() < current_location + size
			) {
			int first_part = Inject_Node->start_offset - current_location; // First part size
			int read_size = ReadR(data, first_part, current_location, PatchArray, PatchIndice - 1); //Read First Part from lower layer
			memcpy(data + first_part, Inject_Node->new_data, Inject_Node->size);  // Copy second part to buffer
			read_size += ReadR(data + first_part + Inject_Node->size, size - (first_part + Inject_Node->size), current_location + first_part, PatchArray, PatchIndice - 1); //Read latest chunk
			return wxMin(read_size + Inject_Node->size, size);
		}
		///State ...[...(),,,]... -> ...[xx(...]...),,,...
		else if (Inject_Node->start_offset > current_location
			&& Inject_Node->start_offset < current_location + size
			&& Inject_Node->end_offset() > current_location + size
			) {
			int first_part = Inject_Node->start_offset - current_location; // First part size
			int read_size = ReadR(data, first_part, current_location, PatchArray, PatchIndice - 1); //Read First Part from lower layer
			memcpy(data + first_part, Inject_Node->new_data, size - first_part);  // Copy second part to buffer
			return wxMin(read_size + Inject_Node->size, size);
		}
		///State ...[...]...(...)... -> ...[...]...()...
		else {
			return ReadR(data, size, current_location, PatchArray, PatchIndice - 1);
		}
	}
	return -1;
}

long UDKFile::DeletionPatcher(uint64_t current_location, unsigned char* data, int size, ArrayOfNode* PatchArray, int PatchIndice)
{
#ifdef _DEBUG_FILE_
	std::cout << "Deletion P:" << std::dec << current_location << "\t size:" << std::setw(3) << size << "\t PtchIndice" << PatchIndice << std::endl;
#endif // _DEBUG_FILE_
	/******* MANUAL of Code Understanding ******
	* current_location                   = [   *
	* current_location + size            = ]   *
	* Deletes start_offset               = (   *
	* Deletes start_offset + it's size   = )   *
	* data                               = ... *
	* first partition                    = xxx *
	*******************************************/
	if (PatchArray->Item(PatchIndice - 1)->size < 0) {
		DiffNode* Delete_Node = PatchArray->Item(PatchIndice - 1);	//For easy debugging
		///If deletion starts behind, so we needed to read bytes from after to show correct location.
		///State ...(...)...[...],,,. -> ...()......[,,,].
		///State ...(..[..).].,,,.... -> ...()..[,,,]....
		///State ...(..[...].)..,,,.. -> ...()..[,,,]..
		if (Delete_Node->start_offset <= current_location) {
#ifdef _DEBUG_FILE_
			std::cout << "D->";
#endif // _DEBUG_FILE_
			return ReadR(data, size, current_location - Delete_Node->size, PatchArray, PatchIndice - 1);
		}
		///State ...[xxx(...)...],,,... -> ...[xxx()...,,,]...
		///State ...[xxx(...]...),,,... -> ...[xxx(),,,...]...
		else if (Delete_Node->start_offset > current_location && Delete_Node->start_offset < current_location + size) {
			int first_part_size = Delete_Node->start_offset - current_location;
			int read_size = 0;
#ifdef _DEBUG_FILE_
			std::cout << "D->";
#endif // _DEBUG_FILE_
			read_size += ReadR(data, first_part_size, current_location, PatchArray, PatchIndice - 1);
#ifdef _DEBUG_FILE_
			std::cout << "D->";
#endif // _DEBUG_FILE_
			read_size += ReadR(data + first_part_size, size - first_part_size, current_location + first_part_size - Delete_Node->size, PatchArray, PatchIndice - 1);
			return read_size;
		}
		///State ...[...]...(...)... -> ...[...]...()...
		else {
#ifdef _DEBUG_FILE_
			std::cout << "D->";
#endif // _DEBUG_FILE_
			return ReadR(data, size, current_location, PatchArray, PatchIndice - 1);
		}
	}
	return -1;
}

void UDKFile::ModificationPatcher(uint64_t current_location, unsigned char* data, int size, DiffNode* Patch)
{
	/***** MANUAL of Code understanding *********
	* current_location                    = [   *
	* current_location + size             = ]   *
	* Patch's start_offset                = (   *
	* Patch's start_offset + it's size    = )   *
	* data                                = ... *
	* changed data                        = xxx *
	********************************************/
	if (Patch->flag_undo && !Patch->flag_commit)	// Already committed to disk, nothing to do here
		return;
	///State: ...[...(xxx]xxx)...
	if (current_location <= Patch->start_offset && current_location + size >= Patch->start_offset) {
		int irq_loc = Patch->start_offset - current_location;
		///...[...(xxx)...]... //not neccessery, this line includes this state too
		int irq_size = (size - irq_loc > Patch->size) ? (Patch->size) : (size - irq_loc);

		//memcpy(data+irq_loc , Patch->flag_undo ? Patch->old_data : Patch->new_data, irq_size );
		Patch->Apply(data + irq_loc, irq_size, 0);
	}

	///State: ...(xxx[xxx)...]...
	else if (current_location <= Patch->start_offset + Patch->size && current_location + size >= Patch->start_offset + Patch->size) {
		int irq_skipper = current_location - Patch->start_offset;	//skip this bytes from start
		int irq_size = Patch->size - irq_skipper;

		//memcpy(data, Patch->flag_undo ? Patch->old_data : Patch->new_data + irq_skipper, irq_size );
		Patch->Apply(data, irq_size, irq_skipper);
	}

	///State: ...(xxx[xxx]xxx)...
	else if (Patch->start_offset <= current_location && Patch->start_offset + Patch->size >= current_location + size) {
		int irq_skipper = current_location - Patch->start_offset;	//skip this bytes from start

		//memcpy(data, Patch->flag_undo ? Patch->old_data : Patch->new_data + irq_skipper, size );
		Patch->Apply(data, size, irq_skipper);
	}
}

bool UDKFile::Apply(void)
{
	bool success = true;
	if (m_FileAccessMode != ReadOnly)
		for (unsigned i = 0; i < m_DiffArray.GetCount(); i++)
		{
			if (m_DiffArray[i]->flag_commit == m_DiffArray[i]->flag_undo)
			{
				// If there is unwriten data
				Seek(m_DiffArray[i]->start_offset, wxFromStart);					// write it

				if (m_BlockRWSize > 0)
				{
					uint64_t StartSector = m_Putptr / m_BlockRWSize;
					unsigned StartShift = m_Putptr - StartSector * m_BlockRWSize;
					uint64_t EndSector = (m_Putptr + m_DiffArray[i]->size) / m_BlockRWSize;
					long unsigned int rd_size = (EndSector - StartSector + 1) * m_BlockRWSize;// +1 for make read least one sector
					char* bfr = new char[rd_size];
					//for( int i=0 i< rd_size; i++) bfr[i]=0;
					long unsigned rd = 0;
					//#ifdef __DEBUG__
					std::cout << "BlockRW sectors:" << StartSector << ":" << EndSector << " Size:" << rd_size << std::endl;
					//#endif // __DEBUG__

					//First read the original sector
					if (m_ProcessID >= 0)
					{
						long word = 0;
#ifdef __WXMSW__
						SIZE_T written = 0;
#endif
						char* addr = 0;
						//unsigned long *ptr = (unsigned long *) buffer;
						while (rd < rd_size) {
							addr = reinterpret_cast<char*>(StartSector * m_BlockRWSize + rd);
#ifdef __WXMSW__
							ReadProcessMemory(m_HDevice, addr, &word, sizeof(word), &written);
#else
							word = ptrace(PTRACE_PEEKTEXT, ProcessID, addr, 0);
#endif
							memcpy(bfr + rd, &word, 4);
							rd += 4;
						}
					}
					else
					{
						wxFile::Seek(StartSector * m_BlockRWSize);
						rd = wxFile::Read(bfr, rd_size);
					}

					if (rd != rd_size)
					{	//Crucial if  block read error.
						delete[] bfr;
						return false;
					}

					//if already written and makeing undo, than use old_data
					memcpy(bfr + StartShift, (m_DiffArray[i]->flag_commit ? m_DiffArray[i]->old_data : m_DiffArray[i]->new_data), m_DiffArray[i]->size);

					//Than apply the changes

					//if memory process
					if (m_ProcessID >= 0)
					{
						unsigned wr = 0;
						long word = 0;
#ifdef __WXMSW__
						SIZE_T written = 0;
#endif
						char* addr;
						while (wr < rd_size) {
							addr = reinterpret_cast<char*>(StartSector * m_BlockRWSize + wr);
							memcpy(&word, bfr + wr, sizeof(word));
#ifdef __WXMSW__
							if (WriteProcessMemory(m_HDevice, addr, &word, sizeof(word), &written) == 0)
#else
							if (ptrace(PTRACE_POKETEXT, ProcessID, addr, word) == -1)
#endif
								wxMessageBox(_("Error on Write operation to Process RAM"), _("FATAL ERROR"));
							wr += 4;
						}
						success &= true;
					}
					else
					{
						wxFile::Seek(StartSector * m_BlockRWSize);
						success = Write(bfr, rd_size) && success;
					}
					delete[] bfr;
				}
				else if (m_FileType == UDK_Buffer)
				{
					memcpy(m_InternalFileBuffer.GetData(), (m_DiffArray[i]->flag_commit ? m_DiffArray[i]->old_data : m_DiffArray[i]->new_data), m_DiffArray[i]->size);
				}
				else
				{
					//if already written and makeing undo, than use old_data
					success = Write((m_DiffArray[i]->flag_commit ? m_DiffArray[i]->old_data : m_DiffArray[i]->new_data), m_DiffArray[i]->size) && success;
				}
				if (success)
				{
					m_DiffArray[i]->flag_commit = m_DiffArray[i]->flag_commit ? false : true;	//alter state of commit flag
				}
			}
		}
	return success;
}

DiffNode* UDKFile::NewNode(uint64_t start_byte, const char* data, int64_t size, bool inject)
{
	DiffNode* newnode = new DiffNode(start_byte, size, inject);
	if (size < 0) {//Deletion!
		newnode->old_data = new unsigned char[-size];
		if (newnode->old_data == NULL)
			wxLogError(_("Not Enought RAM"));
		else {
			Seek(start_byte, wxFromStart);
			Read(newnode->old_data, -size);
			return newnode;
		}
	}
	else if (inject) {
		newnode->new_data = new unsigned char[size];
		if (newnode->new_data == NULL)
			wxLogError(_("Not Enought RAM"));
		else {
			memcpy(newnode->new_data, data, size);
			return newnode;
		}
	}
	else {// Normal opeariton
		newnode->old_data = new unsigned char[size];
		newnode->new_data = new unsigned char[size];
		if (newnode->old_data == NULL || newnode->new_data == NULL) {
			wxLogError(_("Not Enought RAM"));
			delete[] newnode->old_data;
			delete[] newnode->new_data;
			return NULL;
		}
		else {
			memcpy(newnode->new_data, data, size);
			Seek(start_byte, wxFromStart);
			Read(newnode->old_data, size);
			return newnode;
		}
	}
	return NULL;
}

bool UDKFile::Add(uint64_t start_byte, const char* data, int64_t size, bool injection)
{
	//Check for undos first
	for (unsigned i = 0; i < m_DiffArray.GetCount(); i++)
	{
		if (m_DiffArray[i]->flag_undo)
		{
			if (m_DiffArray.Item(i)->flag_commit)
			{
				// commited undo node
				m_DiffArray[i]->flag_undo = false;		//we have to survive this node as it unwriten, non undo node
				m_DiffArray[i]->flag_commit = false;
				unsigned char* temp_buffer = m_DiffArray[i]->old_data;	//swap old <-> new data
				m_DiffArray[i]->old_data = m_DiffArray[i]->new_data;
				m_DiffArray[i]->new_data = temp_buffer;
			}
			else
			{
				// non committed undo node
				while (i < (m_DiffArray.GetCount()))
				{
					// delete beyond here
#ifdef _DEBUG_FILE_
					std::cout << "DiffArray.GetCount() : " << DiffArray.GetCount() << " while i = " << i << std::endl;
#endif
					DiffNode* temp;
					temp = *(m_DiffArray.Detach(i));
					delete temp;
				}
				break;								// break for addition
			}
		}
	}
	//Adding node here
	DiffNode* rtn = NewNode(start_byte, data, size, injection);
	ApplyXOR(rtn->new_data, size, start_byte);
	if (rtn != NULL)
	{
		m_DiffArray.Add(rtn);						//Add new node to tail
		if (m_FileAccessMode == DirectWrite)	//Direct Write mode is always applies directly.
			Apply();
		return true;
	}
	else
		return false;							//if not created successfuly
}

void UDKFile::ApplyXOR(unsigned char* buffer, unsigned size, uint64_t from)
{
	if (m_XORview.GetDataLen())
	{
		unsigned Xi = from % m_XORview.GetDataLen(); //calculates keyshift
		for (unsigned int i = 0; i < size; i++)
		{
			buffer[i] ^= m_XORview[Xi];
			if (Xi++ == m_XORview.GetDataLen() - 1)
				Xi = 0;
		}
	}
}

wxFileOffset UDKFile::Seek(wxFileOffset ofs, wxSeekMode mode)
{
	if (!IsOpened())
		return -1;

	m_Getptr = m_Putptr = ofs;

	if (m_BlockRWSize > 0 || m_FileType == UDK_Buffer || m_ProcessID > 0)
		return ofs;

	return wxFile::Seek(ofs, mode);
}

bool DiffNode::Apply(unsigned char* data_buffer, int64_t size, int64_t irq_skip)
{
	if (virtual_node == NULL)
	{
		memcpy(data_buffer, flag_undo ? old_data : new_data + irq_skip, size);
		return true;
	}
	virtual_node->Seek(virtual_node_start_offset + irq_skip, wxFromStart);
	virtual_node->Read(data_buffer, size);
	return true;
}