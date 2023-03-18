/*
 *   ---------------------
 *  |  UDKFile.h
 *   ---------------------
 *   This file is part of Unreal DeKompiler.
 *
 *   Unreal DeKompiler is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Unreal DeKompiler is distributed in the hope and belief that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Unreal DeKompiler.  If not, see <https://www.gnu.org/licenses/>.
 *
 *   Timeline:
 *   March, 2023: First Inscription.
 */

#pragma once

#include <iostream>	//for std::cout...
#include <wx/wx.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/dynarray.h>
#include <stdint.h>

 //this utility uses old ECS format.
#define KB 1024LL
#define MB 1048576LL
#define GB 1073741824LL
#define TB 1099511627776LL

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/param.h>		// for BSD
#endif

#ifdef __WXMAC__
#include <sys/disk.h>
#endif

#ifdef WX_GCH
#include <wx_pch.h>
#else
#include <wx/wx.h>
#endif

#ifdef __WXMSW__
	#include <windows.h>
	#include <winioctl.h>
	//#include <windrv.h>
#else
	#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __WXGTK__
#include <sys/ioctl.h>
//#include <dev/disk.h>

#ifdef BSD
#include <sys/ptrace.h>
#include <sys/disk.h>
#else
#include <linux/fs.h>
#endif

//#include <link.h>
//#include <elf.h>
//#include <stdlib.h>
//#include <string.h>
#include <sys/wait.h>
//#include <sys/types.h>
//#include <stdio.h>
//#include <errno.h>

#endif

int FDtoBlockSize(int FD);
uint64_t FDtoBlockCount(int FD);

class UDKFile;

class DiffNode
{
public:
	bool flag_undo;			//For undo
	bool flag_commit;			//For undo
	bool flag_inject;			//this is for file size extension. If this flag is true, that means a new data inserted to file
	uint64_t start_offset;	//start byte of node
	int64_t size;				//size of node
	unsigned char* old_data;			//old data buffer
	unsigned char* new_data;			//new data buffer
	inline uint64_t end_offset(void) { return start_offset + abs(size); }
	bool Apply(unsigned char* buffer, int64_t size, int64_t irq_skip);
	UDKFile* virtual_node;
	uint64_t virtual_node_start_offset;
	uint64_t virtual_node_size;

	//	DiffNode *prev, *next;
	DiffNode(uint64_t start, int64_t size_, bool inject)
	{
		start_offset = start;
		flag_inject = inject;
		size = size_;
		flag_commit = flag_undo = false;
		old_data = new_data = NULL;
		virtual_node = NULL;
		virtual_node_start_offset = 0;
	}
	~DiffNode(void) {
		delete[] new_data;
		delete[] old_data;
	}
};

WX_DECLARE_OBJARRAY(DiffNode*, ArrayOfNode);

/// <summary>
/// All the relevant modes
/// </summary>
enum FileAccessMode
{ 
	ReadOnly, 
	ReadWrite, 
	DirectWrite, 
	ForcedReadOnly, 
	AccessInvalid 
};

/// <summary>
/// Types of instances to be read
/// </summary>
enum UDKFileTypes
{
	UDK_File,
	UDK_Device,
	UDK_Process,
	UDK_Buffer
};

#ifdef __WXMSW__
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;
/*
struct _opt {
	bool			list;
	bool			yes;
	unsigned int	quiet;
	ULONGLONG		start;
	ULONGLONG		end;
	bool			read;
	bool			kilobyte;
	unsigned int	refresh;
	};

typedef struct _opt t_opt;
static t_opt opt = {
	false,			// list
	0,				// quiet
	0,				// start
	0,				// end
	false,			// read
	false,			// kilobyte
	1,				// refresh
	};
*/
class WindowsHDD
{
private:
	vector<string> devicenames;
	void list_device(const WCHAR* format_str, const WCHAR* szTmp, int n);
	void list_devices();
public:
	bool FakeDosNameForDevice(const WCHAR* lpszDiskFile, WCHAR* lpszDosDevice, WCHAR* lpszCFDevice, BOOL bNameOnly);
	bool RemoveFakeDosName(const WCHAR* lpszDiskFile, const WCHAR* lpszDosDevice);
	vector<string> getdevicenamevector();
};
#endif

