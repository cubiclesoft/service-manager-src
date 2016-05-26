// A simple program whose sole job is to manage a single executable as a service.
// Platform and language agnostic (once compiled).
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#ifdef _MBCS
#undef _MBCS
#endif

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "convert/convert_int.h"
#include "environment/environment_appinfo.h"
#include "utf8/utf8_appinfo.h"
#include "utf8/utf8_file_dir.h"
#include "utf8/utf8_mixed_var.h"

using namespace CubicleSoft;

void DumpGenericSyntax()
{
	printf("Use an English, hyphenated, lowercase name for 'service-name' to maximize\ncompatibility across platforms.\n\n");

	printf("\n");
	printf("Actions:\n\n");

	printf("install\n");
	printf("\tInstall service/daemon.\n");
	printf("\tRequires NotifyFile and ExecutableToRun.\n");
	printf("\t'NotifyFile.stop' will be created when the process needs to stop.\n");
	printf("\t'NotifyFile.reload' will be created when the process needs to\n\treload its configuration.\n\n");

	printf("uninstall\n");
	printf("\tUninstall service/daemon.\n\n");

	printf("start\n");
	printf("\tStart service manager and service/daemon.\n\n");

	printf("stop\n");
	printf("\tStop service manager and service/daemon.\n");
	printf("\tProcess is responsible for noticing the 'NotifyFile.stop' file\n\tand exiting in a timely manner.\n\n");

	printf("restart\n");
	printf("\tRestart service/daemon.\n\n");

	printf("reload\n");
	printf("\tReload service/daemon.\n");
	printf("\tProcess is responsible for deleting the 'NotifyFile.reload'\n\tfile when the configuration has been reloaded.\n");
	printf("\tIf the process is unable to support reloading, it must\n\tself-terminate.\n\n");

	printf("waitfor\n");
	printf("\tWaits for the specified service manager service to start.\n");
	printf("\tUseful for handling dependencies inside the service itself.\n\n");

	printf("status\n");
	printf("\tRetrieve basic service manager information.\n\n");

	printf("configfile\n");
	printf("\tOutputs the location of the server manager configuration file.\n\n");

	printf("run\n");
	printf("\tRuns the service.\n");
	printf("\tMost useful when -debug is specified, which runs the service as\n\ta simple console/terminal application.\n\n");

	printf("addaction\n");
	printf("\tAdds a custom service manager action.\n");
	printf("\tUseful for actions such as 'configtest'.\n");
	printf("\tRequires CustomActionName, CustomActionDescription, and\n\tExecutableToRun.\n\n");

	printf("\n");
	printf("Options:\n\n");

	printf("-pid=File\n");
	printf("\tWrites the process ID of this executable to the specified file.\n");
	printf("\tInstall and run only.\n\n");

	// NOTE:  Windows automatically writes service start/stop/restart events of the service itself to the Event Log.
	printf("-log=File\n");
	printf("\tWrites management information (start, stop, reload, etc.) to the\n\tspecified log file.  You are on your own for log rotation.\n");
	printf("\tInstall and run only.\n\n");

	printf("-wait[=Milliseconds]\n");
	printf("\tThe amount of time, in milliseconds, to wait for the process to\n\tcomplete an operation before forcefully terminating the process\n\tafter creating the appropriate NotifyFile.\n");
	printf("\tThe default behavior is to wait indefinitely.\n");
	printf("\tInstall and run only.\n\n");

	printf("-dir=StartDir\n");
	printf("\tSets the starting directory of the new process.\n");
	printf("\tInstall and run only.\n\n");

	printf("-debug\n");
	printf("\tEnables console/terminal mode.  Run only.\n\n");
}

bool GxDebug = false;

void WriteLog(UTF8::File &LogFile, const char *Message, bool Display = true)
{
	if (GxDebug && Display)  printf("%s\n", Message);

	if (!LogFile.IsOpen())  return;

	char TempBuffer[64];
	time_t CurrTime;
	size_t y;

	CurrTime = time(NULL);
	strftime(TempBuffer, sizeof(TempBuffer) - 1, "%Y-%m-%d %H:%M:%S", localtime(&CurrTime));

	LogFile.Write(TempBuffer, y);
	LogFile.Write("\t", y);
	LogFile.Write(Message, y);
	LogFile.Write("\n", y);
}


#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)

// Windows.
#include <windows.h>
#include <tchar.h>

#pragma message("Compiling for Windows...")

#ifndef ERROR_ELEVATION_REQUIRED
	#define ERROR_ELEVATION_REQUIRED   740
#endif

#ifndef INHERIT_PARENT_AFFINITY
	#define INHERIT_PARENT_AFFINITY    0x00010000L
#endif

void DumpSyntax(TCHAR *currfile)
{
	_tprintf(_T("(C) 2016 CubicleSoft.  All Rights Reserved.\n\n"));

	_tprintf(_T("Syntax:\n\n%s [options] action service-name [[NotifyFile | CustomActionName CustomActionDescription] ExecutableToRun [arguments]]\n\n"), currfile);

	DumpGenericSyntax();

	// Windows only options.
	_tprintf(_T("-winflag=PriorityClass\n"));
	_tprintf(_T("\tSets the priority class of the new process.\n\tInstall and run only.  Windows only.\n"));
	_tprintf(_T("\tThere is only one priority class per process.\n"));
	_tprintf(_T("\tThe 'PriorityClass' can be one of:\n"));
	_tprintf(_T("\tABOVE_NORMAL_PRIORITY_CLASS\n"));
	_tprintf(_T("\tBELOW_NORMAL_PRIORITY_CLASS\n"));
	_tprintf(_T("\tHIGH_PRIORITY_CLASS\n"));
	_tprintf(_T("\tIDLE_PRIORITY_CLASS\n"));
	_tprintf(_T("\tNORMAL_PRIORITY_CLASS\n"));
	_tprintf(_T("\tREALTIME_PRIORITY_CLASS\n\n"));

	_tprintf(_T("-winflag=CreateFlag\n"));
	_tprintf(_T("\tSets a creation flag for the new process.\n\tInstall and run only.  Windows only.\n"));
	_tprintf(_T("\tMultiple -winflag CreateFlag options can be specified.\n"));
	_tprintf(_T("\tEach 'CreateFlag' can be one of:\n"));
	_tprintf(_T("\tCREATE_DEFAULT_ERROR_MODE\n"));
	_tprintf(_T("\tCREATE_NEW_CONSOLE\n"));
	_tprintf(_T("\tCREATE_NEW_PROCESS_GROUP\n"));
	_tprintf(_T("\tCREATE_NO_WINDOW\n"));
	_tprintf(_T("\tCREATE_PROTECTED_PROCESS\n"));
	_tprintf(_T("\tCREATE_PRESERVE_CODE_AUTHZ_LEVEL\n"));
	_tprintf(_T("\tCREATE_SEPARATE_WOW_VDM\n"));
	_tprintf(_T("\tCREATE_SHARED_WOW_VDM\n"));
	_tprintf(_T("\tDEBUG_ONLY_THIS_PROCESS\n"));
	_tprintf(_T("\tDEBUG_PROCESS\n"));
	_tprintf(_T("\tDETACHED_PROCESS\n"));
	_tprintf(_T("\tINHERIT_PARENT_AFFINITY\n\n"));
}

void DisplayError(TCHAR *intro)
{
	DWORD errnum = ::GetLastError();
	LPTSTR errmsg = NULL;

	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errmsg, 0, NULL);
	_tprintf(intro);
	if (errmsg == NULL)  _tprintf(_T("%d - Unknown error\n\n"), errnum);
	else
	{
		_tprintf(_T("%d - %s\n"), errnum, errmsg);
		::LocalFree(errmsg);
	}
}

// Some globals to make life easier for debug vs. service modes of operation.
HANDLE GxServiceStopEvent = NULL;
SERVICE_STATUS GxServiceStatus = {0};
SERVICE_STATUS_HANDLE GxStatusHandle = NULL;

class AppInitState
{
public:
	int MxArgc;
	TCHAR **MxArgv;
	DWORD MxWaitAmount = INFINITE;
	LPTSTR MxPIDFileStr = NULL;
	LPTSTR MxLogFileStr = NULL;
	DWORD MxPriorityFlag = 0;
#ifdef UNICODE
	DWORD MxCreateFlags = CREATE_UNICODE_ENVIRONMENT;
#else
	DWORD MxCreateFlags = 0;
#endif
	LPTSTR MxStartDir = NULL;
	LPTSTR MxServiceName = NULL;
	LPTSTR MxMainAction = NULL;
	STARTUPINFO MxStartInfo;
	SECURITY_ATTRIBUTES MxSecAttr;
	DWORD MxExitCode = 0;
	int MxExeArgc = 0;

	AppInitState()
	{
		MxStartInfo = {0};
		MxSecAttr = {0};
	}
};

AppInitState GxApp;

