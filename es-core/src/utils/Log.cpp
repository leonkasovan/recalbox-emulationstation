#include "Log.h"
#include "RootFolders.h"
#include "utils/datetime/DateTime.h"

LogLevel Log::reportingLevel = LogLevel::LogInfo;
FILE* Log::sFile = nullptr;
Path Log::mPath;

const char* Log::sStringLevel[] =
{
  "ERROR",
  "WARN!",
  "INFO ",
	"DEBUG",
  "TRACE",
};

Path Log::FormatLogPath(const char* filename)
{
  if (filename == nullptr) filename = "unknown.log";
  Path path(filename);
  if (!path.IsAbsolute())
    path = RootFolders::DataRootFolder / "system/logs" / path;
	return path;
}

void Log::Open(const char* filename)
{
  // Build log path
  mPath = FormatLogPath(filename);

  // Backup?
  if (mPath.Exists())
  {
    Path backup(mPath.ChangeExtension(".backup"));
    if (!backup.Delete()) { printf("[Logs] Cannot remove old log!"); }
    if (!Path::Rename(mPath, backup)) { printf("[Logs] Cannot backup current log!"); }
  }

  // Open new log
  sFile = fopen(mPath.ToChars(), "w");
}

Log& Log::get(LogLevel level)
{
	mMessage.Append('[')
	        .Append(DateTime().ToPreciseTimeStamp())
	        .Append("] (")
	        .Append(sStringLevel[(int)level])
	        .Append(") : ");
	messageLevel = level;

	return *this;
}

void Log::Flush()
{
  (void)fflush(sFile);
}

void Log::Close()
{
  { Log().get(LogLevel::LogInfo) << "Closing logger..."; }
  DoClose();
}

void Log::DoClose()
{
  if (sFile != nullptr) (void)fclose(sFile);
  sFile = nullptr;
}

Log::~Log()
{
	bool loggerClosed = (sFile == nullptr);
	// Reopen temporarily
	if (loggerClosed)
  {
	  Open(mPath.ToChars());
	  mMessage += " [closed!]";
  }

  mMessage += '\n';
  (void)fputs(mMessage.c_str(), sFile);
	if (!loggerClosed) Flush();
	else DoClose();

  // if it's an error, also print to console
  // print all messages if using --debug
  if(messageLevel == LogLevel::LogError || reportingLevel >= LogLevel::LogDebug)
    (void)fputs(mMessage.c_str(), stdout);
}