/// <summary>
/// The UDK's file class. The file class equivalen of wxHexEditor's FAL
/// </summary>
class UDKFile : private wxFile
{
public:
	//std::queue readqueue;
	//or
	//std:sstream readstream;
	UDKFile(wxFileName& myfilename, FileAccessMode FAM = ReadOnly, unsigned ForceBlockRW = 0);
	~UDKFile();

	bool SetAccessMode(FileAccessMode fam);
	int GetAccessMode(void);
	wxString GetAccessModeString(void);
	wxString FAMtoString(FileAccessMode& FAM);
	wxFileName GetFileName(void);

	bool IsChanged(void);	//returns if file is dirty or not
	bool Apply(void);		//flush changes to file
	bool Flush(void) { return wxFile::Flush(); }
	size_t BlockWrite(unsigned char* buffer, unsigned size);
	int64_t Undo(void);	//undo last action
	int64_t Redo(void);	//redo last undo
	void ShowDebugState(void);
	wxFileOffset Length(int PatchIndice = -1);

	wxFileOffset Seek(wxFileOffset ofs, wxSeekMode mode = wxFromStart);
	bool OSDependedOpen(wxFileName& myfilename, FileAccessMode FAM = ReadOnly, unsigned ForceBlockRW = 0);
	bool UDKFileOpen(wxFileName& myfilename, FileAccessMode FAM = ReadOnly, unsigned ForceBlockRW = 0);
	bool Close();
	inline bool IsProcess()
	{	
		//For detect if this is RAM Process device
		return (m_ProcessID >= 0);
	}
	inline int GetPID() const
	{
		return m_ProcessID;
	}
	bool IsOpened() const
	{
		if (m_ProcessID >= 0 || m_FileType == UDK_Buffer)
		{
			return true;
		}
		return wxFile::IsOpened();
	};
	int fd() const { return wxFile::fd(); };
	virtual	long Read(char* buffer, int size);
	virtual	long Read(unsigned char* buffer, int size);
	void SetXORKey(wxMemoryBuffer);
	wxMemoryBuffer GetXORKey(void);
	void ApplyXOR(unsigned char* buffer, unsigned size, uint64_t from);

	bool Add(uint64_t start_byte, const char* data, int64_t size, bool injection = false); //adds new node
	bool AddDiffNode(DiffNode* new_node);
	bool IsAvailable_Undo(void);
	bool IsAvailable_Redo(void);
	bool IsInjected(void);
	const DiffNode* GetFirstUndoNodeOrLast(void);
	int GetBlockSize(void);
#ifdef __WXMSW__
	HANDLE GetHandle(void)
	{
		return m_HDevice;
	}
#endif
	inline UDKFileTypes GetFileType() const { return m_FileType; }

public:
	bool m_FileLock;
	wxString m_FileLockedBy;
	wxString m_OldOwner;

protected:
	long ReadR(unsigned char* buffer, unsigned size, uint64_t location, ArrayOfNode* Patches, int PatchIndice);

	void RemoveTail(DiffNode* remove_node);	//remove further tails.
	DiffNode* NewNode(uint64_t start_byte, const char* data, int64_t size, bool extension = false);
	long DeletionPatcher(uint64_t location, unsigned char* data, int size, ArrayOfNode* Patches, int PatchIndice);
	long InjectionPatcher(uint64_t location, unsigned char* data, int size, ArrayOfNode* Patches, int PatchIndice);
	void ModificationPatcher(uint64_t location, unsigned char* data, int size, DiffNode* Patch);

private:
	UDKFileTypes m_FileType;
	wxMemoryBuffer m_InternalFileBuffer;
	int m_BlockRWSize;
	uint64_t m_BlockRWCount;
	bool m_RAMProcess;
	int m_ProcessID;
	FileAccessMode m_FileAccessMode;
	ArrayOfNode m_DiffArray;
	ArrayOfNode m_TempDiffArray;
	wxFileName m_TheFile;
	uint64_t m_Putptr, m_Getptr;
	wxMemoryBuffer m_XORview;
#ifdef __WXMSW__
	HANDLE m_HDevice;
	WCHAR m_szDosDevice[MAX_PATH], m_szCFDevice[MAX_PATH];
	wxString m_devnm;
	WindowsHDD m_Wdd;
#endif // __WXMSW__
	//		DiffNode *head,*tail;	//linked list holds modification record
};