bool ProcessArgs(int argc, TCHAR **argv)
{
	if (argc < 3)
	{
		DumpSyntax(argv[0]);

		return false;
	}

	// Initialize structures.
	GxApp.MxStartInfo.cb = sizeof(GxApp.MxStartInfo);
	GxApp.MxStartInfo.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
	GxApp.MxStartInfo.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
	GxApp.MxStartInfo.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
	GxApp.MxStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	GxApp.MxSecAttr.nLength = sizeof(GxApp.MxSecAttr);
	GxApp.MxSecAttr.bInheritHandle = TRUE;
	GxApp.MxSecAttr.lpSecurityDescriptor = NULL;

	// Process command-line options.
	int x;
	for (x = 1; x < argc; x++)
	{
		if (!_tcsicmp(argv[x], _T("-debug")))  GxDebug = true;
		else if (!_tcsncicmp(argv[x], _T("-pid="), 5))  GxApp.MxPIDFileStr = argv[x] + 5;
		else if (!_tcsncicmp(argv[x], _T("-log="), 5))  GxApp.MxLogFileStr = argv[x] + 5;
		else if (!_tcsncicmp(argv[x], _T("-wait="), 6))  GxApp.MxWaitAmount = _tstoi(argv[x] + 6);
		else if (!_tcsncicmp(argv[x], _T("-dir="), 5))  GxApp.MxStartDir = argv[x] + 5;
		else if (!_tcsicmp(argv[x], _T("-winflag=ABOVE_NORMAL_PRIORITY_CLASS")))  GxApp.MxPriorityFlag = ABOVE_NORMAL_PRIORITY_CLASS;
		else if (!_tcsicmp(argv[x], _T("-winflag=BELOW_NORMAL_PRIORITY_CLASS")))  GxApp.MxPriorityFlag = BELOW_NORMAL_PRIORITY_CLASS;
		else if (!_tcsicmp(argv[x], _T("-winflag=HIGH_PRIORITY_CLASS")))  GxApp.MxPriorityFlag = HIGH_PRIORITY_CLASS;
		else if (!_tcsicmp(argv[x], _T("-winflag=IDLE_PRIORITY_CLASS")))  GxApp.MxPriorityFlag = IDLE_PRIORITY_CLASS;
		else if (!_tcsicmp(argv[x], _T("-winflag=NORMAL_PRIORITY_CLASS")))  GxApp.MxPriorityFlag = NORMAL_PRIORITY_CLASS;
		else if (!_tcsicmp(argv[x], _T("-winflag=REALTIME_PRIORITY_CLASS")))  GxApp.MxPriorityFlag = REALTIME_PRIORITY_CLASS;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_DEFAULT_ERROR_MODE")))  GxApp.MxCreateFlags |= CREATE_DEFAULT_ERROR_MODE;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_NEW_CONSOLE")))  GxApp.MxCreateFlags |= CREATE_NEW_CONSOLE;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_NEW_PROCESS_GROUP")))  GxApp.MxCreateFlags |= CREATE_NEW_PROCESS_GROUP;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_NO_WINDOW")))  GxApp.MxCreateFlags |= CREATE_NO_WINDOW;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_PROTECTED_PROCESS")))  GxApp.MxCreateFlags |= CREATE_PROTECTED_PROCESS;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_PRESERVE_CODE_AUTHZ_LEVEL")))  GxApp.MxCreateFlags |= CREATE_PRESERVE_CODE_AUTHZ_LEVEL;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_SEPARATE_WOW_VDM")))  GxApp.MxCreateFlags |= CREATE_SEPARATE_WOW_VDM;
		else if (!_tcsicmp(argv[x], _T("-winflag=CREATE_SHARED_WOW_VDM")))  GxApp.MxCreateFlags |= CREATE_SHARED_WOW_VDM;
		else if (!_tcsicmp(argv[x], _T("-winflag=DEBUG_ONLY_THIS_PROCESS")))  GxApp.MxCreateFlags |= DEBUG_ONLY_THIS_PROCESS;
		else if (!_tcsicmp(argv[x], _T("-winflag=DEBUG_PROCESS")))  GxApp.MxCreateFlags |= DEBUG_PROCESS;
		else if (!_tcsicmp(argv[x], _T("-winflag=DETACHED_PROCESS")))  GxApp.MxCreateFlags |= DETACHED_PROCESS;
		else if (!_tcsicmp(argv[x], _T("-winflag=INHERIT_PARENT_AFFINITY")))  GxApp.MxCreateFlags |= INHERIT_PARENT_AFFINITY;
		else if (!_tcsicmp(argv[x], _T("-?")))
		{
			DumpSyntax(argv[0]);

			return false;
		}
		else if (!_tcsnicmp(argv[x], _T("-nixuser="), 9) || !_tcsnicmp(argv[x], _T("-nixgroup="), 10))
		{
			// *NIX-only options.  Ignore.
		}
		else
		{
			// Probably reached the command to execute portion of the arguments.
			break;
		}
	}

	// Failed to find required options.
	if (x + 1 >= argc)
	{
		_tprintf(_T("Error:  'action' or 'service-name' not specified.\n\n"));

		DumpSyntax(argv[0]);

		return false;
	}

	GxApp.MxMainAction = argv[x];
	x++;
	GxApp.MxServiceName = argv[x];
	x++;

	GxApp.MxExeArgc = x;

	return true;
}

bool OpenServiceInfoFile(UTF8::File &DestFile, int Flags, UTF8::UTF8MixedVar<char[8192]> &TempBuffer)
{
	UTF8::UTF8MixedVar<char[8192]> TempBuffer2;
	size_t y;

	// Locate service info file.
	y = sizeof(TempBuffer.MxStr);
	if (!UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer.MxStr, y, "servicemanager"))
	{
		printf("Error:  Unable to retrieve system application storage directory location.\n");

		return false;
	}
	TempBuffer.SetSize(y - 1);

	TempBuffer2.SetUTF8(GxApp.MxServiceName, wcslen(GxApp.MxServiceName));
	TempBuffer.AppendStr(TempBuffer2.MxStr);

	if (!DestFile.Open(TempBuffer.MxStr, Flags))
	{
		printf("Error:  Unable to open '%s'.\n", TempBuffer.MxStr);

		return false;
	}

	return true;
}

bool GetServiceInfoStr(const char *Key, UTF8::UTF8MixedVar<char[8192]> &DestBuffer, bool IgnoreKeyNotFound = false)
{
	UTF8::UTF8MixedVar<char[8192]> TempBuffer;
	UTF8::File TempFile;
	if (!OpenServiceInfoFile(TempFile, O_RDONLY, TempBuffer))  return false;

	bool Found = false;
	size_t y2 = strlen(Key);
	while (!Found && TempFile.GetCurrPos() < TempFile.GetMaxPos())
	{
		char *TempStr = TempFile.LineInput();
		if (!strnicmp(Key, TempStr, y2) && TempStr[y2] == '=')
		{
			DestBuffer.SetStr(TempStr + y2 + 1);

			Found = true;
		}

		delete[] TempStr;
	}

	if (!Found && !IgnoreKeyNotFound)  printf("Warning:  Unable to find '%s' in '%s'.\n", Key, TempBuffer.MxStr);

	TempFile.Close();

	return Found;
}

int RunMain(int argc, TCHAR **argv)
{
	// Load configuration information.
	UTF8::UTF8MixedVar<char[8192]> PIDFilename, LogFilename, NotifyStopFilename, NotifyReloadFilename, TempBuffer, TempBuffer2;
	WCHAR CmdLine[8192];
	WCHAR StartDir[8192];
	size_t y;
	UTF8::File TempFile, LogFile;

	PIDFilename.SetStr("");
	LogFilename.SetStr("");

	if (GxDebug)
	{
		// Running in debug mode requires additional arguments.
		if (GxApp.MxExeArgc + 1 >= argc)
		{
			_tprintf(_T("Missing 'NotifyFile' or 'ExecutableToRun'.\n\n"));

			DumpSyntax(argv[0]);

			return 1;
		}

		NotifyStopFilename.SetUTF8(argv[GxApp.MxExeArgc], wcslen(argv[GxApp.MxExeArgc]));
		NotifyReloadFilename.SetStr(NotifyStopFilename.MxStr);
		NotifyStopFilename.AppendStr(".stop");
		NotifyReloadFilename.AppendStr(".reload");

		if (GxApp.MxPIDFileStr != NULL)  PIDFilename.SetUTF8(GxApp.MxPIDFileStr, wcslen(GxApp.MxPIDFileStr));

		if (GxApp.MxLogFileStr != NULL)  LogFilename.SetUTF8(GxApp.MxLogFileStr, wcslen(GxApp.MxLogFileStr));
		else  GetServiceInfoStr("log", LogFilename);

		if (GxApp.MxStartDir != NULL)  wcscpy(StartDir, GxApp.MxStartDir);
		else  StartDir[0] = _T('\0');

		TempBuffer.SetStr("");
		for (int x = GxApp.MxExeArgc + 1; x < argc; x++)
		{
			if (x > GxApp.MxExeArgc + 1)  TempBuffer.AppendChar(' ');
			TempBuffer.AppendChar('\"');
			TempBuffer.AppendUTF8(argv[x]);
			TempBuffer.AppendChar('\"');
		}

		y = sizeof(CmdLine) / sizeof(WCHAR);
		TempBuffer.ConvertFromUTF8(CmdLine, y);
	}
	else
	{
		if (!GetServiceInfoStr("notify", NotifyStopFilename))  return 1;
		NotifyReloadFilename.SetStr(NotifyStopFilename.MxStr);
		NotifyStopFilename.AppendStr(".stop");
		NotifyReloadFilename.AppendStr(".reload");

		GetServiceInfoStr("pid", PIDFilename);
		GetServiceInfoStr("log", LogFilename);

		if (GetServiceInfoStr("wait", TempBuffer, true) && TempBuffer.MxStrPos)  GxApp.MxWaitAmount = (DWORD)strtoul(TempBuffer.MxStr, NULL, 0);

		if (!GetServiceInfoStr("cmd", TempBuffer))  return 1;
		y = sizeof(CmdLine) / sizeof(WCHAR);
		TempBuffer.ConvertFromUTF8(CmdLine, y);

		if (!GetServiceInfoStr("dir", TempBuffer))  return 1;
		y = sizeof(StartDir) / sizeof(WCHAR);
		TempBuffer.ConvertFromUTF8(StartDir, y);

		// Retrieve optional flags.
		if (GetServiceInfoStr("win_priority", TempBuffer, true))  GxApp.MxPriorityFlag = (DWORD)strtoul(TempBuffer.MxStr, NULL, 0);
		if (GetServiceInfoStr("win_createflags", TempBuffer, true))  GxApp.MxCreateFlags = (DWORD)strtoul(TempBuffer.MxStr, NULL, 0);
	}

	LogFile.Open(LogFilename.MxStr, O_CREAT | O_WRONLY | O_APPEND, UTF8::File::ShareBoth, 0644);

	WriteLog(LogFile, "Service manager started.");

	size_t CurrState = 0, NextState = 0;
	PROCESS_INFORMATION ProcInfo = {0};
	DWORD StateTimeLeft;

	do
	{
		switch (CurrState)
		{
			case 0:
			{
				// Start the service executable.
				TempBuffer.SetStr("Starting process:  ");
				TempBuffer.AppendUTF8(CmdLine);
				WriteLog(LogFile, TempBuffer.MxStr, false);

				if (PIDFilename.MxStrPos)  UTF8::File::Delete(PIDFilename.MxStr);
				UTF8::File::Delete(NotifyStopFilename.MxStr);
				UTF8::File::Delete(NotifyReloadFilename.MxStr);

				ProcInfo = {0};

				if (!::CreateProcessW(NULL, CmdLine, &GxApp.MxSecAttr, &GxApp.MxSecAttr, TRUE, GxApp.MxCreateFlags | GxApp.MxPriorityFlag, NULL, (StartDir[0] != _T('\0') ? StartDir : NULL), &GxApp.MxStartInfo, &ProcInfo))
				{
					DWORD ErrorNum = ::GetLastError();

					TempBuffer.SetStr("An error occurred while attempting to start the process.  Error ");
					Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)ErrorNum);
					TempBuffer.AppendStr(TempBuffer2.MxStr);
					TempBuffer.AppendChar('.');
					if (ErrorNum == ERROR_ELEVATION_REQUIRED)  TempBuffer.AppendStr("  The program must be run as Administrator.");
					TempBuffer.AppendStr("  Command = ");
					TempBuffer.AppendUTF8(CmdLine);

					WriteLog(LogFile, TempBuffer.MxStr);

					GxApp.MxExitCode = 1;
					CurrState = 100;
				}
				else
				{
					::CloseHandle(ProcInfo.hThread);

					// Write the process IDs to the PID file.
					if (PIDFilename.MxStrPos)
					{
						if (!TempFile.Open(PIDFilename.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0644))  WriteLog(LogFile, "Unable to create PID file.", false);
						else
						{
							Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)Environment::AppInfo::GetCurrentProcessID());
							TempFile.Write(TempBuffer2.MxStr, y);
							TempFile.Write("\n", y);

							Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)ProcInfo.dwProcessId);
							TempFile.Write(TempBuffer2.MxStr, y);
							TempFile.Write("\n", y);

							TempFile.Close();
						}
					}

					CurrState = 1;
				}

				break;
			}
			case 1:
			case 4:
			case 5:
			case 6:
			{
				if (::WaitForSingleObject(ProcInfo.hProcess, (CurrState == 1 || StateTimeLeft > 2000 ? 2000 : StateTimeLeft)) == WAIT_OBJECT_0)
				{
					// Process completed.
					NextState = (CurrState == 4 ? 100 : 0);
					CurrState = 3;
				}
				else if (CurrState != 4 && ::WaitForSingleObject(GxServiceStopEvent, 0) == WAIT_OBJECT_0)
				{
					// Service manager has been requested by the OS to terminate.
					if (TempFile.Open(NotifyStopFilename.MxStr, O_CREAT | O_WRONLY))
					{
						TempFile.Close();

						CurrState = 4;
						StateTimeLeft = GxApp.MxWaitAmount;
					}
					else
					{
						// Force terminate the process since communication is not possible.
						CurrState = 2;
						NextState = 100;
					}
				}
				else if (CurrState == 4 || CurrState == 5 || UTF8::File::Exists(NotifyStopFilename.MxStr))
				{
					// Stop.
					if (CurrState != 4 && CurrState != 5)
					{
						CurrState = 5;
						StateTimeLeft = GxApp.MxWaitAmount;
					}
					else
					{
						StateTimeLeft -= (StateTimeLeft > 2000 ? 2000 : StateTimeLeft);
						if (!StateTimeLeft)
						{
							// Force the process to terminate since the timeout has expired.
							NextState = (CurrState == 4 ? 100 : 0);
							CurrState = 2;
						}
					}
				}
				else if (UTF8::File::Exists(NotifyReloadFilename.MxStr))
				{
					// Reload.
					if (CurrState != 6)
					{
						CurrState = 6;
						StateTimeLeft = GxApp.MxWaitAmount;
					}
					else
					{
						StateTimeLeft -= (StateTimeLeft > 2000 ? 2000 : StateTimeLeft);
						if (!StateTimeLeft)
						{
							// Stop the process since it didn't respond in time to reload.
							if (TempFile.Open(NotifyStopFilename.MxStr, O_CREAT | O_WRONLY))
							{
								TempFile.Close();

								CurrState = 5;
								StateTimeLeft = GxApp.MxWaitAmount;
							}
							else
							{
								// Force terminate the process since communication is not possible.
								CurrState = 2;
								NextState = 0;
							}
						}
					}
				}
				else
				{
					CurrState = 1;
				}

				break;
			}
			case 2:
			{
				// Force terminate the process.
				if (!::TerminateProcess(ProcInfo.hProcess, 1))
				{
					WriteLog(LogFile, "Process force termination initiation failed.");

					CurrState = 100;

					break;
				}

				WriteLog(LogFile, "Process force termination initiated.");

				// Wait for termination to complete.
				::WaitForSingleObject(ProcInfo.hProcess, INFINITE);

				// Intentionally fall through to state 3.
			}
			case 3:
			{
				// Process completed.
				if (!::GetExitCodeProcess(ProcInfo.hProcess, &GxApp.MxExitCode))  GxApp.MxExitCode = 0;

				::CloseHandle(ProcInfo.hProcess);

				TempBuffer.SetStr("Process terminated with exit code ");
				Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)GxApp.MxExitCode);
				TempBuffer.AppendStr(TempBuffer2.MxStr);
				TempBuffer.AppendChar('.');

				WriteLog(LogFile, TempBuffer.MxStr);

				// Let the OS have a moment to clean up after the process before continuing.
				::Sleep(1000);

				// Handle the rare instance where the service was told to stop immediately after the executable happened to terminate.
				if (::WaitForSingleObject(GxServiceStopEvent, 0) == WAIT_OBJECT_0)  NextState = 100;

				CurrState = NextState;

				break;
			}
			default:
			{
				if (PIDFilename.MxStrPos)  UTF8::File::Delete(PIDFilename.MxStr);
				UTF8::File::Delete(NotifyStopFilename.MxStr);
				UTF8::File::Delete(NotifyReloadFilename.MxStr);

				WriteLog(LogFile, "Service manager stopped.");

				return (int)GxApp.MxExitCode;
			}
		}
	} while (1);
}

DWORD GxLastCtrlType = (DWORD)-1;
BOOL CtrlHandler(DWORD CtrlType)
{
	// If the same operation is run twice, allow the default handler to run to force-terminate the process.
	BOOL Result = (GxLastCtrlType != CtrlType);
	GxLastCtrlType = CtrlType;

	if (Result)  _tprintf(_T("Attempting clean shutdown.  Please wait.\n"));

	// Let the main function know that it needs to stop running.
	::SetEvent(GxServiceStopEvent);

	return Result;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlType)
{
	if (CtrlType == SERVICE_CONTROL_STOP && GxServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		// Let the service control manager know that the service will be stopping shortly.
		GxServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		GxServiceStatus.dwControlsAccepted = 0;
		GxServiceStatus.dwWin32ExitCode = NO_ERROR;
		GxServiceStatus.dwCheckPoint = 1;
		::SetServiceStatus(GxStatusHandle, &GxServiceStatus);

		// Let the main function know that it needs to stop running.
		::SetEvent(GxServiceStopEvent);
	}
}

VOID WINAPI ServiceMain(DWORD argc, TCHAR **argv)
{
	// Register the service control handler.
	GxStatusHandle = ::RegisterServiceCtrlHandler(GxApp.MxServiceName, ServiceCtrlHandler);
	if (GxStatusHandle == NULL)  return;

	// Let the service control manager know that the application is started.
	GxServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	GxServiceStatus.dwCurrentState = SERVICE_RUNNING;
	GxServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	GxServiceStatus.dwWin32ExitCode = NO_ERROR;
	GxServiceStatus.dwServiceSpecificExitCode = 0;
	GxServiceStatus.dwCheckPoint = 0;
	if (!::SetServiceStatus(GxStatusHandle, &GxServiceStatus))  return;

	GxApp.MxExitCode = (DWORD)RunMain(GxApp.MxArgc, GxApp.MxArgv);

	// Stop the service.
	GxServiceStatus.dwCurrentState = SERVICE_STOPPED;
	GxServiceStatus.dwControlsAccepted = 0;
	GxServiceStatus.dwWin32ExitCode = GxApp.MxExitCode;
	GxServiceStatus.dwCheckPoint = 0;
	::SetServiceStatus(GxStatusHandle, &GxServiceStatus);
}

int _tmain(int argc, TCHAR **argv)
{
	if (!ProcessArgs(argc, argv))  return 1;

	if (!_tcsicmp(GxApp.MxMainAction, _T("install")))
	{
		if (GxDebug)
		{
			_tprintf(_T("The -debug option is not allowed for the install action.\n\n"));

			DumpSyntax(argv[0]);

			return 1;
		}

		// Installation requires additional arguments.
		if (GxApp.MxExeArgc + 1 >= argc)
		{
			_tprintf(_T("Missing 'NotifyFile' or 'ExecutableToRun'.\n\n"));

			DumpSyntax(argv[0]);

			return 1;
		}

		UTF8::UTF8MixedVar<char[8192]> TempBuffer, TempBuffer2, TempBuffer3;
		size_t y;

		// Generate service info file.
		y = sizeof(TempBuffer.MxStr);
		if (!UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer.MxStr, y, "servicemanager"))
		{
			_tprintf(_T("Unable to retrieve system application storage directory location.\n"));

			return 1;
		}
		TempBuffer.SetSize(y - 1);

		UTF8::Dir::Mkdir(TempBuffer.MxStr, 0775, true);

		TempBuffer2.SetUTF8(GxApp.MxServiceName, wcslen(GxApp.MxServiceName));
		TempBuffer.AppendStr(TempBuffer2.MxStr);
		TempBuffer3.SetStr(TempBuffer.MxStr);

		UTF8::File TempFile;
		if (!TempFile.Open(TempBuffer.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0644))
		{
			printf("Unable to create '%s'.\n", TempBuffer.MxStr);

			return 1;
		}

		// Notify file.
		TempBuffer.SetStr("notify=");
		TempBuffer.AppendUTF8(argv[GxApp.MxExeArgc]);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Starting directory.
		TempBuffer.SetStr("dir=");
		if (GxApp.MxStartDir != NULL)  TempBuffer.AppendUTF8(GxApp.MxStartDir);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Command to execute.
		TempBuffer.SetStr("cmd=");
		for (int x = GxApp.MxExeArgc + 1; x < argc; x++)
		{
			if (x > GxApp.MxExeArgc + 1)  TempBuffer.AppendChar(' ');
			TempBuffer.AppendChar('\"');
			TempBuffer.AppendUTF8(argv[x]);
			TempBuffer.AppendChar('\"');
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// PID file.
		TempBuffer.SetStr("pid=");
		if (GxApp.MxPIDFileStr != NULL)  TempBuffer.AppendUTF8(GxApp.MxPIDFileStr);
		else
		{
			TempBuffer.AppendStr(TempBuffer3.MxStr);
			TempBuffer.AppendStr(".pid");
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Log file.
		TempBuffer.SetStr("log=");
		if (GxApp.MxLogFileStr != NULL)  TempBuffer.AppendUTF8(GxApp.MxLogFileStr);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Wait amount.
		TempBuffer.SetStr("wait=");
		if (GxApp.MxWaitAmount != INFINITE)
		{
			Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)GxApp.MxWaitAmount);
			TempBuffer.AppendStr(TempBuffer2.MxStr);
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Windows specific option:  Priority.
		TempBuffer.SetStr("win_priority=");
		Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)GxApp.MxPriorityFlag);
		TempBuffer.AppendStr(TempBuffer2.MxStr);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Windows specific option:  Create flags.
		TempBuffer.SetStr("win_createflags=");
		Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)GxApp.MxCreateFlags);
		TempBuffer.AppendStr(TempBuffer2.MxStr);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		TempFile.Close();

		// Construct the command line to run from the NT service perspective.
		y = sizeof(TempBuffer2.MxStr);
		if (!UTF8::AppInfo::GetExecutableFilename(TempBuffer2.MxStr, y))
		{
			_tprintf(_T("Unable to retrieve executable filename.\n"));

			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}
		TempBuffer2.SetSize(y - 1);
		TempBuffer.SetStr("\"");
		TempBuffer.AppendStr(TempBuffer2.MxStr);
		TempBuffer.AppendStr("\" run \"");
		TempBuffer.AppendUTF8(GxApp.MxServiceName);
		TempBuffer.AppendChar('\"');

		WCHAR CmdLine[8192];
		y = sizeof(CmdLine) / sizeof(WCHAR);
		TempBuffer.ConvertFromUTF8(CmdLine, y);

		SC_HANDLE scm = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
		if (scm == NULL)
		{
			DisplayError(_T("Error opening Service Control Manager.  Are you running as Administrator?\n"));

			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}

		SC_HANDLE service = ::CreateServiceW(scm, GxApp.MxServiceName, GxApp.MxServiceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, CmdLine, NULL, NULL, NULL, NULL, NULL);
		if (service == NULL)
		{
			DisplayError(_T("Error creating service.\n"));
			::CloseServiceHandle(scm);

			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}

		::CloseServiceHandle(service);
		::CloseServiceHandle(scm);

		_tprintf(_T("Service successfully installed.\n"));
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("start")) || !_tcsicmp(GxApp.MxMainAction, _T("stop")) || !_tcsicmp(GxApp.MxMainAction, _T("restart")) || !_tcsicmp(GxApp.MxMainAction, _T("uninstall")))
	{
		// Stop the service manager and service/daemon.
		if (!_tcsicmp(GxApp.MxMainAction, _T("stop")) || !_tcsicmp(GxApp.MxMainAction, _T("restart")) || !_tcsicmp(GxApp.MxMainAction, _T("uninstall")))
		{
			SC_HANDLE scm = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
			if (scm == NULL)
			{
				DisplayError(_T("Error opening Service Control Manager.  Are you running as Administrator?\n"));

				return 1;
			}

			SC_HANDLE service = ::OpenServiceW(scm, GxApp.MxServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS);
			if (service == NULL)
			{
				DisplayError(_T("Error opening service for stopping.\n"));
				::CloseServiceHandle(scm);

				return 1;
			}

			SERVICE_STATUS_PROCESS ssp;
			DWORD BytesNeeded;

			_tprintf(_T("Stopping service..."));

			if (!::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &BytesNeeded))
			{
				DisplayError(_T("Error retrieving service information.\n"));
				::CloseServiceHandle(service);
				::CloseServiceHandle(scm);

				return 1;
			}

			if (!::ControlService(service, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp))
			{
				DisplayError(_T("Error sending stop service control code.\n"));
				::CloseServiceHandle(service);
				::CloseServiceHandle(scm);

				if (!_tcsicmp(GxApp.MxMainAction, _T("stop")))  return 1;
			}
			else
			{
				do
				{
					if (!::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &BytesNeeded))
					{
						DisplayError(_T("\nError retrieving service information.\n"));
						::CloseServiceHandle(service);
						::CloseServiceHandle(scm);

						return 1;
					}

					if (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						_tprintf(_T("."));
						::Sleep(1000);
					}
				} while (ssp.dwCurrentState != SERVICE_STOPPED);

				_tprintf(_T("\n"));

				::CloseServiceHandle(service);
				::CloseServiceHandle(scm);

				_tprintf(_T("Service successfully stopped.\n"));
			}
		}

		// Uninstall the service manager.
		if (!_tcsicmp(GxApp.MxMainAction, _T("uninstall")))
		{
			SC_HANDLE scm = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
			if (scm == NULL)
			{
				DisplayError(_T("Error opening Service Control Manager.  Are you running as Administrator?\n"));

				return 1;
			}

			SC_HANDLE service = ::OpenServiceW(scm, GxApp.MxServiceName, DELETE);
			if (service == NULL)
			{
				DisplayError(_T("Error opening service for deletion.\n"));
				::CloseServiceHandle(scm);

				return 1;
			}

			if (!::DeleteService(service))
			{
				DisplayError(_T("Error marking the service for deletion.\n"));
				::CloseServiceHandle(service);
				::CloseServiceHandle(scm);

				return 1;
			}

			::CloseServiceHandle(service);
			::CloseServiceHandle(scm);

			UTF8::UTF8MixedVar<char[8192]> TempBuffer, TempBuffer2;
			size_t y;

			// Remove service info file.
			y = sizeof(TempBuffer.MxStr);
			if (!UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer.MxStr, y, "servicemanager"))
			{
				_tprintf(_T("Unable to retrieve system application storage directory location.\n"));

				return 1;
			}
			TempBuffer.SetSize(y - 1);

			TempBuffer2.SetUTF8(GxApp.MxServiceName, wcslen(GxApp.MxServiceName));
			TempBuffer.AppendStr(TempBuffer2.MxStr);

			UTF8::File::Delete(TempBuffer.MxStr);

			_tprintf(_T("Service successfully uninstalled.\n"));
		}

		// Start the service manager and service/daemon.
		if (!_tcsicmp(GxApp.MxMainAction, _T("start")) || !_tcsicmp(GxApp.MxMainAction, _T("restart")))
		{
			SC_HANDLE scm = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS);
			if (scm == NULL)
			{
				DisplayError(_T("Error opening Service Control Manager.  Are you running as Administrator?\n"));

				return 1;
			}

			SC_HANDLE service = ::OpenServiceW(scm, GxApp.MxServiceName, SERVICE_START | SERVICE_QUERY_STATUS);
			if (service == NULL)
			{
				DisplayError(_T("Error opening service for starting.\n"));
				::CloseServiceHandle(scm);

				return 1;
			}

			if (!::StartService(service, 0, NULL))
			{
				DisplayError(_T("An error occurred while attempting to start the service.\n"));
				::CloseServiceHandle(service);
				::CloseServiceHandle(scm);

				return 1;
			}

			SERVICE_STATUS_PROCESS ssp;
			DWORD BytesNeeded;

			_tprintf(_T("Starting service..."));

			do
			{
				if (!::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &BytesNeeded))
				{
					DisplayError(_T("\nError retrieving service information.\n"));
					::CloseServiceHandle(service);
					::CloseServiceHandle(scm);

					return 1;
				}

				if (ssp.dwCurrentState == SERVICE_START_PENDING)
				{
					_tprintf(_T("."));
					::Sleep(1000);
				}
			} while (ssp.dwCurrentState == SERVICE_START_PENDING);

			_tprintf(_T("\n"));

			if (ssp.dwCurrentState != SERVICE_RUNNING)
			{
				DisplayError(_T("Service failed to start.\n"));
				::CloseServiceHandle(service);
				::CloseServiceHandle(scm);

				return 1;
			}

			::CloseServiceHandle(service);
			::CloseServiceHandle(scm);

			_tprintf(_T("Service successfully started.\n"));
		}
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("reload")))
	{
		// Reload service configuration.
		UTF8::UTF8MixedVar<char[8192]> TempBuffer;

		if (!GetServiceInfoStr("notify", TempBuffer))  return 1;

		TempBuffer.AppendStr(".reload");

		UTF8::File TempFile;
		if (!TempFile.Open(TempBuffer.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0666))
		{
			printf("Unable to create '%s'.\n", TempBuffer.MxStr);

			return 1;
		}

		TempFile.Close();

		_tprintf(_T("Service reloading..."));
		while (UTF8::File::Exists(TempBuffer.MxStr))
		{
			::Sleep(1000);
			_tprintf(_T("."));
		}
		_tprintf(_T("\n"));

		_tprintf(_T("Service successfully reloaded.\n"));
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("waitfor")))
	{
		// Wait for PID file.
		UTF8::UTF8MixedVar<char[8192]> TempBuffer;

		if (!GetServiceInfoStr("pid", TempBuffer))  return 1;

		while (!UTF8::File::Exists(TempBuffer.MxStr))  ::Sleep(500);

		::Sleep(2000);
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("status")))
	{
		// Retrieve last process restart.
		UTF8::UTF8MixedVar<char[8192]> TempBuffer, TempBuffer2;

		if (!GetServiceInfoStr("pid", TempBuffer))  return 1;

		// Retrieve the status of the service.
		SC_HANDLE scm = ::OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
		if (scm == NULL)
		{
			DisplayError(_T("Error opening Service Control Manager.  Are you running as Administrator?\n"));

			return 1;
		}

		SC_HANDLE service = ::OpenServiceW(scm, GxApp.MxServiceName, SERVICE_QUERY_STATUS);
		if (service == NULL)
		{
			DisplayError(_T("Error opening service for querying its status.\n"));
			::CloseServiceHandle(scm);

			return 1;
		}

		SERVICE_STATUS_PROCESS ssp;
		DWORD BytesNeeded;

		if (!::QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(SERVICE_STATUS_PROCESS), &BytesNeeded))
		{
			DisplayError(_T("Error retrieving service information.\n"));
			::CloseServiceHandle(service);
			::CloseServiceHandle(scm);

			return 1;
		}

		if (ssp.dwCurrentState != SERVICE_RUNNING)
		{
			_tprintf(_T("Service manager is not running.\n"));

			::CloseServiceHandle(service);
			::CloseServiceHandle(scm);

			return 1;
		}

		::CloseServiceHandle(service);
		::CloseServiceHandle(scm);

		_tprintf(_T("Service manager is running.\n"));

		UTF8::File::FileStat TempStat;
		if (!UTF8::File::Stat(TempStat, TempBuffer.MxStr))
		{
			printf("Unable to retrieve information for '%s'.\n", TempBuffer.MxStr);

			return 1;
		}

		strftime(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr) - 1, "%c", localtime(&TempStat.st_mtime));

		printf("Service was started %s.\n", TempBuffer2.MxStr);

		UTF8::File TempFile;
		TempFile.Open(TempBuffer.MxStr, O_RDONLY);

		if (!TempFile.IsOpen())
		{
			printf("Unable to open '%s' for reading.\n", TempBuffer.MxStr);

			return 1;
		}
		else
		{
			char *Line;

			Line = TempFile.LineInput();
			printf("Service manager PID:  %s\n", Line);
			delete[] Line;

			Line = TempFile.LineInput();
			printf("Service PID:  %s\n", Line);
			delete[] Line;

			TempFile.Close();
		}
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("configfile")))
	{
		UTF8::UTF8MixedVar<char[8192]> TempBuffer;
		UTF8::File TempFile;

		if (!OpenServiceInfoFile(TempFile, O_RDONLY, TempBuffer))  return 1;

		printf("%s\n", TempBuffer.MxStr);

		TempFile.Close();
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("addaction")))
	{
		// Running requires additional arguments.
		if (GxApp.MxExeArgc + 2 >= argc)
		{
			_tprintf(_T("Missing 'CustomActionName', 'CustomActionDescription', or 'ExecutableToRun'.\n\n"));

			DumpSyntax(argv[0]);

			return 1;
		}

		UTF8::UTF8MixedVar<char[8192]> TempBuffer, TempBuffer2;
		size_t y;

		TempBuffer2.SetStr("action_");
		TempBuffer2.AppendUTF8(argv[GxApp.MxExeArgc]);
		if (GetServiceInfoStr(TempBuffer2.MxStr, TempBuffer, true))
		{
			printf("Action '%s' already exists in service manager configuration.\n", TempBuffer2.MxStr);

			return 1;
		}

		UTF8::File TempFile;
		if (!OpenServiceInfoFile(TempFile, O_WRONLY | O_APPEND, TempBuffer))  return false;

		// Command to execute.
		TempBuffer.SetStr(TempBuffer2.MxStr);
		TempBuffer.AppendStr("=");
		for (int x = GxApp.MxExeArgc + 2; x < argc; x++)
		{
			if (x > GxApp.MxExeArgc + 2)  TempBuffer.AppendChar(' ');
			TempBuffer.AppendChar('\"');
			TempBuffer.AppendUTF8(argv[x]);
			TempBuffer.AppendChar('\"');
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Action description.
		TempBuffer.SetStr("actiondesc_");
		TempBuffer.AppendUTF8(argv[GxApp.MxExeArgc]);
		TempBuffer.AppendStr("=");
		TempBuffer.AppendUTF8(argv[GxApp.MxExeArgc + 1]);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		TempFile.Close();

		printf("Successfully registered the custom action.\n");
	}
	else if (!_tcsicmp(GxApp.MxMainAction, _T("run")))
	{
		// A rather messy but functional solution to allow the service manager to run as either a NT service or a console application.
		GxServiceStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (GxServiceStopEvent == NULL)
		{
			_tprintf(_T("An error occurred while attempting to create the global event signaling object.\n"));

			return 1;
		}

		if (GxDebug)
		{
			// Override Ctrl+C, Ctrl+Break, etc. to allow for a clean shutdown.
			::SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

			GxApp.MxExitCode = (DWORD)RunMain(argc, argv);
		}
		else
		{
			GxApp.MxArgc = argc;
			GxApp.MxArgv = argv;

// A quick debugger hook.  Put a breakpoint wherever you want despite the 30 second window to attach to the process.
//while (!::IsDebuggerPresent())  ::Sleep(500);
//::Sleep(2000);

			// Start the NT service.
			SERVICE_TABLE_ENTRY ServiceTable[] = {
				{GxApp.MxServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
				{NULL, NULL}
			};

			if (::StartServiceCtrlDispatcher(ServiceTable) == FALSE)
			{
				DisplayError(_T("Error starting service.\n"));

				GxApp.MxExitCode = 1;
			}
		}

		::CloseHandle(GxServiceStopEvent);
	}
	else
	{
		// Try to find a custom action to execute.
		UTF8::UTF8MixedVar<char[8192]> TempBuffer, TempBuffer2;

		TempBuffer2.SetStr("action_");
		TempBuffer2.AppendUTF8(GxApp.MxMainAction);
		if (!GetServiceInfoStr(TempBuffer2.MxStr, TempBuffer))
		{
			printf("\n");

			DumpSyntax(argv[0]);

			return 1;
		}

		// Found a custom action.
		WCHAR CmdLine[8192];
		size_t y;
		y = sizeof(CmdLine) / sizeof(WCHAR);
		TempBuffer.ConvertFromUTF8(CmdLine, y);

		if (!GetServiceInfoStr("dir", TempBuffer))  return 1;

		WCHAR StartDir[8192];
		y = sizeof(StartDir) / sizeof(WCHAR);
		TempBuffer.ConvertFromUTF8(StartDir, y);

		PROCESS_INFORMATION ProcInfo = {0};

		if (!::CreateProcessW(NULL, CmdLine, &GxApp.MxSecAttr, &GxApp.MxSecAttr, TRUE, GxApp.MxCreateFlags | GxApp.MxPriorityFlag, NULL, (StartDir[0] != _T('\0') ? StartDir : NULL), &GxApp.MxStartInfo, &ProcInfo))
		{
			DWORD ErrorNum = ::GetLastError();

			DisplayError(_T("An error occurred while attempting to start the process:\n"));
			if (ErrorNum == ERROR_ELEVATION_REQUIRED)  _tprintf(_T("The program must be run as Administrator.\n"));
			_tprintf(_T("Command = %s\n"), CmdLine);

			return 1;
		}

		::CloseHandle(ProcInfo.hThread);
		if (::WaitForSingleObject(ProcInfo.hProcess, INFINITE) == WAIT_OBJECT_0)
		{
			if (!::GetExitCodeProcess(ProcInfo.hProcess, &GxApp.MxExitCode))  GxApp.MxExitCode = 0;
		}
		::CloseHandle(ProcInfo.hProcess);
	}

	return (int)GxApp.MxExitCode;
}

#else

// Linux, Mac, and most other OSes.
#include "sync/sync_event.h"
#include "templates/fast_find_replace.h"

#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>

#ifdef __APPLE__
#pragma message("Compiling for Mac OSX...")
#else
#pragma message("Compiling for *NIX...")
#endif

void DumpSyntax(char *currfile)
{
	printf("(C) 2016 CubicleSoft.  All Rights Reserved.\n\n");

	printf("Syntax:\n\n%s [options] action service-name [[NotifyFile | CustomActionName CustomActionDescription] ExecutableToRun [arguments]]\n\n", currfile);

	DumpGenericSyntax();

	// *NIX only options.
	printf("-nixuser=Username\n");
	printf("\tSets the user of the new process.\n");
	printf("\tInstall and run only.  *NIX/*BSD/Mac only.\n\n");

	printf("-nixgroup=Groupname\n");
	printf("\tSets the group of the new process.\n");
	printf("\tInstall and run only.  *NIX/*BSD/Mac only.\n\n");
}

// Some globals to make life easier for debug vs. service modes of operation.
Sync::Event GxStopEvent, GxWakeupEvent;

class AppInitState
{
public:
	std::uint32_t MxWaitAmount = INFINITE;
	char *MxPIDFileStr = NULL;
	char *MxLogFileStr = NULL;
	char *MxStartDir = NULL;
	char *MxUserStr = NULL;
	char *MxGroupStr = NULL;
	char *MxServiceName = NULL;
	char *MxMainAction = NULL;
	int MxExitCode = 0;
	int MxExeArgc = 0;
};

AppInitState GxApp;

bool ProcessArgs(int argc, char **argv)
{
	if (argc < 3)
	{
		DumpSyntax(argv[0]);

		return false;
	}

	// Process command-line options.
	int x;
	for (x = 1; x < argc; x++)
	{
		if (!strcasecmp(argv[x], "-debug"))  GxDebug = true;
		else if (!strncasecmp(argv[x], "-pid=", 5))  GxApp.MxPIDFileStr = argv[x] + 5;
		else if (!strncasecmp(argv[x], "-log=", 5))  GxApp.MxLogFileStr = argv[x] + 5;
		else if (!strncasecmp(argv[x], "-wait=", 6))  GxApp.MxWaitAmount = atoi(argv[x] + 6);
		else if (!strncasecmp(argv[x], "-dir=", 5))  GxApp.MxStartDir = argv[x] + 5;
		else if (!strncasecmp(argv[x], "-nixuser=", 9))  GxApp.MxUserStr = argv[x] + 9;
		else if (!strncasecmp(argv[x], "-nixgroup=", 10))  GxApp.MxGroupStr = argv[x] + 10;
		else if (!strcasecmp(argv[x], "-?"))
		{
			DumpSyntax(argv[0]);

			return false;
		}
		else if (!strncasecmp(argv[x], "-winflag=", 9))
		{
			// Windows-only options.  Ignore.
		}
		else
		{
			// Probably reached the command to execute portion of the arguments.
			break;
		}
	}

	// Failed to find required options.
	if (x + 1 >= argc)
	{
		printf("Error:  'action' or 'service-name' not specified.\n\n");

		DumpSyntax(argv[0]);

		return false;
	}

	GxApp.MxMainAction = argv[x];
	x++;
	GxApp.MxServiceName = argv[x];
	x++;

	GxApp.MxExeArgc = x;

	return true;
}

bool OpenServiceInfoFile(UTF8::File &DestFile, int Flags, StaticMixedVar<char[8192]> &TempBuffer)
{
	StaticMixedVar<char[8192]> TempBuffer2;
	size_t y;

	// Locate service info file.
	y = sizeof(TempBuffer.MxStr);
	if (!UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer.MxStr, y, "servicemanager"))
	{
		printf("Error:  Unable to retrieve system application storage directory location.\n");

		return false;
	}
	TempBuffer.SetSize(y - 1);
	TempBuffer.AppendStr(GxApp.MxServiceName);

	if (!DestFile.Open(TempBuffer.MxStr, Flags))
	{
		printf("Error:  Unable to open '%s'.\n", TempBuffer.MxStr);

		return false;
	}

	return true;
}

bool GetServiceInfoStr(const char *Key, StaticMixedVar<char[8192]> &DestBuffer, bool IgnoreKeyNotFound = false)
{
	StaticMixedVar<char[8192]> TempBuffer;
	UTF8::File TempFile;
	if (!OpenServiceInfoFile(TempFile, O_RDONLY, TempBuffer))  return false;

	bool Found = false;
	size_t y2 = strlen(Key);
	while (!Found && TempFile.GetCurrPos() < TempFile.GetMaxPos())
	{
		char *TempStr = TempFile.LineInput();
		if (!strncasecmp(Key, TempStr, y2) && TempStr[y2] == '=')
		{
			DestBuffer.SetStr(TempStr + y2 + 1);

			Found = true;
		}

		delete[] TempStr;
	}

	if (!Found && !IgnoreKeyNotFound)  printf("Warning:  Unable to find '%s' in '%s'.\n", Key, TempBuffer.MxStr);

	TempFile.Close();

	return Found;
}

// Takes a string as input and attempts to parse it into command-line arguments.
// This is fast and mostly functional, not precisely flawless.
// Destroys TempBuffer in the process.
char **ExtractArgs(StaticMixedVar<char[8192]> &TempBuffer)
{
	// Count the number of arguments.
	size_t x, NumArgs = 0, State = 0;

	for (x = 0; x < TempBuffer.MxStrPos; x++)
	{
		switch (State)
		{
			case 0:
			{
				// Looking for the start of the next argument.
				if (TempBuffer.MxStr[x] != ' ' && TempBuffer.MxStr[x] != '\t')
				{
					NumArgs++;

					if (TempBuffer.MxStr[x] == '\'')  State = 1;
					else if (TempBuffer.MxStr[x] == '\\')  State = 2;
					else  State = 3;
				}

				break;
			}
			case 1:
			{
				// Looking for a matching single-quote character.
				if (TempBuffer.MxStr[x] == '\'')  State = 3;

				break;
			}
			case 2:
			{
				// Escaped character found.
				State = 3;

				break;
			}
			case 3:
			{
				if (TempBuffer.MxStr[x] == ' ' || TempBuffer.MxStr[x] == '\t')  State = 0;
				else if (TempBuffer.MxStr[x] == '\'')  State = 1;
				else if (TempBuffer.MxStr[x] == '\\')  State = 2;

				break;
			}
		}
	}

	if (!NumArgs)  return NULL;

	char **Result = new char *[NumArgs + 1];
	NumArgs = 0;
	size_t x2 = 0;
	State = 0;

	for (x = 0; x < TempBuffer.MxStrPos; x++)
	{
		switch (State)
		{
			case 0:
			{
				// Looking for the start of the next argument.
				if (TempBuffer.MxStr[x] != ' ' && TempBuffer.MxStr[x] != '\t')
				{
					Result[NumArgs] = TempBuffer.MxStr + x2;
					NumArgs++;

					if (TempBuffer.MxStr[x] == '\'')  State = 1;
					else if (TempBuffer.MxStr[x] == '\\')  State = 2;
					else  State = 3;
				}

				break;
			}
			case 1:
			{
				// Looking for a matching single-quote character.
				if (TempBuffer.MxStr[x] == '\'')  State = 3;
				else  TempBuffer.MxStr[x2++] = TempBuffer.MxStr[x];

				break;
			}
			case 2:
			{
				// Escaped character found.
				TempBuffer.MxStr[x2++] = TempBuffer.MxStr[x];
				State = 3;

				break;
			}
			case 3:
			{
				// Outside of single-quotes.
				if (TempBuffer.MxStr[x] == '\'')  State = 1;
				else if (TempBuffer.MxStr[x] == '\\')  State = 2;
				else if (TempBuffer.MxStr[x] == ' ' || TempBuffer.MxStr[x] == '\t')
				{
					TempBuffer.MxStr[x2++] = '\0';
					State = 0;
				}
				else
				{
					TempBuffer.MxStr[x2++] = TempBuffer.MxStr[x];
				}

				break;
			}
		}
	}

	TempBuffer.MxStr[x2] = '\0';

	Result[NumArgs] = NULL;

	return Result;
}

int GxLastSignal = -1;
void CtrlHandler(int signum)
{
	if (GxLastSignal != signum)
	{
		printf("Attempting clean shutdown.  Please wait.\n");

		GxStopEvent.Fire();
		GxWakeupEvent.Fire();

		GxLastSignal = signum;
	}
	else
	{
		// User really wants to quit.
		signal(signum, SIG_DFL);
		raise(signum);
	}
}

void WakeupHandler(int signum)
{
	GxWakeupEvent.Fire();

	GxLastSignal = signum;
}

int main(int argc, char **argv)
{
	if (!ProcessArgs(argc, argv))  return 1;

	if (!strcasecmp(GxApp.MxMainAction, "install"))
	{
		if (GxDebug)
		{
			printf("The -debug option is not allowed for the install action.\n\n");

			DumpSyntax(argv[0]);

			return 1;
		}

		// Installation requires additional arguments.
		if (GxApp.MxExeArgc + 1 >= argc)
		{
			printf("Missing 'NotifyFile' or 'ExecutableToRun'.\n\n");

			DumpSyntax(argv[0]);

			return 1;
		}

		StaticMixedVar<char[8192]> TempBuffer, TempBuffer2, TempBuffer3;
		size_t y;

		// Generate service info file.
		y = sizeof(TempBuffer.MxStr);
		if (!UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer.MxStr, y, "servicemanager"))
		{
			printf("Unable to retrieve system application storage directory location.\n");

			return 1;
		}
		TempBuffer.SetSize(y - 1);

		UTF8::Dir::Mkdir(TempBuffer.MxStr, 0775, true);

		TempBuffer.AppendStr(GxApp.MxServiceName);
		TempBuffer3.SetStr(TempBuffer.MxStr);

		UTF8::File TempFile;
		if (!TempFile.Open(TempBuffer.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0644))
		{
			printf("Unable to create '%s'.\n", TempBuffer.MxStr);

			return 1;
		}

		// Notify file.
		TempBuffer.SetStr("notify=");
		TempBuffer.AppendStr(argv[GxApp.MxExeArgc]);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Starting directory.
		TempBuffer.SetStr("dir=");
		if (GxApp.MxStartDir != NULL)  TempBuffer.AppendStr(GxApp.MxStartDir);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Command to execute.
		TempBuffer.SetStr("cmd=");
		for (int x = GxApp.MxExeArgc + 1; x < argc; x++)
		{
			if (x > GxApp.MxExeArgc + 1)  TempBuffer.AppendChar(' ');
			TempBuffer.AppendChar('\'');
			TempBuffer.AppendStr(argv[x]);
			TempBuffer.AppendChar('\'');
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// PID file.
		TempBuffer.SetStr("pid=");
		if (GxApp.MxPIDFileStr != NULL)  TempBuffer.AppendStr(GxApp.MxPIDFileStr);
		else
		{
			TempBuffer.AppendStr(TempBuffer3.MxStr);
			TempBuffer.AppendStr(".pid");
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Log file.
		TempBuffer.SetStr("log=");
		if (GxApp.MxLogFileStr != NULL)  TempBuffer.AppendStr(GxApp.MxLogFileStr);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Wait amount.
		TempBuffer.SetStr("wait=");
		if (GxApp.MxWaitAmount != INFINITE)
		{
			Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)GxApp.MxWaitAmount);
			TempBuffer.AppendStr(TempBuffer2.MxStr);
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// *NIX specific option:  Username.
		TempBuffer.SetStr("nix_user=");
		if (GxApp.MxUserStr != NULL)  TempBuffer.AppendStr(GxApp.MxUserStr);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// *NIX specific option:  Group name.
		TempBuffer.SetStr("nix_group=");
		if (GxApp.MxGroupStr != NULL)  TempBuffer.AppendStr(GxApp.MxGroupStr);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		TempFile.Close();


		// Prepare the system startup configuration file.
		char *FileData, *FileData2;
		size_t y2;

		y = sizeof(TempBuffer.MxStr);
		if (!UTF8::AppInfo::GetExecutablePath(TempBuffer.MxStr, y, argv[0]))
		{
			printf("Unable to retrieve executable path for loading the base platform service file.\n");

			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}
		TempBuffer.SetSize(y - 1);

		#ifdef __APPLE__
		// For Mac OSX, launchd is more natural.
		TempBuffer.AppendStr("servicemanager_mac.launchd");

		#else
		// Use SysVinit for all other OSes.  They generally fallback to init.d.
		TempBuffer.AppendStr("servicemanager_nix.sysvinit");

		#endif

		if (!UTF8::File::LoadEntireFile(TempBuffer.MxStr, FileData, y))
		{
			printf("Failed to load the base platform service file '%s'.\n", TempBuffer.MxStr);

			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}

		// Replace @SERVICENAME@.
		FastReplace<char>::ReplaceAll(FileData2, y2, FileData, y, "@SERVICENAME@", (size_t)-1, GxApp.MxServiceName, strlen(GxApp.MxServiceName));
		delete[] FileData;
		FileData = FileData2;
		y = y2;

		// Replace @SERVICEMANAGER@.
		y2 = sizeof(TempBuffer2.MxStr);
		if (!UTF8::AppInfo::GetExecutableFilename(TempBuffer2.MxStr, y2, argv[0]))
		{
			printf("Unable to retrieve executable filename.\n");

			delete[] FileData2;
			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}
		TempBuffer2.SetSize(y2 - 1);
		FastReplace<char>::ReplaceAll(FileData2, y2, FileData, y, "@SERVICEMANAGER@", (size_t)-1, TempBuffer2.MxStr, TempBuffer2.MxStrPos);
		delete[] FileData;
		FileData = FileData2;
		y = y2;

		// Store the system startup configuration file.
		#ifdef __APPLE__
		TempBuffer.SetStr("/Library/LaunchDaemons/com.servicemanager.");
		TempBuffer.AppendStr(GxApp.MxServiceName);
		TempBuffer.AppendStr(".plist");

		#else
		TempBuffer.SetStr("/etc/init.d/");
		TempBuffer.AppendStr(GxApp.MxServiceName);

		#endif

		if (!TempFile.Open(TempBuffer.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0744))
		{
			printf("Unable to create '%s'.\n", TempBuffer.MxStr);

			delete[] FileData;
			UTF8::File::Delete(TempBuffer3.MxStr);

			return 1;
		}

		TempFile.Write((std::uint8_t *)FileData, y, y2);
		delete[] FileData;

		TempFile.Close();

		printf("Service successfully installed.\n");
	}
	else if (!strcasecmp(GxApp.MxMainAction, "start") || !strcasecmp(GxApp.MxMainAction, "stop") || !strcasecmp(GxApp.MxMainAction, "restart") || !strcasecmp(GxApp.MxMainAction, "uninstall"))
	{
		// Stop the service manager and service/daemon.
		if (!strcasecmp(GxApp.MxMainAction, "stop") || !strcasecmp(GxApp.MxMainAction, "restart") || !strcasecmp(GxApp.MxMainAction, "uninstall"))
		{
		#ifdef __APPLE__
			// Use /bin/launchctl to disable the process (implicit SIGTERM).
			printf("Stopping service...\n");
			fflush(stdout);

			pid_t TempPID = fork();

			if (TempPID < 0)
			{
				printf("An error occurred while attempting to fork() the process to stop the service.\n");

				return 1;
			}
			else if (TempPID == 0)
			{
				StaticMixedVar<char[8192]> TempBuffer;
				TempBuffer.SetStr("/Library/LaunchDaemons/com.servicemanager.");
				TempBuffer.AppendStr(GxApp.MxServiceName);
				TempBuffer.AppendStr(".plist");

				const char *TempArgs[5] = { "/bin/launchctl", "unload", "-w", TempBuffer.MxStr, NULL };

				execv(const_cast<char *>(TempArgs[0]), const_cast<char **>(TempArgs));

				printf("An error occurred while attempting to start the process '%s'.\n", TempArgs[0]);

				return 1;
			}
			else
			{
				// Wait for the child process to complete.
				int Status = 0;
				waitpid(TempPID, &Status, 0);
				GxApp.MxExitCode = (WIFEXITED(Status) ? WEXITSTATUS(Status) : 0);

				if (GxApp.MxExitCode != 0)
				{
					printf("Process exited with exit code %d.\n", GxApp.MxExitCode);

					if (!strcasecmp(GxApp.MxMainAction, "stop"))  return 1;
				}
				else
				{
					printf("Service successfully stopped.\n");
				}
			}

		#else
			// Determine if process is running.
			StaticMixedVar<char[8192]> TempBuffer;

			if (!GetServiceInfoStr("pid", TempBuffer) && !strcasecmp(GxApp.MxMainAction, "stop"))  return 1;

			UTF8::File TempFile;
			TempFile.Open(TempBuffer.MxStr, O_RDONLY);

			if (!TempFile.IsOpen())  printf("Unable to open '%s' for reading.  Service manager is not running.\n", TempBuffer.MxStr);
			else
			{
				char *Line;
				pid_t TempPID;

				Line = TempFile.LineInput();
				TempPID = atoi(Line);
				delete[] Line;

				TempFile.Close();

				printf("Stopping service...");
				fflush(stdout);

				if (kill(TempPID, SIGTERM) < 0)
				{
					printf("\nError sending SIGTERM to service manager process %d.\n", TempPID);

					return 1;
				}

				// Wait for the process to terminate.
				int Result = kill(TempPID, 0);
				while (Result == 0 || (Result < 0 && errno == EPERM))
				{
					sleep(1);
					printf(".");
					fflush(stdout);

					Result = kill(TempPID, 0);
				}

				sleep(1);

				printf("\n");
				printf("Service successfully stopped.\n");
			}

		#endif
		}

		// Uninstall the service manager.
		if (!strcasecmp(GxApp.MxMainAction, "uninstall"))
		{
			StaticMixedVar<char[8192]> TempBuffer;
			size_t y;

			// Delete the system startup configuration file.
			#ifdef __APPLE__
			TempBuffer.SetStr("/Library/LaunchDaemons/com.servicemanager.");
			TempBuffer.AppendStr(GxApp.MxServiceName);
			TempBuffer.AppendStr(".plist");

			#else
			TempBuffer.SetStr("/etc/init.d/");
			TempBuffer.AppendStr(GxApp.MxServiceName);

			#endif

			if (UTF8::File::Exists(TempBuffer.MxStr) && !UTF8::File::Delete(TempBuffer.MxStr))
			{
				printf("Unable to delete '%s'.  Are you root?\n", TempBuffer.MxStr);

				return 1;
			}

			// Remove service info file.
			y = sizeof(TempBuffer.MxStr);
			if (!UTF8::AppInfo::GetSystemAppStorageDir(TempBuffer.MxStr, y, "servicemanager"))
			{
				printf("Unable to retrieve system application storage directory location.\n");

				return 1;
			}
			TempBuffer.SetSize(y - 1);
			TempBuffer.AppendStr(GxApp.MxServiceName);

			UTF8::File::Delete(TempBuffer.MxStr);

			printf("Service successfully uninstalled.\n");
		}

		// Start the service manager and service/daemon.
		if (!strcasecmp(GxApp.MxMainAction, "start") || !strcasecmp(GxApp.MxMainAction, "restart"))
		{
		#ifdef __APPLE__
			// Use /bin/launchctl to enable the process (implicit start).
			printf("Starting service...\n");
			fflush(stdout);

			pid_t TempPID = fork();

			if (TempPID < 0)
			{
				printf("An error occurred while attempting to fork() the process to start the service.\n");

				return 1;
			}
			else if (TempPID == 0)
			{
				StaticMixedVar<char[8192]> TempBuffer;
				TempBuffer.SetStr("/Library/LaunchDaemons/com.servicemanager.");
				TempBuffer.AppendStr(GxApp.MxServiceName);
				TempBuffer.AppendStr(".plist");

				const char *TempArgs[5] = { "/bin/launchctl", "load", "-w", TempBuffer.MxStr, NULL };

				execv(const_cast<char *>(TempArgs[0]), const_cast<char **>(TempArgs));

				printf("An error occurred while attempting to start the process '%s'.\n", TempArgs[0]);

				return 1;
			}
			else
			{
				// Wait for the child process to complete.
				int Status = 0;
				waitpid(TempPID, &Status, 0);
				GxApp.MxExitCode = (WIFEXITED(Status) ? WEXITSTATUS(Status) : 0);

				if (GxApp.MxExitCode != 0)
				{
					printf("Process exited with exit code %d.\n", GxApp.MxExitCode);

					if (!strcasecmp(GxApp.MxMainAction, "stop"))  return 1;
				}
				else
				{
					printf("Service successfully started.\n");
				}
			}

		#else
			// Determine if process is running.
			StaticMixedVar<char[8192]> TempBuffer, TempBuffer2;
			pid_t TempPID;

			if (!GetServiceInfoStr("pid", TempBuffer))  return 1;

			UTF8::File TempFile;
			TempFile.Open(TempBuffer.MxStr, O_RDONLY);

			if (TempFile.IsOpen())
			{
				char *Line;

				Line = TempFile.LineInput();
				TempPID = atoi(Line);
				delete[] Line;

				TempFile.Close();

				int Result = kill(TempPID, 0);
				if (Result == 0 || (Result < 0 && errno == EPERM))
				{
					printf("Service is already running via service manager process %d.\n", TempPID);

					return 1;
				}
			}

			// Service manager not found.  Start the service.
			printf("Starting service...\n");
			fflush(stdout);

			TempPID = fork();

			if (TempPID < 0)
			{
				printf("An error occurred while attempting to fork() the process to start the service.\n");

				return 1;
			}
			else if (TempPID == 0)
			{
				// Start a new session and become session leader.
				if (setsid() < 0)
				{
					printf("An error occurred while attempting to setsid() to become a session leader.\n");

					return 1;
				}

				size_t y = sizeof(TempBuffer.MxStr);
				if (!UTF8::AppInfo::GetExecutableFilename(TempBuffer.MxStr, y, argv[0]))
				{
					printf("Unable to retrieve executable filename.\n");

					return 1;
				}
				TempBuffer.SetSize(y - 1);

				// Change the working directory.
				if (!GetServiceInfoStr("dir", TempBuffer2))  return 1;
				if (TempBuffer2.MxStrPos && chdir(TempBuffer2.MxStr) < 0)
				{
					printf("Unable to change directory to '%s'.\n", TempBuffer2.MxStr);

					return 1;
				}

				// Fork a second time to allow the parent to only wait for everything but execv() to complete successfully.
				TempPID = fork();

				if (TempPID < 0)
				{
					printf("An error occurred while attempting to fork() the process to start the service.\n");

					return 1;
				}
				else if (TempPID == 0)
				{
					// Start service manager.
					const char *TempArgs[4] = { TempBuffer.MxStr, "run", GxApp.MxServiceName, NULL };

					execv(const_cast<char *>(TempArgs[0]), const_cast<char **>(TempArgs));

					printf("An error occurred while attempting to start the process '%s'.\n", TempArgs[0]);

					return 1;
				}
			}
			else
			{
				// Wait for the child process to complete.
				int Status = 0;
				waitpid(TempPID, &Status, 0);
				GxApp.MxExitCode = (WIFEXITED(Status) ? WEXITSTATUS(Status) : 0);

				if (GxApp.MxExitCode != 0)
				{
					printf("Process exited with exit code %d.\n", GxApp.MxExitCode);
				}
				else
				{
					printf("Service successfully started.\n");
				}
			}

		#endif
		}
	}
	else if (!strcasecmp(GxApp.MxMainAction, "reload"))
	{
		// Reload service configuration.
		StaticMixedVar<char[8192]> TempBuffer;

		if (!GetServiceInfoStr("notify", TempBuffer))  return 1;

		TempBuffer.AppendStr(".reload");

		UTF8::File TempFile;
		if (!TempFile.Open(TempBuffer.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0666))
		{
			printf("Unable to create '%s'.\n", TempBuffer.MxStr);

			return 1;
		}

		TempFile.Close();

		printf("Service reloading...");
		fflush(stdout);
		while (UTF8::File::Exists(TempBuffer.MxStr))
		{
			sleep(1);
			printf(".");
			fflush(stdout);
		}
		printf("\n");

		printf("Service successfully reloaded.\n");
	}
	else if (!strcasecmp(GxApp.MxMainAction, "waitfor"))
	{
		// Wait for PID file.
		StaticMixedVar<char[8192]> TempBuffer;

		if (!GetServiceInfoStr("pid", TempBuffer))  return 1;

		while (!UTF8::File::Exists(TempBuffer.MxStr))  usleep(500000);

		sleep(2);
	}
	else if (!strcasecmp(GxApp.MxMainAction, "status"))
	{
		// Retrieve last process restart.
		StaticMixedVar<char[8192]> TempBuffer, TempBuffer2;

		if (!GetServiceInfoStr("pid", TempBuffer))  return 1;

		UTF8::File::FileStat TempStat;
		if (!UTF8::File::Stat(TempStat, TempBuffer.MxStr))
		{
			printf("Unable to retrieve information for '%s'.\n", TempBuffer.MxStr);

			return 1;
		}

		printf("Service manager is running.\n");

		strftime(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr) - 1, "%c", localtime(&TempStat.st_mtime));

		printf("Service was started %s.\n", TempBuffer2.MxStr);

		UTF8::File TempFile;
		TempFile.Open(TempBuffer.MxStr, O_RDONLY);

		if (!TempFile.IsOpen())
		{
			printf("Unable to open '%s' for reading.\n", TempBuffer.MxStr);

			return 1;
		}
		else
		{
			char *Line;

			Line = TempFile.LineInput();
			printf("Service manager PID:  %s\n", Line);
			delete[] Line;

			Line = TempFile.LineInput();
			printf("Service PID:  %s\n", Line);
			delete[] Line;

			TempFile.Close();
		}
	}
	else if (!strcasecmp(GxApp.MxMainAction, "configfile"))
	{
		StaticMixedVar<char[8192]> TempBuffer;
		UTF8::File TempFile;

		if (!OpenServiceInfoFile(TempFile, O_RDONLY, TempBuffer))  return 1;

		printf("%s\n", TempBuffer.MxStr);

		TempFile.Close();
	}
	else if (!strcasecmp(GxApp.MxMainAction, "addaction"))
	{
		// Running requires additional arguments.
		if (GxApp.MxExeArgc + 2 >= argc)
		{
			printf("Missing 'CustomActionName', 'CustomActionDescription', or 'ExecutableToRun'.\n\n");

			DumpSyntax(argv[0]);

			return 1;
		}

		StaticMixedVar<char[8192]> TempBuffer, TempBuffer2;
		size_t y;

		TempBuffer2.SetStr("action_");
		TempBuffer2.AppendStr(argv[GxApp.MxExeArgc]);
		if (GetServiceInfoStr(TempBuffer2.MxStr, TempBuffer, true))
		{
			printf("Action '%s' already exists in service manager configuration.\n", TempBuffer2.MxStr);

			return 1;
		}

		UTF8::File TempFile;
		if (!OpenServiceInfoFile(TempFile, O_WRONLY | O_APPEND, TempBuffer))  return false;

		// Command to execute.
		TempBuffer.SetStr(TempBuffer2.MxStr);
		TempBuffer.AppendStr("=");
		for (int x = GxApp.MxExeArgc + 2; x < argc; x++)
		{
			if (x > GxApp.MxExeArgc + 2)  TempBuffer.AppendChar(' ');
			TempBuffer.AppendChar('\'');
			TempBuffer.AppendStr(argv[x]);
			TempBuffer.AppendChar('\'');
		}

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		// Action description.
		TempBuffer.SetStr("actiondesc_");
		TempBuffer.AppendStr(argv[GxApp.MxExeArgc]);
		TempBuffer.AppendStr("=");
		TempBuffer.AppendStr(argv[GxApp.MxExeArgc + 1]);

		TempFile.Write(TempBuffer.MxStr, y);
		TempFile.Write("\n", y);

		TempFile.Close();

		printf("Successfully registered the custom action.\n");
	}
	else if (!strcasecmp(GxApp.MxMainAction, "run"))
	{
		// Load configuration information.
		StaticMixedVar<char[8192]> PIDFilename, LogFilename, NotifyStopFilename, NotifyReloadFilename, CmdLine, TempBuffer, TempBuffer2;
		char **CmdLineArgs;
		size_t y;
		UTF8::File TempFile, LogFile;
		uid_t UserID = 0;
		gid_t GroupID = 0;

		PIDFilename.SetStr("");
		LogFilename.SetStr("");

		// Some CPU saving objects.
		GxStopEvent.Create();
		GxWakeupEvent.Create();

		if (GxDebug)
		{
			// Running in debug mode requires additional arguments.
			if (GxApp.MxExeArgc + 1 >= argc)
			{
				printf("Missing 'NotifyFile' or 'ExecutableToRun'.\n\n");

				DumpSyntax(argv[0]);

				return 1;
			}

			NotifyStopFilename.SetStr(argv[GxApp.MxExeArgc]);
			NotifyStopFilename.AppendStr(".stop");

			NotifyReloadFilename.SetStr(argv[GxApp.MxExeArgc]);
			NotifyReloadFilename.AppendStr(".reload");

			if (GxApp.MxPIDFileStr != NULL)  PIDFilename.SetStr(GxApp.MxPIDFileStr);

			if (GxApp.MxLogFileStr != NULL)  LogFilename.SetStr(GxApp.MxLogFileStr);
			else  GetServiceInfoStr("log", LogFilename);

			// Retrieve the user.
			if (GxApp.MxUserStr != NULL)
			{
				struct passwd *TempPasswd = getpwnam(GxApp.MxUserStr);
				if (TempPasswd == NULL)
				{
					printf("Unknown user '%s' specified.\n", GxApp.MxUserStr);

					return 1;
				}

				UserID = TempPasswd->pw_uid;
			}

			// Retrieve the group.
			if (GxApp.MxGroupStr != NULL)
			{
				struct group *TempGroup = getgrnam(GxApp.MxGroupStr);
				if (TempGroup == NULL)
				{
					printf("Unknown group '%s' specified.\n", GxApp.MxGroupStr);

					return 1;
				}

				GroupID = TempGroup->gr_gid;
			}

			// Parse command-line arguments.
			CmdLineArgs = new char *[argc - GxApp.MxExeArgc];
			y = 0;
			for (int x = GxApp.MxExeArgc + 1; x < argc; x++)
			{
				CmdLineArgs[y++] = argv[x];
			}
			CmdLineArgs[y] = NULL;

			// Override Ctrl+C to allow for a clean shutdown.
			signal(SIGINT, CtrlHandler);
		}
		else
		{
			if (!GetServiceInfoStr("notify", NotifyStopFilename))  return 1;
			NotifyReloadFilename.SetStr(NotifyStopFilename.MxStr);

			NotifyStopFilename.AppendStr(".stop");
			NotifyReloadFilename.AppendStr(".reload");

			GetServiceInfoStr("pid", PIDFilename);
			GetServiceInfoStr("log", LogFilename);

			if (GetServiceInfoStr("wait", TempBuffer, true) && TempBuffer.MxStrPos)  GxApp.MxWaitAmount = (std::uint32_t)strtoul(TempBuffer.MxStr, NULL, 0);

			// Parse command-line arguments.
			if (!GetServiceInfoStr("cmd", CmdLine))  return 1;

			CmdLineArgs = ExtractArgs(CmdLine);

			if (CmdLineArgs == NULL)
			{
				printf("An error occurred while attempting to extract the arguments to run the service.\n");

				return 1;
			}

			// Retrieve the user.
			if (GetServiceInfoStr("nix_user", TempBuffer, true) && TempBuffer.MxStrPos)
			{
				struct passwd *TempPasswd = getpwnam(TempBuffer.MxStr);
				if (TempPasswd == NULL)
				{
					printf("Unknown user '%s' specified in configuration.\n", TempBuffer.MxStr);

					return 1;
				}

				UserID = TempPasswd->pw_uid;
			}

			// Retrieve the group.
			if (GetServiceInfoStr("nix_group", TempBuffer, true) && TempBuffer.MxStrPos)
			{
				struct group *TempGroup = getgrnam(TempBuffer.MxStr);
				if (TempGroup == NULL)
				{
					printf("Unknown group '%s' specified in configuration.\n", TempBuffer.MxStr);

					return 1;
				}

				GroupID = TempGroup->gr_gid;
			}

			// Override simple termination signals to allow for a clean shutdown.  Obviously can't override SIGKILL.
			signal(SIGINT, CtrlHandler);
			signal(SIGTERM, CtrlHandler);
			signal(SIGQUIT, CtrlHandler);

			// Finalize the service startup by adjusting standard handles to point nowhere.
			if (freopen("/dev/null", "r", stdin) == NULL)  {}
			if (freopen("/dev/null", "w", stdout) == NULL)  {}
			if (freopen("/dev/null", "w", stderr) == NULL)  {}
		}

		// Handle expected OS events a bit differently from the default (wake up the main loop sooner).
		// Leave unexpected OS events alone.  They'll be lonely but will probably do the right thing.
		signal(SIGHUP, WakeupHandler);
		signal(SIGCHLD, WakeupHandler);

		LogFile.Open(LogFilename.MxStr, O_CREAT | O_WRONLY | O_APPEND, UTF8::File::ShareBoth, 0644);

		WriteLog(LogFile, "Service manager started.");

		size_t CurrState = 0, NextState = 0;
		std::uint32_t StateTimeLeft = 0;
		pid_t MainPID = 0;
		int Status;

		do
		{
			switch (CurrState)
			{
				case 0:
				{
					// Start the service executable.
					TempBuffer.SetStr("Starting process:  ");
					for (int x = 0; CmdLineArgs[x] != NULL; x++)
					{
						if (x)  TempBuffer.AppendChar(' ');
						TempBuffer.AppendChar('\'');
						TempBuffer.AppendStr(CmdLineArgs[x]);
						TempBuffer.AppendChar('\'');
					}
					WriteLog(LogFile, TempBuffer.MxStr, false);

					if (PIDFilename.MxStrPos)  UTF8::File::Delete(PIDFilename.MxStr);
					UTF8::File::Delete(NotifyStopFilename.MxStr);
					UTF8::File::Delete(NotifyReloadFilename.MxStr);

					MainPID = fork();

					if (MainPID < 0)
					{
						WriteLog(LogFile, "An error occurred while attempting to fork() the process to start the service.");

						return 1;
					}
					else if (MainPID == 0)
					{
						// Start service.
						if (GxApp.MxStartDir != NULL)
						{
							if (chdir(GxApp.MxStartDir) < 0)  WriteLog(LogFile, "Unable to change directories.");
						}

						if (UserID && setuid(UserID) < 0)  WriteLog(LogFile, "Unable to setuid().");
						if (GroupID && setgid(GroupID) < 0)  WriteLog(LogFile, "Unable to setgid().");

						execv(CmdLineArgs[0], CmdLineArgs);

						TempBuffer.SetStr("An error occurred while attempting to start the process.  Command = ");
						for (int x = 0; CmdLineArgs[x] != NULL; x++)
						{
							if (x)  TempBuffer.AppendChar(' ');
							TempBuffer.AppendChar('\'');
							TempBuffer.AppendStr(CmdLineArgs[x]);
							TempBuffer.AppendChar('\'');
						}
						WriteLog(LogFile, TempBuffer.MxStr);

						GxApp.MxExitCode = 1;
						CurrState = 100;
					}
					else
					{
						// Write the process IDs to the PID file.
						if (PIDFilename.MxStrPos)
						{
							if (!TempFile.Open(PIDFilename.MxStr, O_CREAT | O_WRONLY | O_TRUNC, UTF8::File::ShareBoth, 0644))  WriteLog(LogFile, "Unable to create PID file.", false);
							else
							{
								Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)Environment::AppInfo::GetCurrentProcessID());
								TempFile.Write(TempBuffer2.MxStr, y);
								TempFile.Write("\n", y);

								Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)MainPID);
								TempFile.Write(TempBuffer2.MxStr, y);
								TempFile.Write("\n", y);

								TempFile.Close();
							}
						}

						Status = 0;
						CurrState = 1;
					}

					break;
				}
				case 1:
				case 4:
				case 5:
				case 6:
				{
					GxWakeupEvent.Wait(CurrState == 1 || StateTimeLeft > 2000 ? 2000 : StateTimeLeft);

					if (waitpid(MainPID, &Status, WNOHANG) == MainPID)
					{
						GxApp.MxExitCode = (WIFEXITED(Status) ? WEXITSTATUS(Status) : 0);

						// Process completed.
						NextState = (CurrState == 4 ? 100 : 0);
						CurrState = 3;
					}
					else if (CurrState != 4 && GxStopEvent.Wait(0))
					{
						// Service manager has been requested by the OS to terminate.
						if (TempFile.Open(NotifyStopFilename.MxStr, O_CREAT | O_WRONLY))
						{
							TempFile.Close();

							CurrState = 4;
							StateTimeLeft = GxApp.MxWaitAmount;
						}
						else
						{
							// Force terminate the process since communication is not possible.
							CurrState = 2;
							NextState = 100;
						}
					}
					else if (CurrState == 4 || CurrState == 5 || UTF8::File::Exists(NotifyStopFilename.MxStr))
					{
						// Stop.
						if (CurrState != 4 && CurrState != 5)
						{
							CurrState = 5;
							StateTimeLeft = GxApp.MxWaitAmount;
						}
						else
						{
							StateTimeLeft -= (StateTimeLeft > 2000 ? 2000 : StateTimeLeft);
							if (!StateTimeLeft)
							{
								// Force the process to terminate since the timeout has expired.
								NextState = (CurrState == 4 ? 100 : 0);
								CurrState = 2;
							}
						}
					}
					else if (UTF8::File::Exists(NotifyReloadFilename.MxStr))
					{
						// Reload.
						if (CurrState != 6)
						{
							CurrState = 6;
							StateTimeLeft = GxApp.MxWaitAmount;
						}
						else
						{
							StateTimeLeft -= (StateTimeLeft > 2000 ? 2000 : StateTimeLeft);
							if (!StateTimeLeft)
							{
								// Stop the process since it didn't respond in time to reload.
								if (TempFile.Open(NotifyStopFilename.MxStr, O_CREAT | O_WRONLY))
								{
									TempFile.Close();

									CurrState = 5;
									StateTimeLeft = GxApp.MxWaitAmount;
								}
								else
								{
									// Force terminate the process since communication is not possible.
									CurrState = 2;
									NextState = 0;
								}
							}
						}
					}
					else
					{
						CurrState = 1;
					}

					break;
				}
				case 2:
				{
					// Try a standard termination signal.
					if (kill(MainPID, SIGTERM) < 0)
					{
						// Force terminate the process.
						if (kill(MainPID, SIGKILL) < 0)
						{
							WriteLog(LogFile, "Process force termination initiation failed.");

							CurrState = 100;

							break;
						}

						GxApp.MxExitCode = 1;
					}
					else if (GxWakeupEvent.Wait(3000) && waitpid(MainPID, &Status, WNOHANG) == MainPID)
					{
						GxApp.MxExitCode = (WIFEXITED(Status) ? WEXITSTATUS(Status) : 0);
					}
					else
					{
						// Force terminate the process.
						if (kill(MainPID, SIGKILL) < 0)
						{
							WriteLog(LogFile, "Process force termination initiation failed.");

							CurrState = 100;

							break;
						}

						GxApp.MxExitCode = 1;
					}

					WriteLog(LogFile, "Process force terminated.");

					// Intentionally fall through to state 3.
				}
				case 3:
				{
					// Process completed.
					TempBuffer.SetStr("Process terminated with exit code ");
					Convert::Int::ToString(TempBuffer2.MxStr, sizeof(TempBuffer2.MxStr), (std::uint64_t)GxApp.MxExitCode);
					TempBuffer.AppendStr(TempBuffer2.MxStr);
					TempBuffer.AppendChar('.');

					WriteLog(LogFile, TempBuffer.MxStr);

					// Let the OS have a moment to clean up after the process before continuing.
					sleep(1);

					// Handle the rare instance where the service was told to stop immediately after the executable happened to terminate.
					if (GxStopEvent.Wait(0))  NextState = 100;

					CurrState = NextState;

					break;
				}
				default:
				{
					if (PIDFilename.MxStrPos)  UTF8::File::Delete(PIDFilename.MxStr);
					UTF8::File::Delete(NotifyStopFilename.MxStr);
					UTF8::File::Delete(NotifyReloadFilename.MxStr);

					WriteLog(LogFile, "Service manager stopped.");

					delete[] CmdLineArgs;

					return (int)GxApp.MxExitCode;
				}
			}
		} while (1);
	}
	else
	{
		// Try to find a custom action to execute.
		StaticMixedVar<char[8192]> TempBuffer, TempBuffer2;

		TempBuffer2.SetStr("action_");
		TempBuffer2.AppendStr(GxApp.MxMainAction);
		if (!GetServiceInfoStr(TempBuffer2.MxStr, TempBuffer))
		{
			printf("\n");

			DumpSyntax(argv[0]);

			return 1;
		}

		pid_t TempPID = fork();

		if (TempPID < 0)
		{
			printf("An error occurred while attempting to fork() the process to run the custom action '%s'.\n", GxApp.MxMainAction);

			return 1;
		}
		else if (TempPID == 0)
		{
			// Change the working directory.
			if (!GetServiceInfoStr("dir", TempBuffer2))  return 1;
			if (TempBuffer2.MxStrPos && chdir(TempBuffer2.MxStr) < 0)
			{
				printf("Unable to change directory to '%s'.\n", TempBuffer2.MxStr);

				return 1;
			}

			// Run the process.
			char **TempArgs = ExtractArgs(TempBuffer);

			if (TempArgs == NULL)
			{
				printf("An error occurred while attempting to extract the arguments to run the custom action '%s'.\n", GxApp.MxMainAction);

				return 1;
			}

			execv(TempArgs[0], TempArgs);

			printf("An error occurred while attempting to start the process '%s'.\n", TempArgs[0]);

			delete[] TempArgs;

			return 1;
		}
		else
		{
			// Wait for the child process to complete.
			int Status = 0;
			waitpid(TempPID, &Status, 0);
			GxApp.MxExitCode = (WIFEXITED(Status) ? WEXITSTATUS(Status) : 0);
		}
	}

	return GxApp.MxExitCode;
}

#endif
